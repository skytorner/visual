#include "AVSamplerEditor.h"
#include "CueRouting.h"

namespace
{
constexpr int editorWidth = 820;
constexpr int editorHeight = 740;
constexpr double videoEndDetectionWindowSeconds = 0.25;
constexpr double loopStartPositionToleranceSeconds = 0.1;
constexpr int loopRestartDelayTicks = 3;
constexpr int loopSeekRetryTicks = 30;
}

AVSamplerAudioProcessorEditor::AVSamplerAudioProcessorEditor(AVSamplerAudioProcessor& samplerProcessor)
    : AudioProcessorEditor(&samplerProcessor), audioProcessor(samplerProcessor)
{
    setSize(editorWidth, editorHeight);
    setResizable(true, true);
    setResizeLimits(560, 540, 1400, 1000);
    addAndMakeVisible(videoComponent);
    addAndMakeVisible(timelineComponent);
    addAndMakeVisible(titleLabel);
    addAndMakeVisible(sessionInfoLabel);
    addAndMakeVisible(mediaInfoLabel);
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
            displayedPlaybackRequested = false;
            audioProcessor.setPlaybackRequested(false);
        }
        else
        {
            manualPauseRequested = false;
            videoComponent.play();
            displayedPlaybackRequested = true;
            audioProcessor.setPlaybackRequested(true);
        }
    };

    loopButton.setClickingTogglesState(true);
    loopButton.onClick = [this]
    {
        displayedLoopState = loopButton.getToggleState();
        audioProcessor.setLoopEnabled(displayedLoopState);
        updateLoopButtonAppearance(displayedLoopState);
    };

    positionLabel.setJustificationType(juce::Justification::centredRight);
    statusLabel.setJustificationType(juce::Justification::centredLeft);
    statusLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);

    timelineComponent.onCuePositionChanged = [this](int cueIndex, double positionSeconds)
    {
        audioProcessor.setCuePosition(cueIndex, positionSeconds);
    };
    timelineComponent.onCueSelected = [this](int cueIndex)
    {
        audioProcessor.setSelectedCueIndex(cueIndex);
    };

    titleLabel.setText("AV SAMPLER", juce::dontSendNotification);
    titleLabel.setFont(juce::FontOptions(16.0f).withStyle("Bold"));
    titleLabel.setColour(juce::Label::textColourId, juce::Colour(0xffe7edf6));
    sessionInfoLabel.setText("SESSION STATE  ·  HOST PROJECT", juce::dontSendNotification);
    sessionInfoLabel.setJustificationType(juce::Justification::centredRight);
    sessionInfoLabel.setColour(juce::Label::textColourId, juce::Colour(0xff8793a6));
    mediaInfoLabel.setJustificationType(juce::Justification::centredLeft);
    mediaInfoLabel.setColour(juce::Label::textColourId, juce::Colour(0xffd4dbe7));

    videoComponent.onErrorOccurred = [safeThis = juce::Component::SafePointer<AVSamplerAudioProcessorEditor>(this)]
        (const juce::String& message)
    {
        if (safeThis != nullptr)
            safeThis->statusLabel.setText("Video error: " + message, juce::dontSendNotification);
    };

    const auto state = audioProcessor.getProjectState();
    displayedLoopState = state.loopEnabled;
    displayedPlaybackRequested = state.playbackRequested;
    updateLoopButtonAppearance(displayedLoopState);
    timelineComponent.setSelectedCue(state.selectedCueIndex);
    playButton.setButtonText(displayedPlaybackRequested ? "Pause" : "Play");
    loadVideoAtPath(state.videoPath, displayedPlaybackRequested);
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
    auto header = bounds.removeFromTop(28);
    titleLabel.setBounds(header.removeFromLeft(160));
    sessionInfoLabel.setBounds(header);
    bounds.removeFromTop(5);
    auto toolbar = bounds.removeFromTop(36);
    loadButton.setBounds(toolbar.removeFromLeft(118));
    toolbar.removeFromLeft(5);
    playButton.setBounds(toolbar.removeFromLeft(70));
    toolbar.removeFromLeft(5);
    loopButton.setBounds(toolbar.removeFromLeft(88));
    toolbar.removeFromLeft(6);
    positionLabel.setBounds(toolbar.removeFromRight(115));
    mediaInfoLabel.setBounds(toolbar);

    bounds.removeFromTop(8);
    auto timelineArea = bounds.removeFromBottom(178);
    bounds.removeFromBottom(6);
    statusLabel.setBounds(bounds.removeFromBottom(26));
    bounds.removeFromBottom(6);
    videoComponent.setBounds(bounds);
    timelineComponent.setBounds(timelineArea);
}

