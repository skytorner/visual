#pragma once

#include <array>

namespace avsampler
{
constexpr std::array<int, 4> defaultCueMidiNotes { 36, 38, 40, 41 };

constexpr int cueIndexForMidiNote(int midiNote) noexcept
{
    for (int cueIndex = 0; cueIndex < static_cast<int>(defaultCueMidiNotes.size()); ++cueIndex)
        if (defaultCueMidiNotes[static_cast<std::size_t>(cueIndex)] == midiNote)
            return cueIndex;

    return -1;
}

constexpr int latestCueForMidiEvent(int previousCueIndex, int midiNote, bool isNoteOn) noexcept
{
    const auto cueIndex = isNoteOn ? cueIndexForMidiNote(midiNote) : -1;
    return cueIndex >= 0 ? cueIndex : previousCueIndex;
}
}
