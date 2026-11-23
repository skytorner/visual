#pragma once

#include <algorithm>
#include <array>

namespace avsampler
{
constexpr std::array<int, 4> defaultCueMidiNotes { 36, 38, 40, 41 };
constexpr int videoSpeedControllerNumber = 1;
constexpr int defaultVideoSpeedControllerValue = 64;
constexpr double minimumVideoSpeedMultiplier = 0.5;
constexpr double maximumVideoSpeedMultiplier = 1.5;

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

constexpr int videoSpeedValueForMidiEvent(int previousValue,
                                          int controllerNumber,
                                          int controllerValue) noexcept
{
    return controllerNumber == videoSpeedControllerNumber
        ? std::clamp(controllerValue, 0, 127)
        : std::clamp(previousValue, 0, 127);
}

constexpr double videoSpeedMultiplierForControllerValue(int controllerValue) noexcept
{
    const auto value = std::clamp(controllerValue, 0, 127);
    if (value <= defaultVideoSpeedControllerValue)
    {
        const auto proportion = static_cast<double>(value)
            / static_cast<double>(defaultVideoSpeedControllerValue);
        return minimumVideoSpeedMultiplier
            + proportion * (1.0 - minimumVideoSpeedMultiplier);
    }

    const auto proportion = static_cast<double>(value - defaultVideoSpeedControllerValue)
        / static_cast<double>(127 - defaultVideoSpeedControllerValue);
    return 1.0 + proportion * (maximumVideoSpeedMultiplier - 1.0);
}
}
