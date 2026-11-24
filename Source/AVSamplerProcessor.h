#pragma once

#include <JuceHeader.h>
#include "CueRouting.h"
#include <array>
#include <atomic>
#include <cstdint>

class AVSamplerAudioProcessor final : public juce::AudioProcessor
{
public:
    static constexpr int cueCount = 4;

    struct ProjectState
    {
        juce::String videoPath;
        std::array<double, cueCount> cuePositionsSeconds {};
        bool loopEnabled = false;
        bool playbackRequested = true;
        bool rgbSplitEnabled = true;
        bool flashEnabled = false;
        bool zoomPunchEnabled = false;
        int selectedCueIndex = 0;
        int speedMidiCcValue = avsampler::defaultVideoSpeedControllerValue;
    };

    AVSamplerAudioProcessor();
    ~AVSamplerAudioProcessor() override = default;

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

    ProjectState getProjectState() const;
    void setVideoPath(const juce::String& path);
    void setCuePosition(int cueIndex, double positionSeconds);
    void setLoopEnabled(bool shouldLoop);
    void setPlaybackRequested(bool shouldPlay);
    void setRgbSplitEnabled(bool shouldEnable);
    void setFlashEnabled(bool shouldEnable);
    void setZoomPunchEnabled(bool shouldEnable);
    void setSelectedCueIndex(int cueIndex);
    int consumePendingCue() noexcept;
    std::uint32_t getLatestCueEffectTrigger() const noexcept;

private:
    mutable juce::CriticalSection projectStateLock;
    ProjectState projectState;
    std::atomic<int> pendingCueIndex { -1 };
    std::atomic<int> speedMidiCcValue { avsampler::defaultVideoSpeedControllerValue };
    std::atomic<std::uint32_t> latestCueEffectTrigger { 0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AVSamplerAudioProcessor)
};
