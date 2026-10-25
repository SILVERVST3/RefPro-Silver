#pragma once

#include "Common.h"
#include "Analyzer.h"
#include "Targets.h"

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

    juce::AudioProcessorValueTreeState apvts;
    TonalAnalyzer analyzer;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

    mutable juce::CriticalSection capLock;
    std::vector<TargetData> captured;
    int captureCounter = 0;
    TargetData allPurpose;
    std::atomic<float>* avgParam = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RefProProcessor)
};
