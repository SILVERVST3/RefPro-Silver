#include "PluginProcessor.h"
#include "PluginEditor.h"

juce::AudioProcessorValueTreeState::ParameterLayout RefProProcessor::createLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;
    p.push_back (std::make_unique<juce::AudioParameterInt> (juce::ParameterID { "target", 1 }, "Target", 0, maxCaptures, 0));
    p.push_back (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { "avg", 1 }, "Averaging",
                    juce::StringArray { "Short (3 s)", "Medium (10 s)", "Long (30 s)", "Infinite" }, 1));
    return { p.begin(), p.end() };
}

RefProProcessor::RefProProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "RefProSilver", createLayout()),
      allPurpose (Targets::makeAllPurpose())
{
    avgParam = apvts.getRawParameterValue ("avg");
}

void RefProProcessor::prepareToPlay (double sampleRate, int)
{
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

    // O áudio passa intacto (plugin de medição); só analisamos.
    const float* l = buffer.getReadPointer (0);
    const float* r = buffer.getNumChannels() > 1 ? buffer.getReadPointer (1) : l;
    analyzer.process (l, r, n);
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
