#include "CueRouting.h"

#include <iostream>

int main()
{
    for (std::size_t cue = 0; cue < avsampler::defaultCueMidiNotes.size(); ++cue)
    {
        if (avsampler::cueIndexForMidiNote(avsampler::defaultCueMidiNotes[cue]) != static_cast<int>(cue))
        {
            std::cerr << "MIDI note did not map to its expected Cue index\n";
            return 1;
        }
    }

    for (const auto note : { -1, 0, 37, 39, 127, 128 })
    {
        if (avsampler::cueIndexForMidiNote(note) != -1)
        {
            std::cerr << "Unassigned MIDI note unexpectedly mapped to a Cue\n";
            return 1;
        }
    }

    auto latestCue = avsampler::latestCueForMidiEvent(-1, 36, true);
    if (latestCue != 0)
        return 1;

    latestCue = avsampler::latestCueForMidiEvent(latestCue, 36, false);
    if (latestCue != 0)
    {
        std::cerr << "NoteOff changed the active Cue request\n";
        return 1;
    }

    latestCue = avsampler::latestCueForMidiEvent(latestCue, 37, true);
    if (latestCue != 0)
    {
        std::cerr << "Unassigned note changed the active Cue request\n";
        return 1;
    }

    latestCue = avsampler::latestCueForMidiEvent(latestCue, 38, true);
    latestCue = avsampler::latestCueForMidiEvent(latestCue, 41, true);
    if (latestCue != 3)
    {
        std::cerr << "The last valid Cue request did not take priority\n";
        return 1;
    }

    std::cout << "Cue note mapping and event priority passed\n";
    return 0;
}
