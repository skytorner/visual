#include "AVSamplerProcessor.h"
#include "AVSamplerEditor.h"
#include "CueRouting.h"

#include <cmath>

AVSamplerAudioProcessor::AVSamplerAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
}

void AVSamplerAudioProcessor::prepareToPlay(double, int)
{
}

void AVSamplerAudioProcessor::releaseResources()
{
}

bool AVSamplerAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto input = layouts.getMainInputChannelSet();
    const auto output = layouts.getMainOutputChannelSet();

    return input.isDisabled()
        && (output == juce::AudioChannelSet::mono() || output == juce::AudioChannelSet::stereo());
}

void AVSamplerAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                           juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    juce::ignoreUnused(noDenormals);

    auto latestCueIndex = -1;
    auto latestSpeedCcValue = speedMidiCcValue.load(std::memory_order_relaxed);
    auto latestTriggerVelocity = 0;
    auto hasCueEffectTrigger = false;
    for (const auto metadata : midiMessages)
    {
        const auto& message = metadata.getMessage();
        latestCueIndex = avsampler::latestCueForMidiEvent(
            latestCueIndex, message.getNoteNumber(), message.isNoteOn());
        if (message.isNoteOn() && avsampler::cueIndexForMidiNote(message.getNoteNumber()) >= 0)
        {
            latestTriggerVelocity = message.getVelocity();
            hasCueEffectTrigger = true;
        }

        if (message.isController())
            latestSpeedCcValue = avsampler::videoSpeedValueForMidiEvent(
                latestSpeedCcValue, message.getControllerNumber(), message.getControllerValue());
    }

    if (latestCueIndex >= 0)
        pendingCueIndex.store(latestCueIndex, std::memory_order_release);
    if (hasCueEffectTrigger)
    {
        constexpr std::uint32_t sequenceMask = 0x00ffffffu;
        const auto previous = latestCueEffectTrigger.load(std::memory_order_relaxed);
        auto sequence = ((previous >> 8u) + 1u) & sequenceMask;
        if (sequence == 0u)
            sequence = 1u;
        const auto event = (sequence << 8u) | static_cast<std::uint32_t>(latestTriggerVelocity & 0xff);
        latestCueEffectTrigger.store(event, std::memory_order_release);
    }
    speedMidiCcValue.store(latestSpeedCcValue, std::memory_order_release);

    // This is a visual instrument; it never generates or passes audio.
    buffer.clear();
}

juce::AudioProcessorEditor* AVSamplerAudioProcessor::createEditor()
{
    return new AVSamplerAudioProcessorEditor(*this);
}

bool AVSamplerAudioProcessor::hasEditor() const
{
    return true;
}

const juce::String AVSamplerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AVSamplerAudioProcessor::acceptsMidi() const
{
    return true;
}

bool AVSamplerAudioProcessor::producesMidi() const
{
    return false;
}

bool AVSamplerAudioProcessor::isMidiEffect() const
{
    return false;
}

double AVSamplerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AVSamplerAudioProcessor::getNumPrograms()
{
    return 1;
}

int AVSamplerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AVSamplerAudioProcessor::setCurrentProgram(int)
{
}

const juce::String AVSamplerAudioProcessor::getProgramName(int)
{
    return {};
}

void AVSamplerAudioProcessor::changeProgramName(int, const juce::String&)
{
}

void AVSamplerAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    const auto snapshot = getProjectState();
    juce::ValueTree state("AVSamplerState");
    state.setProperty("videoPath", snapshot.videoPath, nullptr);
    state.setProperty("loopEnabled", snapshot.loopEnabled, nullptr);
    state.setProperty("playbackRequested", snapshot.playbackRequested, nullptr);
    state.setProperty("rgbSplitEnabled", snapshot.rgbSplitEnabled, nullptr);
    state.setProperty("flashEnabled", snapshot.flashEnabled, nullptr);
    state.setProperty("zoomPunchEnabled", snapshot.zoomPunchEnabled, nullptr);
    state.setProperty("colorBoostEnabled", snapshot.colorBoostEnabled, nullptr);
    state.setProperty("selectedCueIndex", snapshot.selectedCueIndex, nullptr);
    state.setProperty("speedMidiCcValue", snapshot.speedMidiCcValue, nullptr);

    for (int i = 0; i < cueCount; ++i)
        state.setProperty("cue" + juce::String(i + 1), snapshot.cuePositionsSeconds[static_cast<std::size_t>(i)], nullptr);

    if (auto xml = state.createXml())
        copyXmlToBinary(*xml, destData);
}

void AVSamplerAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
    {
        if (! xml->hasTagName("AVSamplerState"))
            return;

        const auto restored = juce::ValueTree::fromXml(*xml);
        ProjectState newState;
        newState.videoPath = restored.getProperty("videoPath").toString();
        newState.loopEnabled = static_cast<bool>(restored.getProperty("loopEnabled", false));
        newState.playbackRequested = static_cast<bool>(restored.getProperty("playbackRequested", true));
        newState.rgbSplitEnabled = static_cast<bool>(restored.getProperty("rgbSplitEnabled", true));
        newState.flashEnabled = static_cast<bool>(restored.getProperty("flashEnabled", false));
        newState.zoomPunchEnabled = static_cast<bool>(restored.getProperty("zoomPunchEnabled", false));
        newState.colorBoostEnabled = static_cast<bool>(restored.getProperty("colorBoostEnabled", false));
        newState.selectedCueIndex = juce::jlimit(0, cueCount - 1,
            static_cast<int>(restored.getProperty("selectedCueIndex", 0)));
        newState.speedMidiCcValue = juce::jlimit(0, 127,
            static_cast<int>(restored.getProperty("speedMidiCcValue",
                avsampler::defaultVideoSpeedControllerValue)));

        for (int i = 0; i < cueCount; ++i)
        {
            const auto property = "cue" + juce::String(i + 1);
            const auto seconds = static_cast<double>(restored.getProperty(property, 0.0));
            newState.cuePositionsSeconds[static_cast<std::size_t>(i)] =
                std::isfinite(seconds) && seconds >= 0.0 ? seconds : 0.0;
        }

        speedMidiCcValue.store(newState.speedMidiCcValue, std::memory_order_release);
        const juce::ScopedLock lock(projectStateLock);
        projectState = std::move(newState);
    }
}

AVSamplerAudioProcessor::ProjectState AVSamplerAudioProcessor::getProjectState() const
{
    const juce::ScopedLock lock(projectStateLock);
    auto snapshot = projectState;
    snapshot.speedMidiCcValue = speedMidiCcValue.load(std::memory_order_acquire);
    return snapshot;
}

void AVSamplerAudioProcessor::setVideoPath(const juce::String& path)
{
    const juce::ScopedLock lock(projectStateLock);
    projectState.videoPath = path;
}

void AVSamplerAudioProcessor::setCuePosition(int cueIndex, double positionSeconds)
{
    if (cueIndex < 0 || cueIndex >= cueCount || ! std::isfinite(positionSeconds))
        return;

    const juce::ScopedLock lock(projectStateLock);
    projectState.cuePositionsSeconds[static_cast<std::size_t>(cueIndex)] = juce::jmax(0.0, positionSeconds);
}

void AVSamplerAudioProcessor::setLoopEnabled(bool shouldLoop)
{
    const juce::ScopedLock lock(projectStateLock);
    projectState.loopEnabled = shouldLoop;
}

void AVSamplerAudioProcessor::setPlaybackRequested(bool shouldPlay)
{
    const juce::ScopedLock lock(projectStateLock);
    projectState.playbackRequested = shouldPlay;
}

void AVSamplerAudioProcessor::setRgbSplitEnabled(bool shouldEnable)
{
    const juce::ScopedLock lock(projectStateLock);
    projectState.rgbSplitEnabled = shouldEnable;
}

void AVSamplerAudioProcessor::setFlashEnabled(bool shouldEnable)
{
    const juce::ScopedLock lock(projectStateLock);
    projectState.flashEnabled = shouldEnable;
}

void AVSamplerAudioProcessor::setZoomPunchEnabled(bool shouldEnable)
{
    const juce::ScopedLock lock(projectStateLock);
    projectState.zoomPunchEnabled = shouldEnable;
}

void AVSamplerAudioProcessor::setColorBoostEnabled(bool shouldEnable)
{
    const juce::ScopedLock lock(projectStateLock);
    projectState.colorBoostEnabled = shouldEnable;
}

void AVSamplerAudioProcessor::setSelectedCueIndex(int cueIndex)
{
    if (cueIndex < 0 || cueIndex >= cueCount)
        return;

    const juce::ScopedLock lock(projectStateLock);
    projectState.selectedCueIndex = cueIndex;
}

int AVSamplerAudioProcessor::consumePendingCue() noexcept
{
    return pendingCueIndex.exchange(-1, std::memory_order_acq_rel);
}

std::uint32_t AVSamplerAudioProcessor::getLatestCueEffectTrigger() const noexcept
{
    return latestCueEffectTrigger.load(std::memory_order_acquire);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AVSamplerAudioProcessor();
}
