#include "PluginProcessor.h"
#include "PluginEditor.h"

AudioReactiveImagePOCAudioProcessor::AudioReactiveImagePOCAudioProcessor()
    : AudioProcessor(BusesProperties()
          .withInput("Input", juce::AudioChannelSet::stereo(), true)
          .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "Parameters", createParameterLayout())
{
}

void AudioReactiveImagePOCAudioProcessor::prepareToPlay(double, int)
{
    resetVisualState();
}

void AudioReactiveImagePOCAudioProcessor::releaseResources()
{
}

bool AudioReactiveImagePOCAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto& input = layouts.getMainInputChannelSet();
    const auto& output = layouts.getMainOutputChannelSet();

    return input == juce::AudioChannelSet::stereo()
        && output == juce::AudioChannelSet::stereo();
}

void AudioReactiveImagePOCAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                                       juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);
    juce::ScopedNoDenormals noDenormals;

    const auto numInputChannels = getTotalNumInputChannels();
    const auto numOutputChannels = getTotalNumOutputChannels();
    const auto numSamples = buffer.getNumSamples();
    const auto channelsToAnalyse = juce::jmin(numInputChannels, buffer.getNumChannels());

    if (resetRequested.exchange(false, std::memory_order_acq_rel))
        envelopeState = 0.0f;

    double sumSquares = 0.0;
    float peak = 0.0f;

    for (int channel = 0; channel < channelsToAnalyse; ++channel)
    {
        const auto* samples = buffer.getReadPointer(channel);

        for (int sample = 0; sample < numSamples; ++sample)
        {
            const auto value = samples[sample];
            sumSquares += static_cast<double>(value) * static_cast<double>(value);
            peak = juce::jmax(peak, std::abs(value));
        }
    }

    for (int channel = numInputChannels; channel < numOutputChannels; ++channel)
        buffer.clear(channel, 0, numSamples);

    const auto sampleCount = juce::jmax(1, channelsToAnalyse * numSamples);
    const auto rms = static_cast<float>(std::sqrt(sumSquares / static_cast<double>(sampleCount)));

    const auto sensitivity = parameters.getRawParameterValue(sensitivityParamId)->load();
    const auto smoothing = parameters.getRawParameterValue(smoothingParamId)->load();
    const auto targetEnvelope = juce::jlimit(0.0f, 1.0f, rms * sensitivity);

    const auto attackAmount = 0.35f;
    const auto releaseAmount = juce::jmap(smoothing, 0.0f, 1.0f, 0.22f, 0.025f);
    const auto envelopeAmount = targetEnvelope > envelopeState ? attackAmount : releaseAmount;

    envelopeState += (targetEnvelope - envelopeState) * envelopeAmount;
    envelopeState = juce::jlimit(0.0f, 1.0f, envelopeState);

    rmsLevel.store(rms, std::memory_order_relaxed);
    peakLevel.store(juce::jlimit(0.0f, 1.0f, peak), std::memory_order_relaxed);
    envelopeLevel.store(envelopeState, std::memory_order_relaxed);
}

juce::AudioProcessorEditor* AudioReactiveImagePOCAudioProcessor::createEditor()
{
    return new AudioReactiveImagePOCAudioProcessorEditor(*this);
}

bool AudioReactiveImagePOCAudioProcessor::hasEditor() const
{
    return true;
}

const juce::String AudioReactiveImagePOCAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AudioReactiveImagePOCAudioProcessor::acceptsMidi() const
{
    return false;
}

bool AudioReactiveImagePOCAudioProcessor::producesMidi() const
{
    return false;
}

bool AudioReactiveImagePOCAudioProcessor::isMidiEffect() const
{
    return false;
}

double AudioReactiveImagePOCAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AudioReactiveImagePOCAudioProcessor::getNumPrograms()
{
    return 1;
}

int AudioReactiveImagePOCAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AudioReactiveImagePOCAudioProcessor::setCurrentProgram(int)
{
}

const juce::String AudioReactiveImagePOCAudioProcessor::getProgramName(int)
{
    return {};
}

void AudioReactiveImagePOCAudioProcessor::changeProgramName(int, const juce::String&)
{
}

void AudioReactiveImagePOCAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();

    {
        const juce::ScopedLock lock(imagePathLock);
        state.setProperty("imagePath", imagePath, nullptr);
    }

    if (auto xml = state.createXml())
        copyXmlToBinary(*xml, destData);
}

void AudioReactiveImagePOCAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
    {
        if (xml->hasTagName(parameters.state.getType()))
        {
            const auto restoredState = juce::ValueTree::fromXml(*xml);
            parameters.replaceState(restoredState);
            setImagePath(restoredState.getProperty("imagePath").toString());
        }
    }
}

void AudioReactiveImagePOCAudioProcessor::setImagePath(const juce::String& path)
{
    const juce::ScopedLock lock(imagePathLock);
    imagePath = path;
}

juce::String AudioReactiveImagePOCAudioProcessor::getImagePath() const
{
    const juce::ScopedLock lock(imagePathLock);
    return imagePath;
}

void AudioReactiveImagePOCAudioProcessor::resetVisualState()
{
    resetRequested.store(true, std::memory_order_release);
    rmsLevel.store(0.0f, std::memory_order_relaxed);
    peakLevel.store(0.0f, std::memory_order_relaxed);
    envelopeLevel.store(0.0f, std::memory_order_relaxed);
}

AudioReactiveImagePOCAudioProcessor::APVTS::ParameterLayout
AudioReactiveImagePOCAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { sensitivityParamId, 1 },
        "Sensitivity",
        juce::NormalisableRange<float>(0.1f, 5.0f, 0.01f, 0.5f),
        1.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { smoothingParamId, 1 },
        "Smoothing",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.45f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { minScaleParamId, 1 },
        "Min Scale",
        juce::NormalisableRange<float>(0.5f, 1.5f, 0.01f),
        0.92f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { maxScaleParamId, 1 },
        "Max Scale",
        juce::NormalisableRange<float>(1.0f, 3.0f, 0.01f),
        1.35f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { rotationAmountParamId, 1 },
        "Rotation Amount",
        juce::NormalisableRange<float>(0.0f, 30.0f, 0.1f),
        6.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { opacityAmountParamId, 1 },
        "Opacity Amount",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.25f));

    return { params.begin(), params.end() };
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioReactiveImagePOCAudioProcessor();
}
