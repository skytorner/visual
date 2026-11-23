#include "AVSamplerProcessor.h"

#include <cmath>
#include <iostream>

namespace
{
bool check(bool condition, const char* message)
{
    if (! condition)
        std::cerr << message << '\n';
    return condition;
}
}

int main()
{
    AVSamplerAudioProcessor processor;
    juce::AudioBuffer<float> audio(2, 16);
    for (int channel = 0; channel < audio.getNumChannels(); ++channel)
        for (int sample = 0; sample < audio.getNumSamples(); ++sample)
            audio.setSample(channel, sample, 0.75f);
    juce::MidiBuffer midi;

    midi.addEvent(juce::MidiMessage::noteOn(1, 36, static_cast<juce::uint8>(100)), 0);
    midi.addEvent(juce::MidiMessage::noteOn(1, 38, static_cast<juce::uint8>(100)), 4);
    midi.addEvent(juce::MidiMessage::noteOff(1, 38), 8);
    processor.processBlock(audio, midi);
    if (! check(processor.consumePendingCue() == 1, "NoteOn/last Cue priority failed"))
        return 1;

    for (int channel = 0; channel < audio.getNumChannels(); ++channel)
        for (int sample = 0; sample < audio.getNumSamples(); ++sample)
            if (! check(audio.getSample(channel, sample) == 0.0f, "Instrument output was not silent"))
                return 1;

    midi.clear();
    midi.addEvent(juce::MidiMessage::noteOff(1, 36), 0);
    midi.addEvent(juce::MidiMessage::noteOn(1, 37, static_cast<juce::uint8>(100)), 4);
    processor.processBlock(audio, midi);
    if (! check(processor.consumePendingCue() == -1, "NoteOff/unassigned note triggered a Cue"))
        return 1;

    processor.setVideoPath("/tmp/av-sampler-reference.mov");
    processor.setCuePosition(0, 0.25);
    processor.setCuePosition(1, 1.5);
    processor.setCuePosition(2, 3.75);
    processor.setCuePosition(3, 8.0);
    processor.setLoopEnabled(true);
    processor.setPlaybackRequested(false);
    processor.setSelectedCueIndex(2);

    juce::MemoryBlock savedState;
    processor.getStateInformation(savedState);
    AVSamplerAudioProcessor restored;
    restored.setStateInformation(savedState.getData(), static_cast<int>(savedState.getSize()));
    const auto state = restored.getProjectState();

    if (! check(state.videoPath == "/tmp/av-sampler-reference.mov", "Video path did not restore"))
        return 1;
    if (! check(state.loopEnabled, "Loop setting did not restore"))
        return 1;
    if (! check(! state.playbackRequested, "Paused playback state did not restore"))
        return 1;
    if (! check(state.selectedCueIndex == 2, "Selected Cue did not restore"))
        return 1;

    constexpr double expectedPositions[] { 0.25, 1.5, 3.75, 8.0 };
    for (int i = 0; i < AVSamplerAudioProcessor::cueCount; ++i)
        if (! check(std::abs(state.cuePositionsSeconds[static_cast<std::size_t>(i)] - expectedPositions[i]) < 1.0e-9,
                    "Cue position did not restore"))
            return 1;

    restored.setVideoPath("/tmp/av-sampler-relocated.mov");
    const auto relinkedState = restored.getProjectState();
    if (! check(relinkedState.videoPath == "/tmp/av-sampler-relocated.mov",
                "Relinked media path did not update"))
        return 1;
    for (int i = 0; i < AVSamplerAudioProcessor::cueCount; ++i)
        if (! check(std::abs(relinkedState.cuePositionsSeconds[static_cast<std::size_t>(i)] - expectedPositions[i]) < 1.0e-9,
                    "Relinking media changed the saved Cue timing"))
            return 1;

    juce::ValueTree legacyState("AVSamplerState");
    legacyState.setProperty("videoPath", "/tmp/legacy-session.mov", nullptr);
    legacyState.setProperty("loopEnabled", true, nullptr);
    auto legacyXml = legacyState.createXml();
    juce::MemoryBlock legacyData;
    juce::AudioProcessor::copyXmlToBinary(*legacyXml, legacyData);
    AVSamplerAudioProcessor legacyRestored;
    legacyRestored.setStateInformation(legacyData.getData(), static_cast<int>(legacyData.getSize()));
    const auto legacyProjectState = legacyRestored.getProjectState();
    if (! check(legacyProjectState.playbackRequested && legacyProjectState.selectedCueIndex == 0,
                "Legacy session did not default to autoplay with the first Cue selected"))
        return 1;

    std::cout << "AV Sampler MIDI processing and state tests passed\n";
    return 0;
}
