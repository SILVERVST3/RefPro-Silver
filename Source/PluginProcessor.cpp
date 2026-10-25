#include "PluginProcessor.h"
#include "PluginEditor.h"

juce::AudioProcessorValueTreeState::ParameterLayout RefProProcessor::createLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;
    p.push_back (std::make_unique<juce::AudioParameterInt> (juce::ParameterID { "target", 1 }, "Target", 0, maxCaptures, 0));
    p.push_back (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { "avg", 1 }, "Averaging",
                    juce::StringArray { "Short (3 s)", "Medium (10 s)", "Long (30 s)", "Infinite" }, 1));
    p.push_back (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { "eqOn", 1 }, "EQ On", true));
    p.push_back (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { "anPost", 1 }, "Analysis Point",
                    juce::StringArray { "Pre-EQ", "Post-EQ" }, 1));

    auto freqText = [] (float v, int) { return v >= 1000.0f ? juce::String (v / 1000.0f, 2) + " kHz" : juce::String ((int) std::round (v)) + " Hz"; };
    auto freqVal  = [] (const juce::String& s)
    {
        const auto l = s.trim().toLowerCase();
        float v = l.getFloatValue();
        if (l.contains ("k")) v *= 1000.0f;
        return v;
    };

    for (int b = 0; b < Eq::numBands; ++b)
    {
        const auto name = "Band " + juce::String (b + 1) + " ";
        juce::NormalisableRange<float> fr (20.0f, 20000.0f, 0.01f);
        fr.setSkewForCentre (1000.0f);
        juce::NormalisableRange<float> qr (0.1f, 10.0f, 0.01f);
        qr.setSkewForCentre (1.0f);

        p.push_back (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { bandId (b, "on"), 1 }, name + "On", false));
        p.push_back (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { bandId (b, "type"), 1 }, name + "Type", Eq::typeNames(), 0));
        p.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { bandId (b, "freq"), 1 }, name + "Freq", fr, Eq::defaultFreqs()[b],
                        juce::AudioParameterFloatAttributes().withStringFromValueFunction (freqText).withValueFromStringFunction (freqVal)));
        p.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { bandId (b, "gain"), 1 }, name + "Gain",
                        juce::NormalisableRange<float> (-18.0f, 18.0f, 0.1f), 0.0f,
                        juce::AudioParameterFloatAttributes().withStringFromValueFunction ([] (float v, int) { return juce::String (v, 1) + " dB"; })
                                                              .withValueFromStringFunction ([] (const juce::String& s) { return s.getFloatValue(); })));
        p.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { bandId (b, "q"), 1 }, name + "Q", qr, 1.0f,
                        juce::AudioParameterFloatAttributes().withStringFromValueFunction ([] (float v, int) { return juce::String (v, 2); })
                                                              .withValueFromStringFunction ([] (const juce::String& s) { return s.getFloatValue(); })));
    }
    return { p.begin(), p.end() };
}

RefProProcessor::RefProProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "RefProSilver", createLayout()),
      allPurpose (Targets::makeAllPurpose())
{
    avgParam  = apvts.getRawParameterValue ("avg");
    eqOnParam = apvts.getRawParameterValue ("eqOn");
    postParam = apvts.getRawParameterValue ("anPost");
    for (int b = 0; b < Eq::numBands; ++b)
    {
        pOn[(size_t) b]   = apvts.getRawParameterValue (bandId (b, "on"));
        pType[(size_t) b] = apvts.getRawParameterValue (bandId (b, "type"));
        pFreq[(size_t) b] = apvts.getRawParameterValue (bandId (b, "freq"));
        pGain[(size_t) b] = apvts.getRawParameterValue (bandId (b, "gain"));
        pQ[(size_t) b]    = apvts.getRawParameterValue (bandId (b, "q"));
    }
}

Eq::Params RefProProcessor::getBandParams (int b) const
{
    Eq::Params p;
    p.on   = pOn[(size_t) b]->load() > 0.5f;
    p.type = juce::jlimit (0, (int) Eq::numTypes - 1, (int) std::round (pType[(size_t) b]->load()));
    p.freq = pFreq[(size_t) b]->load();
    p.gain = pGain[(size_t) b]->load();
    p.q    = pQ[(size_t) b]->load();
    return p;
}

void RefProProcessor::prepareToPlay (double sampleRate, int)
{
    currentSampleRate = sampleRate;
    forceEqUpdate = true;
    for (auto& b : eq) { b.z1[0] = b.z1[1] = b.z2[0] = b.z2[1] = 0.0; }
    analyzer.prepare (sampleRate);
}

bool RefProProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    if (l.getMainInputChannelSet() != l.getMainOutputChannelSet()) return false;
    return l.getMainOutputChannelSet() == juce::AudioChannelSet::stereo()
        || l.getMainOutputChannelSet() == juce::AudioChannelSet::mono();
}

void RefProProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int n = buffer.getNumSamples();
    if (n == 0 || buffer.getNumChannels() < 1) return;

    static const float taus[] = { 3.0f, 10.0f, 30.0f, 0.0f };
    analyzer.setAveragingSeconds (taus[juce::jlimit (0, 3, (int) avgParam->load())]);

    const int numCh = juce::jmin (2, buffer.getNumChannels());
    const bool post = postParam->load() > 0.5f;

    auto analyse = [&]
    {
        const float* l = buffer.getReadPointer (0);
        const float* r = buffer.getNumChannels() > 1 ? buffer.getReadPointer (1) : l;
        analyzer.process (l, r, n);
    };

    if (! post) analyse();

    // ---- equalizador de 10 bandas (biquads RBJ, estado em double) ----
    if (eqOnParam->load() > 0.5f)
    {
        for (int b = 0; b < Eq::numBands; ++b)
        {
            auto& st = eq[(size_t) b];
            const auto prm = getBandParams (b);
            if (forceEqUpdate || prm != st.last)
            {
                st.last = prm;
                st.c = Eq::make (prm, currentSampleRate);
                const bool wasActive = st.active;
                st.active = Eq::isActive (prm);
                if (! wasActive && st.active) { st.z1[0] = st.z1[1] = st.z2[0] = st.z2[1] = 0.0; }
            }
            if (! st.active) continue;

            const auto& c = st.c;
            for (int ch = 0; ch < numCh; ++ch)
            {
                float* d = buffer.getWritePointer (ch);
                double z1 = st.z1[ch], z2 = st.z2[ch];
                for (int i = 0; i < n; ++i)
                {
                    const double x = d[i];
                    const double y = c.b0 * x + z1;
                    z1 = c.b1 * x - c.a1 * y + z2;
                    z2 = c.b2 * x - c.a2 * y;
                    d[i] = (float) y;
                }
                st.z1[ch] = z1; st.z2[ch] = z2;
            }
        }
        forceEqUpdate = false;
    }

    if (post) analyse();
}

// ---------------------------------------------------------------- alvos
int RefProProcessor::getNumTargets() const
{
    const juce::ScopedLock sl (capLock);
    return 1 + (int) captured.size();
}

TargetData RefProProcessor::getTarget (int index) const
{
    const juce::ScopedLock sl (capLock);
    if (index <= 0 || index > (int) captured.size()) return allPurpose;
    return captured[(size_t) index - 1];
}

RefProProcessor::CaptureResult RefProProcessor::captureCurrent()
{
    if (! analyzer.hasData.load()) return CaptureResult::noData;

    const juce::ScopedLock sl (capLock);
    if ((int) captured.size() >= maxCaptures) return CaptureResult::full;

    TargetData t;
    t.name = "Capture " + juce::String (++captureCounter);
    t.builtIn = false;
    for (int i = 0; i < rp::numBands; ++i)
    {
        t.centre[(size_t) i] = analyzer.avgDb[(size_t) i].load();
        t.tol[(size_t) i] = 1.5f;
    }
    t.psr = analyzer.avgPsr.load();
    t.widthDb = analyzer.avgWidthDb.load();
    t.vocalDb = analyzer.vocalConf.load() >= 0.12f ? analyzer.vocalDb.load() : -6.0f;   // sem voz detectada: mantém o padrão
    captured.push_back (t);
    ++targetsVersion;
    return CaptureResult::ok;
}

void RefProProcessor::deleteCapture (int targetIndex)
{
    const juce::ScopedLock sl (capLock);
    if (targetIndex >= 1 && targetIndex <= (int) captured.size())
    {
        captured.erase (captured.begin() + (targetIndex - 1));
        ++targetsVersion;
    }
}

// ---------------------------------------------------------------- estado
void RefProProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    auto state = apvts.copyState();
    state.removeChild (state.getChildWithName ("Captures"), nullptr);

    juce::ValueTree caps ("Captures");
    {
        const juce::ScopedLock sl (capLock);
        caps.setProperty ("counter", captureCounter, nullptr);
        for (auto& c : captured)
        {
            juce::ValueTree v ("Capture");
            v.setProperty ("name", c.name, nullptr);
            v.setProperty ("psr", c.psr, nullptr);
            v.setProperty ("width", c.widthDb, nullptr);
            v.setProperty ("vocal", c.vocalDb, nullptr);
            juce::StringArray ce, to;
            for (int i = 0; i < rp::numBands; ++i)
            {
                ce.add (juce::String (c.centre[(size_t) i], 2));
                to.add (juce::String (c.tol[(size_t) i], 2));
            }
            v.setProperty ("centre", ce.joinIntoString (" "), nullptr);
            v.setProperty ("tol", to.joinIntoString (" "), nullptr);
            caps.appendChild (v, nullptr);
        }
    }
    state.appendChild (caps, nullptr);

    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, dest);
}

void RefProProcessor::setStateInformation (const void* data, int size)
{
    auto xml = getXmlFromBinary (data, size);
    if (xml == nullptr || ! xml->hasTagName (apvts.state.getType())) return;

    auto vt = juce::ValueTree::fromXml (*xml);
    {
        const juce::ScopedLock sl (capLock);
        captured.clear();
        auto caps = vt.getChildWithName ("Captures");
        captureCounter = (int) caps.getProperty ("counter", 0);
        for (auto v : caps)
        {
            if ((int) captured.size() >= maxCaptures) break;
            TargetData t;
            t.name = v.getProperty ("name", "Capture").toString();
            t.psr = (float) v.getProperty ("psr", 11.0);
            t.widthDb = (float) v.getProperty ("width", -8.0);
            t.vocalDb = (float) v.getProperty ("vocal", -6.0);
            t.builtIn = false;
            auto ce = juce::StringArray::fromTokens (v.getProperty ("centre").toString(), " ", "");
            auto to = juce::StringArray::fromTokens (v.getProperty ("tol").toString(), " ", "");
            for (int i = 0; i < rp::numBands; ++i)
            {
                t.centre[(size_t) i] = i < ce.size() ? ce[i].getFloatValue() : 0.0f;
                t.tol[(size_t) i]    = i < to.size() ? to[i].getFloatValue() : 1.5f;
            }
            captured.push_back (t);
        }
        vt.removeChild (caps, nullptr);
    }
    apvts.replaceState (vt);
    ++targetsVersion;
}

juce::AudioProcessorEditor* RefProProcessor::createEditor() { return new RefProEditor (*this); }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new RefProProcessor(); }
