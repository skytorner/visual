#include "AVSamplerEditor.h"
#include "CueRouting.h"

namespace
{
constexpr int editorWidth = 900;
constexpr int editorHeight = 760;
constexpr double videoEndDetectionWindowSeconds = 0.25;
constexpr double loopStartPositionToleranceSeconds = 0.1;
constexpr int loopRestartDelayTicks = 3;
constexpr int loopSeekRetryTicks = 30;

juce::String fitMediaName(const juce::String& value, float availableWidth)
{
    auto text = value;
    const juce::Font font(juce::FontOptions(14.0f));
    const auto ellipsis = juce::String::fromUTF8("…");
    const auto widthOf = [&font](const juce::String& candidate)
    {
        juce::GlyphArrangement glyphs;
        glyphs.addLineOfText(font, candidate, 0.0f, 0.0f);
        return glyphs.getBoundingBox(0, glyphs.getNumGlyphs(), true).getWidth();
    };
    if (widthOf(text) <= availableWidth)
        return text;

    while (text.isNotEmpty() && widthOf(text + ellipsis) > availableWidth)
        text = text.dropLastCharacters(1);
    return text + ellipsis;
}
}

AVSamplerAudioProcessorEditor::AVSamplerAudioProcessorEditor(AVSamplerAudioProcessor& samplerProcessor)
    : AudioProcessorEditor(&samplerProcessor), audioProcessor(samplerProcessor)
{
    setSize(editorWidth, editorHeight);
    setResizable(true, true);
    setResizeLimits(560, 600, 1400, 1000);
    addAndMakeVisible(videoComponent);
    addAndMakeVisible(timelineComponent);
    addAndMakeVisible(fxControlBar);
    addAndMakeVisible(assignedPadsComponent);
    addAndMakeVisible(titleLabel);
    addAndMakeVisible(sessionInfoLabel);
    addAndMakeVisible(mediaInfoLabel);
    addAndMakeVisible(loadButton);
    addAndMakeVisible(playButton);
    addAndMakeVisible(loopButton);
    addAndMakeVisible(ejectButton);
    addAndMakeVisible(positionLabel);
    addAndMakeVisible(statusLabel);

    loadButton.onClick = [this] { chooseVideo(); };
    loadButton.setTooltip("Load, replace, or relink a local video");
    ejectButton.setTooltip("Eject the current video. Cue points and session settings are kept.");
    playButton.setTooltip("Play or pause the video");
    loopButton.setTooltip("Toggle video looping");
    ejectButton.onClick = [this] { ejectVideo(); };
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
            statusLabel.setText("Ready · Paused", juce::dontSendNotification);
        }
        else
        {
            manualPauseRequested = false;
            videoComponent.play();
            displayedPlaybackRequested = true;
            audioProcessor.setPlaybackRequested(true);
            statusLabel.setText("Ready · Playing", juce::dontSendNotification);
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
    statusLabel.setColour(juce::Label::textColourId, juce::Colour(0xffd4dbe7));
    statusLabel.setColour(juce::Label::backgroundColourId, juce::Colour(0xdd111217));
    statusLabel.setOpaque(true);

    timelineComponent.onCuePositionChanged = [this](int cueIndex, double positionSeconds)
    {
        audioProcessor.setCuePosition(cueIndex, positionSeconds);
    };
    timelineComponent.onCueSelected = [this](int cueIndex)
    {
        audioProcessor.setSelectedCueIndex(cueIndex);
        assignedPadsComponent.setSelectedCue(cueIndex);
    };
    assignedPadsComponent.onCueSelected = [this](int cueIndex)
    {
        audioProcessor.setSelectedCueIndex(cueIndex);
        timelineComponent.setSelectedCue(cueIndex);
    };
    fxControlBar.onRgbSplitEnabledChanged = [this](bool enabled)
    {
        displayedRgbSplitEnabled = enabled;
        audioProcessor.setRgbSplitEnabled(enabled);
        videoComponent.setRgbSplitEnabled(enabled);
    };
    fxControlBar.onFlashEnabledChanged = [this](bool enabled)
    {
        displayedFlashEnabled = enabled;
        audioProcessor.setFlashEnabled(enabled);
        videoComponent.setFlashEnabled(enabled);
    };

    titleLabel.setText("AV SAMPLER", juce::dontSendNotification);
    titleLabel.setFont(juce::FontOptions(16.0f).withStyle("Bold"));
    titleLabel.setColour(juce::Label::textColourId, juce::Colour(0xffe7edf6));
    sessionInfoLabel.setText("SESSION STATE  ·  HOST PROJECT", juce::dontSendNotification);
    sessionInfoLabel.setJustificationType(juce::Justification::centredRight);
    sessionInfoLabel.setColour(juce::Label::textColourId, juce::Colour(0xff8793a6));
    mediaInfoLabel.setJustificationType(juce::Justification::centredLeft);
    mediaInfoLabel.setColour(juce::Label::textColourId, juce::Colour(0xffd4dbe7));
    mediaInfoLabel.setFont(juce::FontOptions(14.0f));

    videoComponent.onErrorOccurred = [safeThis = juce::Component::SafePointer<AVSamplerAudioProcessorEditor>(this)]
        (const juce::String& message)
    {
        if (safeThis != nullptr)
            safeThis->statusLabel.setText("Video error: " + message, juce::dontSendNotification);
    };

    const auto state = audioProcessor.getProjectState();
    displayedLoopState = state.loopEnabled;
    displayedPlaybackRequested = state.playbackRequested;
    displayedRgbSplitEnabled = state.rgbSplitEnabled;
    displayedFlashEnabled = state.flashEnabled;
    lastSeenCueEffectTrigger = audioProcessor.getLatestCueEffectTrigger();
    fxControlBar.setRgbSplitAvailable(videoComponent.isRgbSplitAvailable());
    fxControlBar.setFlashAvailable(videoComponent.isFlashAvailable());
    fxControlBar.setRgbSplitEnabled(displayedRgbSplitEnabled);
    fxControlBar.setFlashEnabled(displayedFlashEnabled);
    videoComponent.setRgbSplitEnabled(displayedRgbSplitEnabled);
    videoComponent.setFlashEnabled(displayedFlashEnabled);
    fxControlBar.setSpeedMultiplier(
        avsampler::videoSpeedMultiplierForControllerValue(state.speedMidiCcValue));
    updateLoopButtonAppearance(displayedLoopState);
    timelineComponent.setSelectedCue(state.selectedCueIndex);
    assignedPadsComponent.setCueData(state.cuePositionsSeconds, state.selectedCueIndex);
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
    auto header = bounds.removeFromTop(22);
    titleLabel.setBounds(header.removeFromLeft(190));
    sessionInfoLabel.setBounds(header);
    bounds.removeFromTop(5);
    auto toolbar = bounds.removeFromTop(40);
    playButton.setBounds(toolbar.removeFromLeft(38));
    toolbar.removeFromLeft(6);
    loopButton.setBounds(toolbar.removeFromLeft(38));
    toolbar.removeFromLeft(8);
    loadButton.setBounds(toolbar.removeFromRight(76));
    toolbar.removeFromRight(5);
    ejectButton.setBounds(toolbar.removeFromRight(34));
    toolbar.removeFromRight(6);
    positionLabel.setBounds(toolbar.removeFromRight(82));
    toolbar.removeFromRight(8);
    mediaInfoLabel.setBounds(toolbar);

    bounds.removeFromTop(7);
    auto pads = bounds.removeFromBottom(78);
    bounds.removeFromBottom(6);
    auto effects = bounds.removeFromBottom(74);
    bounds.removeFromBottom(6);
    auto timelineArea = bounds.removeFromBottom(122);
    bounds.removeFromBottom(6);
    videoComponent.setBounds(bounds);
    statusLabel.setBounds(bounds.reduced(8).withY(bounds.getBottom() - 26).withHeight(18));
    timelineComponent.setBounds(timelineArea);
    fxControlBar.setBounds(effects);
    assignedPadsComponent.setBounds(pads);
    updateMediaPresentation(loadedVideoPath, videoComponent.isVideoOpen()
        ? videoComponent.getVideoDuration() : 0.0);
}

