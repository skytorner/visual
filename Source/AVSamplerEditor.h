#pragma once

#include <JuceHeader.h>
#include "AVSamplerProcessor.h"
#include "CueTimelineComponent.h"
#include "SamplerUiComponents.h"

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
    void ejectVideo();
    void loadVideoAtPath(const juce::String& path, bool resumePlayback);
    void updateMediaPresentation(const juce::String& path, double durationSeconds = 0.0);
    void updateLoopButtonAppearance(bool enabled);

    AVSamplerAudioProcessor& audioProcessor;
    juce::VideoComponent videoComponent { false };
    CueTimelineComponent timelineComponent;
    FxControlBar fxControlBar;
    AssignedPadsComponent assignedPadsComponent;
    juce::Label titleLabel;
    juce::Label sessionInfoLabel;
    juce::Label mediaInfoLabel;
    juce::TextButton loadButton { "Load" };
    SamplerIconButton playButton { "Playback", SamplerIconButton::Icon::play };
    SamplerIconButton loopButton { "Loop", SamplerIconButton::Icon::loop };
    SamplerIconButton ejectButton { "Eject Media", SamplerIconButton::Icon::eject };
    juce::Label positionLabel;
    juce::Label statusLabel;
    std::unique_ptr<juce::FileChooser> fileChooser;
    juce::String loadedVideoPath;
    double previousVideoPosition = 0.0;
    bool displayedLoopState = false;
    bool manualPauseRequested = false;
    bool previousVideoWasPlaying = false;
    bool loopRestartPending = false;
    int loopRestartWaitTicks = 0;
    int activeCueIndex = -1;
    bool displayedPlaybackRequested = true;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AVSamplerAudioProcessorEditor)
};
