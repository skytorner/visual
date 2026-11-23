#include "TimelineMath.h"

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
    using namespace avsampler::timeline;

    if (! check(midiNoteName(36) == "C1" && midiNoteName(38) == "D1"
                && midiNoteName(40) == "E1" && midiNoteName(41) == "F1",
                "Logic MIDI note labels did not match the fixed cue mapping"))
        return 1;

    constexpr double duration = 16.24;
    constexpr double left = 55.0;
    constexpr double width = 1110.0;
    constexpr double cueTime = 4.5;
    const auto x = xAtTime(cueTime, left, width, duration);
    if (! check(std::abs(timeAtX(x, left, width, duration) - cueTime) < 1.0e-9,
                "Timeline position/time conversion did not round-trip"))
        return 1;
    if (! check(timeAtX(left - 10.0, left, width, duration) == 0.0
                && timeAtX(left + width + 10.0, left, width, duration) == duration,
                "Timeline conversion did not clamp to video bounds"))
        return 1;
    if (! check(nudgeByFrames(1.0, 25.0, 1, duration) == 1.04
                && nudgeByFrames(1.0, 25.0, -1, duration) == 0.96,
                "Keyboard frame nudge did not honor the requested frame rate"))
        return 1;
    if (! check(clampCuePosition(20.0, duration) == duration
                && clampCuePosition(-1.0, duration) == 0.0,
                "Cue position did not clamp at media boundaries"))
        return 1;
    std::cout << "Timeline time/position and note-name tests passed\n";
    return 0;
}
