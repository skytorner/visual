#pragma once

#include <JuceHeader.h>
#include <atomic>

class AudioReactiveImagePOCAudioProcessor final : public juce::AudioProcessor
{
public:
    using APVTS = juce::AudioProcessorValueTreeState;

    static constexpr const char* sensitivityParamId = "sensitivity";
    static constexpr const char* smoothingParamId = "smoothing";
    static constexpr const char* minScaleParamId = "minScale";
    static constexpr const char* maxScaleParamId = "maxScale";
    static constexpr const char* rotationAmountParamId = "rotationAmount";
    static constexpr const char* opacityAmountParamId = "opacityAmount";

    AudioReactiveImagePOCAudioProcessor();
    ~AudioReactiveImagePOCAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    APVTS& getValueTreeState() noexcept { return parameters; }

    float getRmsLevel() const noexcept { return rmsLevel.load(std::memory_order_relaxed); }
    float getPeakLevel() const noexcept { return peakLevel.load(std::memory_order_relaxed); }
    float getEnvelopeLevel() const noexcept { return envelopeLevel.load(std::memory_order_relaxed); }

    void setImagePath(const juce::String& path);
    juce::String getImagePath() const;

    void resetVisualState();

private:
    static APVTS::ParameterLayout createParameterLayout();

    APVTS parameters;

    std::atomic<float> rmsLevel { 0.0f };
    std::atomic<float> peakLevel { 0.0f };
    std::atomic<float> envelopeLevel { 0.0f };
    std::atomic<bool> resetRequested { false };

    float envelopeState = 0.0f;

    mutable juce::CriticalSection imagePathLock;
    juce::String imagePath;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioReactiveImagePOCAudioProcessor)
};
