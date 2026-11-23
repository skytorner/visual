#include "AVSamplerEditor.h"
#include "CueRouting.h"

namespace
{
constexpr int editorWidth = 820;
constexpr int editorHeight = 610;
constexpr double videoEndDetectionWindowSeconds = 0.25;
constexpr double loopStartPositionToleranceSeconds = 0.1;
constexpr int loopRestartDelayTicks = 3;
constexpr int loopSeekRetryTicks = 30;
}

AVSamplerAudioProcessorEditor::AVSamplerAudioProcessorEditor(AVSamplerAudioProcessor& samplerProcessor)
    : AudioProcessorEditor(&samplerProcessor), audioProcessor(samplerProcessor)
{
    setSize(editorWidth, editorHeight);
    addAndMakeVisible(videoComponent);
    addAndMakeVisible(loadButton);
    addAndMakeVisible(playButton);
    addAndMakeVisible(loopButton);
    addAndMakeVisible(positionLabel);
    addAndMakeVisible(statusLabel);

    loadButton.onClick = [this] { chooseVideo(); };
    playButton.onClick = [this]
    {
        if (! videoComponent.isVideoOpen())
            return;

        if (videoComponent.isPlaying())
        {
            manualPauseRequested = true;
            videoComponent.stop();
        }
        else
        {
            manualPauseRequested = false;
            videoComponent.play();
        }
    };

    loopButton.setClickingTogglesState(true);
    loopButton.onClick = [this]
    {
        displayedLoopState = loopButton.getToggleState();
        audioProcessor.setLoopEnabled(displayedLoopState);
    };

    positionLabel.setJustificationType(juce::Justification::centredRight);
    statusLabel.setJustificationType(juce::Justification::centredLeft);
    statusLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);

    for (int i = 0; i < AVSamplerAudioProcessor::cueCount; ++i)
    {
        auto& button = cueButtons[static_cast<std::size_t>(i)];
        auto& label = cueLabels[static_cast<std::size_t>(i)];
        const auto midiNote = avsampler::defaultCueMidiNotes[static_cast<std::size_t>(i)];

        button.setButtonText("Set Cue " + juce::String(i + 1) + " (MIDI " + juce::String(midiNote) + ")");
        button.onClick = [this, i]
        {
            if (videoComponent.isVideoOpen())
                audioProcessor.setCuePosition(i, videoComponent.getPlayPosition());
        };
        label.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(button);
        addAndMakeVisible(label);
    }

    videoComponent.onErrorOccurred = [safeThis = juce::Component::SafePointer<AVSamplerAudioProcessorEditor>(this)]
        (const juce::String& message)
    {
        if (safeThis != nullptr)
            safeThis->statusLabel.setText("Video error: " + message, juce::dontSendNotification);
    };

    const auto state = audioProcessor.getProjectState();
    displayedLoopState = state.loopEnabled;
    loopButton.setToggleState(displayedLoopState, juce::dontSendNotification);
    updateCueLabels(state);
    loadVideoAtPath(state.videoPath);
    startTimerHz(60);
}

void AVSamplerAudioProcessorEditor::paint(juce::Graphics& graphics)
{
    graphics.fillAll(juce::Colour(0xff111217));
    graphics.setColour(juce::Colour(0xff292d38));
    graphics.drawRect(videoComponent.getBounds(), 1);
}

void AVSamplerAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(12);
    auto toolbar = bounds.removeFromTop(34);
    loadButton.setBounds(toolbar.removeFromLeft(140));
    toolbar.removeFromLeft(8);
    playButton.setBounds(toolbar.removeFromLeft(86));
    toolbar.removeFromLeft(8);
    loopButton.setBounds(toolbar.removeFromLeft(86));
    positionLabel.setBounds(toolbar);

    bounds.removeFromTop(8);
    auto cueArea = bounds.removeFromBottom(112);
    statusLabel.setBounds(bounds.removeFromBottom(26));
    bounds.removeFromBottom(6);
    videoComponent.setBounds(bounds);

    const auto cellWidth = cueArea.getWidth() / AVSamplerAudioProcessor::cueCount;
    for (int i = 0; i < AVSamplerAudioProcessor::cueCount; ++i)
    {
        auto cell = cueArea.removeFromLeft(cellWidth).reduced(4, 0);
        cueButtons[static_cast<std::size_t>(i)].setBounds(cell.removeFromTop(38));
        cell.removeFromTop(6);
        cueLabels[static_cast<std::size_t>(i)].setBounds(cell.removeFromTop(28));
    }
}

