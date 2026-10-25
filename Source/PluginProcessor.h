#pragma once

#include "Common.h"
#include "Analyzer.h"
#include "Targets.h"
#include "Eq.h"

class RefProProcessor : public juce::AudioProcessor
{
public:
    RefProProcessor();
    ~RefProProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    using juce::AudioProcessor::processBlock;
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "RefPro SILVER"; }
    bool acceptsMidi() const override  { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    // --- alvos ---------------------------------------------------------
    static constexpr int maxCaptures = 8;
    enum class CaptureResult { ok, noData, full };

    int        getNumTargets() const;
    TargetData getTarget (int index) const;
    CaptureResult captureCurrent();            // thread de mensagem
    void       deleteCapture (int targetIndex);
    std::atomic<int> targetsVersion { 0 };

    // --- equalizador ---------------------------------------------------
    static juce::String bandId (int band, const char* what) { return "b" + juce::String (band + 1) + "_" + what; }
    Eq::Params getBandParams (int band) const;

    juce::AudioProcessorValueTreeState apvts;
    TonalAnalyzer analyzer;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

    mutable juce::CriticalSection capLock;
    std::vector<TargetData> captured;
    int captureCounter = 0;
    TargetData allPurpose;
    std::atomic<float>* avgParam = nullptr;
    std::atomic<float>* eqOnParam = nullptr;
    std::atomic<float>* postParam = nullptr;
    std::array<std::atomic<float>*, Eq::numBands> pOn {}, pType {}, pFreq {}, pGain {}, pQ {};

    struct BandState
    {
        Eq::Params last;
        Eq::Coeffs c;
        bool active = false;
        double z1[2] { 0, 0 }, z2[2] { 0, 0 };
    };
    std::array<BandState, Eq::numBands> eq;
    double currentSampleRate = 44100.0;
    bool forceEqUpdate = true;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RefProProcessor)
};