void AVSamplerAudioProcessorEditor::timerCallback()
{
    const auto state = audioProcessor.getProjectState();
    if (state.videoPath != loadedVideoPath)
        loadVideoAtPath(state.videoPath, state.playbackRequested);

    if (state.loopEnabled != displayedLoopState)
    {
        displayedLoopState = state.loopEnabled;
        updateLoopButtonAppearance(displayedLoopState);
    }

    timelineComponent.setSelectedCue(state.selectedCueIndex);

    if (videoComponent.isVideoOpen())
    {
        if (state.playbackRequested != displayedPlaybackRequested)
        {
            displayedPlaybackRequested = state.playbackRequested;
            manualPauseRequested = ! state.playbackRequested;
            if (state.playbackRequested)
                videoComponent.play();
            else
                videoComponent.stop();
        }

        const auto pendingCue = audioProcessor.consumePendingCue();
        if (pendingCue >= 0 && pendingCue < AVSamplerAudioProcessor::cueCount)
        {
            loopRestartPending = false;
            loopRestartWaitTicks = 0;
            manualPauseRequested = false;
            const auto cuePosition = state.cuePositionsSeconds[static_cast<std::size_t>(pendingCue)];
            videoComponent.setPlayPosition(cuePosition);
            videoComponent.play();
            displayedPlaybackRequested = true;
            audioProcessor.setPlaybackRequested(true);

            activeCueIndex = pendingCue;
            timelineComponent.setActiveCue(activeCueIndex);
            timelineComponent.setSelectedCue(pendingCue);
            audioProcessor.setSelectedCueIndex(pendingCue);

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

        if (! state.loopEnabled && pendingCue < 0 && previousVideoWasPlaying && ! isPlaying
            && ! manualPauseRequested && duration > 0.0
            && previousVideoPosition >= duration - videoEndDetectionWindowSeconds)
        {
            displayedPlaybackRequested = false;
            audioProcessor.setPlaybackRequested(false);
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
        timelineComponent.setTimelineData(duration, position, state.cuePositionsSeconds);
    }
    else
    {
        previousVideoWasPlaying = false;
        loopRestartPending = false;
        loopRestartWaitTicks = 0;
        positionLabel.setText("-- / -- s", juce::dontSendNotification);
        playButton.setButtonText("Play");
        timelineComponent.setTimelineData(0.0, 0.0, state.cuePositionsSeconds);
    }
}

void AVSamplerAudioProcessorEditor::chooseVideo()
{
    const auto currentPath = audioProcessor.getProjectState().videoPath;
    const auto currentFileExists = currentPath.isNotEmpty() && juce::File(currentPath).existsAsFile();
    const auto title = currentPath.isEmpty() ? "Load media"
        : currentFileExists ? "Replace media" : "Relink missing media";
    fileChooser = std::make_unique<juce::FileChooser>(
        title, juce::File::getSpecialLocation(juce::File::userMoviesDirectory),
        "*.mov;*.mp4;*.m4v;*.avi");

    const juce::Component::SafePointer<AVSamplerAudioProcessorEditor> safeThis(this);
    fileChooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        [safeThis](const juce::FileChooser& chooser)
        {
            if (safeThis == nullptr)
                return;

            const auto file = chooser.getResult();
            if (file.existsAsFile())
            {
                const auto state = safeThis->audioProcessor.getProjectState();
                safeThis->loadVideoAtPath(file.getFullPathName(), state.playbackRequested);
            }
        });
}

void AVSamplerAudioProcessorEditor::loadVideoAtPath(const juce::String& path, bool resumePlayback)
{
    loadedVideoPath = path;
    updateMediaPresentation(path);
    previousVideoPosition = 0.0;
    previousVideoWasPlaying = false;
    manualPauseRequested = false;
    loopRestartPending = false;
    loopRestartWaitTicks = 0;
    videoComponent.closeVideo();

    if (path.isEmpty())
    {
        statusLabel.setText("Load local media. Drag timeline markers A–D to position Cue Points.",
                            juce::dontSendNotification);
        return;
    }

    const juce::File file(path);
    if (! file.existsAsFile())
    {
        statusLabel.setText("Media is missing. Relink it to keep the saved Cue timings.", juce::dontSendNotification);
        return;
    }

    audioProcessor.setVideoPath(path);
    audioProcessor.setPlaybackRequested(resumePlayback);
    statusLabel.setText("Opening video…", juce::dontSendNotification);
    const juce::Component::SafePointer<AVSamplerAudioProcessorEditor> safeThis(this);
    videoComponent.loadAsync(juce::URL(file), [safeThis, resumePlayback](const juce::URL&, juce::Result result)
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
        if (resumePlayback)
            safeThis->videoComponent.play();
        safeThis->previousVideoPosition = 0.0;
        safeThis->previousVideoWasPlaying = false;
        safeThis->manualPauseRequested = false;
        safeThis->displayedPlaybackRequested = resumePlayback;
        safeThis->playButton.setButtonText(resumePlayback ? "Pause" : "Play");
        safeThis->statusLabel.setText(resumePlayback
                ? "Video playing. Cue timings are stored with the host session."
                : "Video ready. Playback is paused in the host session.",
                                      juce::dontSendNotification);
    });
}

