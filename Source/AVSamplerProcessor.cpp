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
    for (const auto metadata : midiMessages)
    {
        const auto& message = metadata.getMessage();
        latestCueIndex = avsampler::latestCueForMidiEvent(
            latestCueIndex, message.getNoteNumber(), message.isNoteOn());
    }

    if (latestCueIndex >= 0)
        pendingCueIndex.store(latestCueIndex, std::memory_order_release);

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

        for (int i = 0; i < cueCount; ++i)
        {
            const auto property = "cue" + juce::String(i + 1);
            const auto seconds = static_cast<double>(restored.getProperty(property, 0.0));
            newState.cuePositionsSeconds[static_cast<std::size_t>(i)] =
                std::isfinite(seconds) && seconds >= 0.0 ? seconds : 0.0;
        }

        const juce::ScopedLock lock(projectStateLock);
        projectState = std::move(newState);
    }
}

AVSamplerAudioProcessor::ProjectState AVSamplerAudioProcessor::getProjectState() const
{
    const juce::ScopedLock lock(projectStateLock);
    return projectState;
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

int AVSamplerAudioProcessor::consumePendingCue() noexcept
{
    return pendingCueIndex.exchange(-1, std::memory_order_acq_rel);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AVSamplerAudioProcessor();
}
