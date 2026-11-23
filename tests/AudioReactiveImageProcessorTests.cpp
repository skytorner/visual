#include "PluginProcessor.h"

#include <array>
#include <cmath>
#include <cstring>
#include <iostream>
#include <vector>

namespace
{
int failures = 0;

void expectTrue(const char* test, bool condition)
{
    if (! condition)
    {
        std::cerr << "FAIL: " << test << '\n';
        ++failures;
    }
}

void expectNear(const char* test, float actual, float expected, float tolerance = 1.0e-6f)
{
    expectTrue(test, std::isfinite(actual) && std::abs(actual - expected) <= tolerance);
}

void testTransparentProcessing(juce::AudioChannelSet channelSet)
{
    AudioReactiveImagePOCAudioProcessor processor;
    auto layout = processor.getBusesLayout();
    layout.inputBuses.set(0, channelSet);
    layout.outputBuses.set(0, channelSet);
    const auto acceptedLayout = processor.setBusesLayout(layout);
    expectTrue("processor accepts a matching mono or stereo layout", acceptedLayout);
    if (! acceptedLayout)
        return;

    constexpr int samples = 64;
    const auto channelCount = channelSet.size();
    processor.prepareToPlay(48000.0, samples);
    juce::AudioBuffer<float> buffer(channelCount, samples);
    std::vector<std::vector<float>> original(static_cast<std::size_t>(channelCount),
                                             std::vector<float>(static_cast<std::size_t>(samples)));

    for (int channel = 0; channel < channelCount; ++channel)
    {
        for (int sample = 0; sample < samples; ++sample)
        {
            const auto value = (channel == 0 ? 0.25f : -0.5f)
                * (sample % 2 == 0 ? 1.0f : -1.0f);
            buffer.setSample(channel, sample, value);
            original[static_cast<std::size_t>(channel)][static_cast<std::size_t>(sample)] = value;
        }
    }

    juce::MidiBuffer midi;
    processor.processBlock(buffer, midi);

    double sumSquares = 0.0;
    float expectedPeak = 0.0f;
    for (int channel = 0; channel < channelCount; ++channel)
    {
        for (int sample = 0; sample < samples; ++sample)
        {
            const auto expected = original[static_cast<std::size_t>(channel)][static_cast<std::size_t>(sample)];
            const auto actual = buffer.getSample(channel, sample);
            expectTrue("processBlock leaves every audio sample unchanged",
                       std::memcmp(&actual, &expected, sizeof(float)) == 0);
            sumSquares += static_cast<double>(expected) * expected;
            expectedPeak = std::max(expectedPeak, std::abs(expected));
        }
    }

    const auto expectedRms = static_cast<float>(std::sqrt(sumSquares / (channelCount * samples)));
    expectNear("processor reports expected RMS", processor.getRmsLevel(), expectedRms);
    expectNear("processor reports expected peak", processor.getPeakLevel(), expectedPeak);
}

void setParameter(AudioReactiveImagePOCAudioProcessor& processor, const char* parameterId, float value)
{
    auto* parameter = processor.getValueTreeState().getParameter(parameterId);
    if (parameter != nullptr)
        parameter->setValueNotifyingHost(parameter->convertTo0to1(value));
    else
        expectTrue("expected APVTS parameter exists", false);
}

void expectRestoredParameter(AudioReactiveImagePOCAudioProcessor& processor,
                             const char* parameterId,
                             float expected)
{
    const auto* value = processor.getValueTreeState().getRawParameterValue(parameterId);
    expectTrue("restored APVTS parameter exists", value != nullptr);
    if (value != nullptr)
        expectNear(parameterId, value->load(), expected);
}

void testStateRoundTrip()
{
    AudioReactiveImagePOCAudioProcessor source;
    setParameter(source, AudioReactiveImagePOCAudioProcessor::sensitivityParamId, 3.5f);
    setParameter(source, AudioReactiveImagePOCAudioProcessor::smoothingParamId, 0.8f);
    setParameter(source, AudioReactiveImagePOCAudioProcessor::minScaleParamId, 0.7f);
    setParameter(source, AudioReactiveImagePOCAudioProcessor::maxScaleParamId, 2.4f);
    setParameter(source, AudioReactiveImagePOCAudioProcessor::rotationAmountParamId, 17.0f);
    setParameter(source, AudioReactiveImagePOCAudioProcessor::opacityAmountParamId, 0.6f);
    source.setImagePath("/tmp/visual-004-image-state-test.png");

    juce::MemoryBlock state;
    source.getStateInformation(state);

    AudioReactiveImagePOCAudioProcessor restored;
    restored.setStateInformation(state.getData(), static_cast<int>(state.getSize()));

    expectTrue("image path is restored", restored.getImagePath() == "/tmp/visual-004-image-state-test.png");
    expectRestoredParameter(restored, AudioReactiveImagePOCAudioProcessor::sensitivityParamId, 3.5f);
    expectRestoredParameter(restored, AudioReactiveImagePOCAudioProcessor::smoothingParamId, 0.8f);
    expectRestoredParameter(restored, AudioReactiveImagePOCAudioProcessor::minScaleParamId, 0.7f);
    expectRestoredParameter(restored, AudioReactiveImagePOCAudioProcessor::maxScaleParamId, 2.4f);
    expectRestoredParameter(restored, AudioReactiveImagePOCAudioProcessor::rotationAmountParamId, 17.0f);
    expectRestoredParameter(restored, AudioReactiveImagePOCAudioProcessor::opacityAmountParamId, 0.6f);
}
}

int main()
{
    testTransparentProcessing(juce::AudioChannelSet::mono());
    testTransparentProcessing(juce::AudioChannelSet::stereo());
    testStateRoundTrip();

    if (failures != 0)
    {
        std::cerr << failures << " processor test(s) failed\n";
        return 1;
    }

    std::cout << "Audio Reactive processor transparency and state tests passed\n";
    return 0;
}