void AVSamplerAudioProcessorEditor::timerCallback()
{
    const auto state = audioProcessor.getProjectState();
    if (state.videoPath != loadedVideoPath)
        loadVideoAtPath(state.videoPath);

    if (state.loopEnabled != displayedLoopState)
    {
        displayedLoopState = state.loopEnabled;
        loopButton.setToggleState(displayedLoopState, juce::dontSendNotification);
    }

    updateCueLabels(state);

    if (videoComponent.isVideoOpen())
    {
        const auto pendingCue = audioProcessor.consumePendingCue();
        if (pendingCue >= 0 && pendingCue < AVSamplerAudioProcessor::cueCount)
        {
            loopRestartPending = false;
            loopRestartWaitTicks = 0;
            manualPauseRequested = false;
            const auto cuePosition = state.cuePositionsSeconds[static_cast<std::size_t>(pendingCue)];
            videoComponent.setPlayPosition(cuePosition);
            videoComponent.play();

            if (pendingCue != activeCueIndex)
            {
                if (activeCueIndex >= 0)
                    cueButtons[static_cast<std::size_t>(activeCueIndex)].setColour(
                        juce::TextButton::buttonColourId, juce::Colour(0xff292d38));

                activeCueIndex = pendingCue;
                cueButtons[static_cast<std::size_t>(activeCueIndex)].setColour(
                    juce::TextButton::buttonColourId, juce::Colour(0xffb14c2c));
            }

            statusLabel.setText("Triggered Cue " + juce::String(pendingCue + 1)
                + " (MIDI " + juce::String(avsampler::defaultCueMidiNotes[static_cast<std::size_t>(pendingCue)]) + ")",
                juce::dontSendNotification);
        }

        const auto duration = videoComponent.getVideoDuration();
        auto position = videoComponent.getPlayPosition();
        auto isPlaying = videoComponent.isPlaying();

        // JUCE's macOS backend stops and seeks to zero at EOF. Use the last running
        // position to distinguish that stop from a manual pause, then wait for the
        // asynchronous seek to finish before starting playback again.
        if (! state.loopEnabled)
        {
            loopRestartPending = false;
            loopRestartWaitTicks = 0;
        }
        else if (pendingCue < 0 && ! loopRestartPending && previousVideoWasPlaying
                 && ! isPlaying && ! manualPauseRequested && duration > 0.0
                 && previousVideoPosition >= duration - videoEndDetectionWindowSeconds)
        {
            loopRestartPending = true;
            loopRestartWaitTicks = 0;
            if (position > loopStartPositionToleranceSeconds)
                videoComponent.setPlayPosition(0.0);
        }

        if (loopRestartPending)
        {
            if (isPlaying)
            {
                loopRestartPending = false;
                loopRestartWaitTicks = 0;
            }
            else if (position <= loopStartPositionToleranceSeconds)
            {
                if (++loopRestartWaitTicks >= loopRestartDelayTicks)
                {
                    videoComponent.play();
                    loopRestartWaitTicks = 0;
                }
            }
            else if (++loopRestartWaitTicks == loopSeekRetryTicks)
            {
                videoComponent.setPlayPosition(0.0);
                loopRestartWaitTicks = 0;
            }
        }

        position = videoComponent.getPlayPosition();
        isPlaying = videoComponent.isPlaying();
        previousVideoPosition = position;
        previousVideoWasPlaying = isPlaying;
        positionLabel.setText(juce::String(position, 2) + " / " + juce::String(duration, 2) + " s",
                              juce::dontSendNotification);
        playButton.setButtonText(isPlaying ? "Pause" : "Play");
    }
    else
    {
        previousVideoWasPlaying = false;
        loopRestartPending = false;
        loopRestartWaitTicks = 0;
        positionLabel.setText("-- / -- s", juce::dontSendNotification);
        playButton.setButtonText("Play");
    }
}

void AVSamplerAudioProcessorEditor::chooseVideo()
{
    fileChooser = std::make_unique<juce::FileChooser>(
        "Choose a video", juce::File::getSpecialLocation(juce::File::userMoviesDirectory),
        "*.mov;*.mp4;*.m4v;*.avi");

    const juce::Component::SafePointer<AVSamplerAudioProcessorEditor> safeThis(this);
    fileChooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        [safeThis](const juce::FileChooser& chooser)
        {
            if (safeThis == nullptr)
                return;

            const auto file = chooser.getResult();
            if (file.existsAsFile())
                safeThis->loadVideoAtPath(file.getFullPathName());
        });
}

void AVSamplerAudioProcessorEditor::loadVideoAtPath(const juce::String& path)
{
    loadedVideoPath = path;
    previousVideoPosition = 0.0;
    previousVideoWasPlaying = false;
    manualPauseRequested = false;
    loopRestartPending = false;
    loopRestartWaitTicks = 0;
    videoComponent.closeVideo();

    if (path.isEmpty())
    {
        statusLabel.setText("Load a local video. MIDI 36, 38, 40 and 41 trigger Cue 1–4.",
                            juce::dontSendNotification);
        return;
    }

    const juce::File file(path);
    if (! file.existsAsFile())
    {
        statusLabel.setText("Video not found. Load the file again to relink it.", juce::dontSendNotification);
        return;
    }

    audioProcessor.setVideoPath(path);
    statusLabel.setText("Opening video…", juce::dontSendNotification);
    const juce::Component::SafePointer<AVSamplerAudioProcessorEditor> safeThis(this);
    videoComponent.loadAsync(juce::URL(file), [safeThis](const juce::URL&, juce::Result result)
    {
        if (safeThis == nullptr)
            return;

        if (result.failed())
        {
            safeThis->statusLabel.setText("Could not open video: " + result.getErrorMessage(),
                                          juce::dontSendNotification);
            return;
        }

        safeThis->videoComponent.setAudioVolume(0.0f);
        safeThis->videoComponent.play();
        safeThis->previousVideoPosition = 0.0;
        safeThis->previousVideoWasPlaying = true;
        safeThis->manualPauseRequested = false;
        safeThis->statusLabel.setText("Video playing. Set Cue buttons at the current position.",
                                      juce::dontSendNotification);
    });
}

void AVSamplerAudioProcessorEditor::updateCueLabels(const AVSamplerAudioProcessor::ProjectState& state)
{
    for (int i = 0; i < AVSamplerAudioProcessor::cueCount; ++i)
    {
        const auto seconds = state.cuePositionsSeconds[static_cast<std::size_t>(i)];
        cueLabels[static_cast<std::size_t>(i)].setText("MIDI "
            + juce::String(avsampler::defaultCueMidiNotes[static_cast<std::size_t>(i)])
            + "  ·  " + juce::String(seconds, 2) + " s", juce::dontSendNotification);
    }
}
