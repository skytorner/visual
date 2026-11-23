#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <string>

namespace avsampler::timeline
{
inline double clampCuePosition(double seconds, double duration) noexcept
{
    if (! std::isfinite(seconds) || ! std::isfinite(duration) || duration <= 0.0)
        return 0.0;

    return std::clamp(seconds, 0.0, duration);
}

inline double timeAtX(double x, double left, double width, double duration) noexcept
{
    if (! std::isfinite(x) || ! std::isfinite(left) || ! std::isfinite(width)
        || ! std::isfinite(duration) || width <= 0.0 || duration <= 0.0)
        return 0.0;

    return clampCuePosition((x - left) / width * duration, duration);
}

inline double xAtTime(double seconds, double left, double width, double duration) noexcept
{
    if (! std::isfinite(left) || ! std::isfinite(width) || ! std::isfinite(duration)
        || width <= 0.0 || duration <= 0.0)
        return left;

    return left + clampCuePosition(seconds, duration) / duration * width;
}

inline double nudgeByFrames(double seconds, double frameRate, int frameCount,
                            double duration) noexcept
{
    const auto safeFrameRate = std::isfinite(frameRate) && frameRate > 0.0 ? frameRate : 30.0;
    return clampCuePosition(seconds + static_cast<double>(frameCount) / safeFrameRate, duration);
}

inline std::string midiNoteName(int noteNumber)
{
    constexpr std::array<const char*, 12> pitchNames {
        "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
    };

    if (noteNumber < 0 || noteNumber > 127)
        return "MIDI " + std::to_string(noteNumber);

    const auto pitchClass = noteNumber % 12;
    const auto octave = noteNumber / 12 - 2; // Matches Logic's displayed octave (MIDI 36 = C1).
    return std::string(pitchNames[static_cast<std::size_t>(pitchClass)]) + std::to_string(octave);
}
}
