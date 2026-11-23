#pragma once

#include <JuceHeader.h>
#include "AVSamplerProcessor.h"

class AVSamplerAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                             private juce::Timer
{
public:
    explicit AVSamplerAudioProcessorEditor(AVSamplerAudioProcessor& samplerProcessor);
    ~AVSamplerAudioProcessorEditor() override = default;

    void paint(juce::Graphics& graphics) override;
    void resized() override;

private:
    void timerCallback() override;
    void chooseVideo();
    void loadVideoAtPath(const juce::String& path);
    void updateCueLabels(const AVSamplerAudioProcessor::ProjectState& state);

    AVSamplerAudioProcessor& audioProcessor;
    juce::VideoComponent videoComponent { false };
    juce::TextButton loadButton { "Load Video" };
    juce::TextButton playButton { "Play" };
    juce::TextButton loopButton { "Loop" };
    juce::Label positionLabel;
    juce::Label statusLabel;
    std::array<juce::TextButton, AVSamplerAudioProcessor::cueCount> cueButtons;
    std::array<juce::Label, AVSamplerAudioProcessor::cueCount> cueLabels;
    std::unique_ptr<juce::FileChooser> fileChooser;
    juce::String loadedVideoPath;
    double previousVideoPosition = 0.0;
    bool displayedLoopState = false;
    bool manualPauseRequested = false;
    bool previousVideoWasPlaying = false;
    bool loopRestartPending = false;
    int loopRestartWaitTicks = 0;
    int activeCueIndex = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AVSamplerAudioProcessorEditor)
};