void AVSamplerAudioProcessorEditor::timerCallback()
{
    const auto state = audioProcessor.getProjectState();
    if (state.speedMidiCcValue != lastAppliedSpeedMidiCcValue)
    {
        const auto speedMultiplier = avsampler::videoSpeedMultiplierForControllerValue(state.speedMidiCcValue);
        fxControlBar.setSpeedMultiplier(speedMultiplier);
        if (videoComponent.isVideoOpen())
            videoComponent.setPlaySpeed(speedMultiplier);
        lastAppliedSpeedMidiCcValue = state.speedMidiCcValue;
    }

    if (state.videoPath != loadedVideoPath)
        loadVideoAtPath(state.videoPath, state.playbackRequested);

    if (state.loopEnabled != displayedLoopState)
    {
        displayedLoopState = state.loopEnabled;
        updateLoopButtonAppearance(displayedLoopState);
    }

    if (state.rgbSplitEnabled != displayedRgbSplitEnabled)
    {
        displayedRgbSplitEnabled = state.rgbSplitEnabled;
        fxControlBar.setRgbSplitEnabled(displayedRgbSplitEnabled);
        videoComponent.setRgbSplitEnabled(displayedRgbSplitEnabled);
    }

    if (state.flashEnabled != displayedFlashEnabled)
    {
        displayedFlashEnabled = state.flashEnabled;
        fxControlBar.setFlashEnabled(displayedFlashEnabled);
        videoComponent.setFlashEnabled(displayedFlashEnabled);
    }

    const auto cueEffectTrigger = audioProcessor.getLatestCueEffectTrigger();
    if (cueEffectTrigger != lastSeenCueEffectTrigger)
    {
        lastSeenCueEffectTrigger = cueEffectTrigger;
        const auto velocity = static_cast<int>(cueEffectTrigger & 0xffu);
        if (velocity > 0)
        {
            fxControlBar.setLastMidiVelocity(velocity);
            if (videoComponent.isRgbSplitAvailable() && displayedRgbSplitEnabled)
                videoComponent.triggerRgbSplitVelocity(velocity);
            if (videoComponent.isFlashAvailable() && displayedFlashEnabled)
                videoComponent.triggerFlashVelocity(velocity);
        }
    }

    timelineComponent.setSelectedCue(state.selectedCueIndex);
    assignedPadsComponent.setSelectedCue(state.selectedCueIndex);

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
            statusLabel.setText(state.playbackRequested ? "Ready · Playing" : "Ready · Paused",
                                juce::dontSendNotification);
        }

        const auto pendingCue = audioProcessor.consumePendingCue();
        const auto selectedCueForDisplay = pendingCue >= 0 ? pendingCue : state.selectedCueIndex;
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
            assignedPadsComponent.setSelectedCue(pendingCue);

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
        updateMediaPresentation(loadedVideoPath, duration);
        playButton.setIcon(isPlaying ? SamplerIconButton::Icon::pause : SamplerIconButton::Icon::play);
        playButton.setEnabled(true);
        timelineComponent.setTimelineData(duration, position, state.cuePositionsSeconds);
        assignedPadsComponent.setCueData(state.cuePositionsSeconds, selectedCueForDisplay);
    }
    else
    {
        previousVideoWasPlaying = false;
        loopRestartPending = false;
        loopRestartWaitTicks = 0;
        updateMediaPresentation(loadedVideoPath, 0.0);
        playButton.setIcon(SamplerIconButton::Icon::play);
        playButton.setEnabled(false);
        if (loadedVideoPath.isEmpty())
            statusLabel.setText("No video loaded · Cue points are kept", juce::dontSendNotification);
        timelineComponent.setTimelineData(0.0, 0.0, state.cuePositionsSeconds);
        assignedPadsComponent.setCueData(state.cuePositionsSeconds, state.selectedCueIndex);
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

void AVSamplerAudioProcessorEditor::ejectVideo()
{
    if (loadedVideoPath.isEmpty() && ! videoComponent.isVideoOpen())
        return;

    videoComponent.closeVideo();
    loadedVideoPath.clear();
    audioProcessor.setVideoPath({});
    previousVideoPosition = 0.0;
    previousVideoWasPlaying = false;
    loopRestartPending = false;
    loopRestartWaitTicks = 0;
    updateMediaPresentation({}, 0.0);
    statusLabel.setText("Media ejected · Cue points and session settings kept",
                        juce::dontSendNotification);
    playButton.setIcon(SamplerIconButton::Icon::play);
    playButton.setEnabled(false);
    timelineComponent.setTimelineData(0.0, 0.0, audioProcessor.getProjectState().cuePositionsSeconds);
    assignedPadsComponent.setCueData(audioProcessor.getProjectState().cuePositionsSeconds,
                                     audioProcessor.getProjectState().selectedCueIndex);
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
    playButton.setEnabled(false);

    if (path.isEmpty())
    {
        audioProcessor.setVideoPath({});
        statusLabel.setText("No video loaded · Cue points are kept", juce::dontSendNotification);
        playButton.setEnabled(false);
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
            safeThis->playButton.setEnabled(false);
            return;
        }

        safeThis->videoComponent.setAudioVolume(0.0f);
        safeThis->videoComponent.setPlaySpeed(avsampler::videoSpeedMultiplierForControllerValue(
            safeThis->audioProcessor.getProjectState().speedMidiCcValue));
        if (resumePlayback)
            safeThis->videoComponent.play();
        safeThis->previousVideoPosition = 0.0;
        safeThis->previousVideoWasPlaying = false;
        safeThis->manualPauseRequested = false;
        safeThis->displayedPlaybackRequested = resumePlayback;
        safeThis->playButton.setIcon(resumePlayback ? SamplerIconButton::Icon::pause
                                                    : SamplerIconButton::Icon::play);
        safeThis->playButton.setEnabled(true);
        safeThis->updateMediaPresentation(safeThis->loadedVideoPath,
                                          safeThis->videoComponent.getVideoDuration());
        safeThis->statusLabel.setText(resumePlayback ? "Ready · Playing" : "Ready · Paused",
                                      juce::dontSendNotification);
    });
}