void AVSamplerAudioProcessorEditor::updateMediaPresentation(const juce::String& path)
{
    if (path.isEmpty())
    {
        loadButton.setButtonText("Load Media");
        mediaInfoLabel.setText("No media loaded", juce::dontSendNotification);
        mediaInfoLabel.setColour(juce::Label::textColourId, juce::Colour(0xff8793a6));
        mediaInfoLabel.setTooltip("Choose a local video file for this host session.");
        return;
    }

    const juce::File mediaFile(path);
    mediaInfoLabel.setTooltip(path);
    if (! mediaFile.existsAsFile())
    {
        loadButton.setButtonText("Relink Media");
        mediaInfoLabel.setText("Missing media  ·  " + mediaFile.getFileName(), juce::dontSendNotification);
        mediaInfoLabel.setColour(juce::Label::textColourId, juce::Colour(0xffff8f79));
        return;
    }

    loadButton.setButtonText("Replace Media");
    mediaInfoLabel.setText(mediaFile.getFileName(), juce::dontSendNotification);
    mediaInfoLabel.setColour(juce::Label::textColourId, juce::Colour(0xffd4dbe7));
}

void AVSamplerAudioProcessorEditor::updateLoopButtonAppearance(bool enabled)
{
    loopButton.setToggleState(enabled, juce::dontSendNotification);
    loopButton.setButtonText(enabled ? "Loop ON" : "Loop OFF");
    loopButton.setColour(juce::TextButton::buttonColourId,
                         enabled ? juce::Colour(0xff254a38) : juce::Colour(0xff292d38));
    loopButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff42dc86));
    loopButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffc4ccd8));
    loopButton.setColour(juce::TextButton::textColourOnId, juce::Colours::black);
}
