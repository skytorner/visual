#pragma once

#include <algorithm>
#include <cmath>

namespace visual
{
struct AudioLevels
{
    float rms = 0.0f;
    float peak = 0.0f;
};

// Pure, allocation-free analysis of input channels. Never writes to audio samples.
inline AudioLevels analyseAudio(const float* const* channels, int channelCount, int sampleCount) noexcept
{
    if (channels == nullptr || channelCount <= 0 || sampleCount <= 0)
        return {};

    double sumSquares = 0.0;
    float peak = 0.0f;

    for (int channel = 0; channel < channelCount; ++channel)
    {
        const auto* samples = channels[channel];

        for (int sample = 0; sample < sampleCount; ++sample)
        {
            const auto value = samples[sample];
            sumSquares += static_cast<double>(value) * static_cast<double>(value);
            peak = std::max(peak, std::abs(value));
        }
    }

    const auto totalSamples = static_cast<double>(channelCount) * sampleCount;
    return { static_cast<float>(std::sqrt(sumSquares / totalSamples)), peak };
}

// Retain the POC's per-block smoothing. Time-constant correction belongs to VISUAL-005.
inline float nextEnvelope(float current, float rms, float sensitivity, float smoothing) noexcept
{
    const auto target = std::clamp(rms * sensitivity, 0.0f, 1.0f);
    const auto attackAmount = 0.35f;
    const auto releaseAmount = 0.22f + smoothing * (0.025f - 0.22f);
    const auto amount = target > current ? attackAmount : releaseAmount;

    return std::clamp(current + (target - current) * amount, 0.0f, 1.0f);
}
}