void AVSamplerAudioProcessorEditor::updateMediaPresentation(const juce::String& path,
                                                             double durationSeconds)
{
    ejectButton.setEnabled(path.isNotEmpty());
    const auto durationText = durationSeconds > 0.0
        ? juce::String(durationSeconds, 2) + " s" : juce::String("-- s");
    if (positionLabel.getText() != durationText)
        positionLabel.setText(durationText, juce::dontSendNotification);

    if (path.isEmpty())
    {
        if (loadButton.getButtonText() != "Load")
            loadButton.setButtonText("Load");
        mediaInfoLabel.setText("No media loaded", juce::dontSendNotification);
        mediaInfoLabel.setColour(juce::Label::textColourId, juce::Colour(0xff8793a6));
        mediaInfoLabel.setTooltip("Choose a local video file for this host session.");
        return;
    }

    const juce::File mediaFile(path);
    mediaInfoLabel.setTooltip(path);
    if (! mediaFile.existsAsFile())
    {
        if (loadButton.getButtonText() != "Relink")
            loadButton.setButtonText("Relink");
        const auto text = fitMediaName("Missing · " + mediaFile.getFileName(),
                                       static_cast<float>(mediaInfoLabel.getWidth() - 8));
        if (mediaInfoLabel.getText() != text)
            mediaInfoLabel.setText(text, juce::dontSendNotification);
        mediaInfoLabel.setColour(juce::Label::textColourId, juce::Colour(0xffff8f79));
        mediaInfoLabel.setTooltip(path);
        return;
    }

    if (loadButton.getButtonText() != "Replace")
        loadButton.setButtonText("Replace");
    auto mediaDescription = mediaFile.getFileName();
    const auto nativeSize = videoComponent.getVideoNativeSize();
    if (! nativeSize.isEmpty())
        mediaDescription += " · " + juce::String(nativeSize.getWidth())
            + " × " + juce::String(nativeSize.getHeight());
    const auto text = fitMediaName(mediaDescription,
                                   static_cast<float>(mediaInfoLabel.getWidth() - 8));
    if (mediaInfoLabel.getText() != text)
        mediaInfoLabel.setText(text, juce::dontSendNotification);
    mediaInfoLabel.setColour(juce::Label::textColourId, juce::Colour(0xffd4dbe7));
    mediaInfoLabel.setTooltip(path);
}

void AVSamplerAudioProcessorEditor::updateLoopButtonAppearance(bool enabled)
{
    loopButton.setToggleState(enabled, juce::dontSendNotification);
    loopButton.setTooltip(enabled ? "Loop on · click to turn off" : "Loop off · click to turn on");
}
