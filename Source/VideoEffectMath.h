#pragma once

#include <algorithm>

namespace avsampler::videoEffects
{
constexpr float rgbSplitIntensityForVelocity(int velocity) noexcept
{
    const auto clamped = std::clamp(velocity, 1, 127);
    return velocity <= 1 ? 0.0f : static_cast<float>(clamped - 1) / 126.0f;
}

constexpr double rgbSplitEnvelopeForElapsedSeconds(double elapsedSeconds) noexcept
{
    constexpr double attackSeconds = 0.005;
    constexpr double holdSeconds = 0.040;
    constexpr double releaseSeconds = 0.180;
    constexpr double releaseStart = attackSeconds + holdSeconds;
    constexpr double envelopeEnd = releaseStart + releaseSeconds;

    if (elapsedSeconds <= 0.0 || elapsedSeconds >= envelopeEnd)
        return 0.0;

    const auto smoothStep = [](double value)
    {
        const auto clamped = std::clamp(value, 0.0, 1.0);
        return clamped * clamped * (3.0 - 2.0 * clamped);
    };

    if (elapsedSeconds < attackSeconds)
        return smoothStep(elapsedSeconds / attackSeconds);

    if (elapsedSeconds < releaseStart)
        return 1.0;

    return 1.0 - smoothStep((elapsedSeconds - releaseStart) / releaseSeconds);
}

constexpr double rgbSplitMaximumOffsetUv(double intensity, double envelope) noexcept
{
    const auto boundedIntensity = std::clamp(intensity, 0.0, 1.0);
    const auto boundedEnvelope = std::clamp(envelope, 0.0, 1.0);
    return 0.015 * boundedIntensity * boundedEnvelope;
}
}
