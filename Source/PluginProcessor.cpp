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

    p.push_back (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { "dynOn", 1 }, "Dynamic EQ On", false));
    p.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "dynAmount", 1 }, "Dynamic Amount",
                    juce::NormalisableRange<float> (0.0f, 100.0f, 1.0f), 50.0f,
                    juce::AudioParameterFloatAttributes().withStringFromValueFunction ([] (float v, int) { return juce::String ((int) std::round (v)) + " %"; })
                                                          .withValueFromStringFunction ([] (const juce::String& s) { return s.getFloatValue(); })));
    p.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "dynMax", 1 }, "Dynamic Max Cut",
                    juce::NormalisableRange<float> (0.0f, 12.0f, 0.1f), 4.0f,
                    juce::AudioParameterFloatAttributes().withStringFromValueFunction ([] (float v, int) { return juce::String (v, 1) + " dB"; })
                                                          .withValueFromStringFunction ([] (const juce::String& s) { return s.getFloatValue(); })));
    p.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "dynTol", 1 }, "Dynamic Tolerance",
                    juce::NormalisableRange<float> (0.0f, 6.0f, 0.1f), 0.5f,
                    juce::AudioParameterFloatAttributes().withStringFromValueFunction ([] (float v, int) { return "+" + juce::String (v, 1) + " dB"; })
                                                          .withValueFromStringFunction ([] (const juce::String& s) { return s.getFloatValue(); })));
    p.push_back (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { "dynSpeed", 1 }, "Dynamic Speed",
                    juce::StringArray { "Fast", "Medium", "Slow" }, 1));

    auto freqText = [] (float v, int) { return v >= 1000.0f ? juce::String (v / 1000.0f, 2) + " kHz" : juce::String ((int) std::round (v)) + " Hz"; };
    auto freqVal  = [] (const juce::String& s)
    {
        const auto l = s.trim().toLowerCase();
        float v = l.getFloatValue();
        if (l.contains ("k")) v *= 1000.0f;
        return v;
    };

    {
        juce::NormalisableRange<float> fr (20.0f, 20000.0f, 1.0f);
        fr.setSkewForCentre (1000.0f);
        p.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "dynLo", 1 }, "Dynamic Low Limit", fr, 20.0f,
                        juce::AudioParameterFloatAttributes().withStringFromValueFunction (freqText).withValueFromStringFunction (freqVal)));
        p.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "dynHi", 1 }, "Dynamic High Limit", fr, 20000.0f,
                        juce::AudioParameterFloatAttributes().withStringFromValueFunction (freqText).withValueFromStringFunction (freqVal)));
    }

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
    for (auto& v : dynGainDb) v.store (0.0f);
    for (auto& v : dynTc) v.store (0.0f);
    for (auto& v : dynTt) v.store (1.5f);
    detector.setVocalEnabled (false);
    dynOnParam     = apvts.getRawParameterValue ("dynOn");
    dynAmountParam = apvts.getRawParameterValue ("dynAmount");
    dynMaxParam    = apvts.getRawParameterValue ("dynMax");
    dynTolParam    = apvts.getRawParameterValue ("dynTol");
    dynSpeedParam  = apvts.getRawParameterValue ("dynSpeed");
    dynLoParam     = apvts.getRawParameterValue ("dynLo");
    dynHiParam     = apvts.getRawParameterValue ("dynHi");
    apvts.addParameterListener ("target", this);
    publishDynTarget();
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

RefProProcessor::~RefProProcessor()
{
    apvts.removeParameterListener ("target", this);
    cancelPendingUpdate();
}

