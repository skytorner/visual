#include "VideoEffectMath.h"

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

bool near(double actual, double expected)
{
    return std::abs(actual - expected) < 1.0e-6;
}
}

int main()
{
    using avsampler::videoEffects::rgbSplitEnvelopeForElapsedSeconds;
    using avsampler::videoEffects::rgbSplitIntensityForVelocity;
    using avsampler::videoEffects::rgbSplitMaximumOffsetUv;
    using avsampler::videoEffects::flashWhiteMixForVelocityAndElapsedSeconds;
    using avsampler::videoEffects::zoomPunchScaleForVelocityAndElapsedSeconds;

    if (! check(rgbSplitIntensityForVelocity(0) == 0.0f, "Velocity 0 must not trigger RGB Split"))
        return 1;
    if (! check(rgbSplitIntensityForVelocity(1) == 0.0f, "Velocity 1 must map to zero intensity"))
        return 1;
    if (! check(near(rgbSplitIntensityForVelocity(20), 19.0 / 126.0), "Velocity 20 mapping failed"))
        return 1;
    if (! check(near(rgbSplitIntensityForVelocity(64), 0.5), "Velocity 64 mapping failed"))
        return 1;
    if (! check(near(rgbSplitIntensityForVelocity(100), 99.0 / 126.0), "Velocity 100 mapping failed"))
        return 1;
    if (! check(rgbSplitIntensityForVelocity(127) == 1.0f, "Velocity 127 must map to maximum intensity"))
        return 1;
    if (! check(rgbSplitIntensityForVelocity(128) == 1.0f, "Velocity mapping upper clamp failed"))
        return 1;

    if (! check(rgbSplitEnvelopeForElapsedSeconds(0.0) == 0.0, "Envelope must begin at zero"))
        return 1;
    if (! check(near(rgbSplitEnvelopeForElapsedSeconds(0.0025), 0.5), "Envelope attack curve failed"))
        return 1;
    if (! check(rgbSplitEnvelopeForElapsedSeconds(0.005) == 1.0, "Envelope attack end failed"))
        return 1;
    if (! check(rgbSplitEnvelopeForElapsedSeconds(0.045) == 1.0, "Envelope hold end failed"))
        return 1;
    if (! check(near(rgbSplitEnvelopeForElapsedSeconds(0.135), 0.5), "Envelope release curve failed"))
        return 1;
    if (! check(rgbSplitEnvelopeForElapsedSeconds(0.225) == 0.0, "Envelope release end failed"))
        return 1;

    if (! check(near(rgbSplitMaximumOffsetUv(1.0f, 1.0), 0.015), "Maximum RGB offset must be 1.5 percent"))
        return 1;
    if (! check(near(rgbSplitMaximumOffsetUv(0.5f, 1.0), 0.0075), "Offset must follow velocity intensity"))
        return 1;
    if (! check(rgbSplitMaximumOffsetUv(1.0f, 0.0) == 0.0, "Envelope end must restore the original image"))
        return 1;

    if (! check(flashWhiteMixForVelocityAndElapsedSeconds(0, 0.005) == 0.0,
                "Velocity 0 must not trigger Flash"))
        return 1;
    if (! check(flashWhiteMixForVelocityAndElapsedSeconds(1, 0.005) == 0.0,
                "Velocity 1 must map to zero Flash intensity"))
        return 1;
    if (! check(near(flashWhiteMixForVelocityAndElapsedSeconds(64, 0.005), 0.325),
                "Flash intensity must scale with velocity"))
        return 1;
    if (! check(near(flashWhiteMixForVelocityAndElapsedSeconds(127, 0.005), 0.65),
                "Maximum velocity must reach the bounded white mix"))
        return 1;
    if (! check(flashWhiteMixForVelocityAndElapsedSeconds(127, 0.225) == 0.0,
                "Flash envelope must return to the original frame"))
        return 1;

    if (! check(zoomPunchScaleForVelocityAndElapsedSeconds(0, 0.005) == 1.0,
                "Velocity 0 must not trigger Zoom Punch"))
        return 1;
    if (! check(near(zoomPunchScaleForVelocityAndElapsedSeconds(64, 0.005), 1.06),
                "Zoom Punch scale must follow MIDI velocity"))
        return 1;
    if (! check(near(zoomPunchScaleForVelocityAndElapsedSeconds(127, 0.005), 1.12),
                "Maximum velocity must reach the bounded Zoom Punch scale"))
        return 1;
    if (! check(zoomPunchScaleForVelocityAndElapsedSeconds(127, 0.225) == 1.0,
                "Zoom Punch envelope must return to the original framing"))
        return 1;

    std::cout << "Video effect velocity mapping and envelope tests passed\n";
    return 0;
}