void RefProProcessor::publishDynTarget()
{
    const int idx = (int) std::round (apvts.getRawParameterValue ("target")->load());
    const auto t = getTarget (idx);
    for (int i = 0; i < rp::numBands; ++i)
    {
        dynTc[(size_t) i].store (t.centre[(size_t) i]);
        dynTt[(size_t) i].store (t.tol[(size_t) i]);
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
    detector.prepare (sampleRate);
    dynWasOn = false;
    for (auto& d : dyn) d = DynBand {};
    for (auto& v : dynGainDb) v.store (0.0f);
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

    runDynamic (buffer, n, numCh);

    if (post) analyse();
}


// ---------------------------------------------------------------- EQ dinâmico
// Compara (média de alguns segundos) o espectro medido na ENTRADA deste estágio com o alvo.
// Só o que passar da faixa de tolerância é atenuado, proporcionalmente (Amount) e limitado (Max Cut).
void RefProProcessor::runDynamic (juce::AudioBuffer<float>& buffer, int n, int numCh)
{
    if (dynOnParam->load() < 0.5f)
    {
        if (dynWasOn)
        {
            dynWasOn = false;
            detector.requestReset();
            for (int j = 0; j < dynBands; ++j) { dyn[(size_t) j] = DynBand {}; dynGainDb[(size_t) j].store (0.0f); }
        }
        return;
    }
    if (! dynWasOn) { dynWasOn = true; detector.requestReset(); }

    static const float taus[] = { 1.5f, 3.0f, 6.0f }, atks[] = { 0.15f, 0.3f, 0.6f }, rels[] = { 0.5f, 1.0f, 2.0f };
    const int sp = juce::jlimit (0, 2, (int) dynSpeedParam->load());
    detector.setAveragingSeconds (taus[sp]);

    const float* l = buffer.getReadPointer (0);
    const float* r = buffer.getNumChannels() > 1 ? buffer.getReadPointer (1) : l;
    detector.process (l, r, n);

    const float amount = dynAmountParam->load() * 0.01f, maxCut = dynMaxParam->load(), extraTol = dynTolParam->load();
    const float lo = dynLoParam->load(), hi = dynHiParam->load();

    std::array<float, dynBands> raw {}, tgt {};
    if (detector.hasData.load())
    {
        for (int j = 0; j < dynBands; ++j)
        {
            const float fc = dynFreq (j);
            if (fc < lo * 0.8f || fc > hi * 1.25f) continue;
            float dev = 0.0f;
            for (int k = 4 * j - 2; k <= 4 * j + 2; ++k)
            {
                const auto kk = (size_t) juce::jlimit (0, rp::numBands - 1, k);
                dev += detector.avgDb[kk].load() - dynTc[kk].load();
            }
            dev /= 5.0f;
            const float tol = dynTt[(size_t) juce::jlimit (0, rp::numBands - 1, 4 * j)].load();
            const float excess = dev - (tol + extraTol);
            if (excess > 0.0f) raw[(size_t) j] = -std::min (maxCut, amount * excess);
        }
    }
    for (int j = 0; j < dynBands; ++j)
    {
        const float a = raw[(size_t) std::max (j - 1, 0)], b = raw[(size_t) j], c = raw[(size_t) std::min (j + 1, dynBands - 1)];
        tgt[(size_t) j] = 0.25f * a + 0.5f * b + 0.25f * c;
    }

    const float dt = (float) n / (float) currentSampleRate;
    const float aA = 1.0f - std::exp (-dt / atks[sp]), aR = 1.0f - std::exp (-dt / rels[sp]);

    for (int j = 0; j < dynBands; ++j)
    {
        auto& d = dyn[(size_t) j];
        float g = d.gainDb;
        const float target = tgt[(size_t) j];
        g += (target < g ? aA : aR) * (target - g);
        d.gainDb = g;
        dynGainDb[(size_t) j].store (g);

        const float fc = dynFreq (j);
        const bool act = g < -0.05f && fc < (float) currentSampleRate * 0.45f;
        if (! act)
        {
            if (d.active) { d.active = false; d.z1[0] = d.z1[1] = d.z2[0] = d.z2[1] = 0.0; }
            continue;
        }
        if (! d.active || std::abs (g - d.coefGain) > 0.02f)
        {
            Eq::Params pp;
            pp.on = true; pp.type = Eq::Bell; pp.freq = fc; pp.gain = g; pp.q = 4.3f;
            d.c = Eq::make (pp, currentSampleRate);
            d.coefGain = g;
        }
        d.active = true;

        const auto& c = d.c;
        for (int ch = 0; ch < numCh; ++ch)
        {
            float* x = buffer.getWritePointer (ch);
            double z1 = d.z1[ch], z2 = d.z2[ch];
            for (int i = 0; i < n; ++i)
            {
                const double in = x[i];
                const double y = c.b0 * in + z1;
                z1 = c.b1 * in - c.a1 * y + z2;
                z2 = c.b2 * in - c.a2 * y;
                x[i] = (float) y;
            }
            d.z1[ch] = z1; d.z2[ch] = z2;
        }
    }
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
    triggerAsyncUpdate();
    return CaptureResult::ok;
}

void RefProProcessor::deleteCapture (int targetIndex)
{
    const juce::ScopedLock sl (capLock);
    if (targetIndex >= 1 && targetIndex <= (int) captured.size())
    {
        captured.erase (captured.begin() + (targetIndex - 1));
        ++targetsVersion;
        triggerAsyncUpdate();
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
    triggerAsyncUpdate();
}

juce::AudioProcessorEditor* RefProProcessor::createEditor() { return new RefProEditor (*this); }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new RefProProcessor(); }
