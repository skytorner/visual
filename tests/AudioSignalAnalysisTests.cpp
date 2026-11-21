#include "../Source/AudioSignalAnalysis.h"

#include <array>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>

namespace
{
int failures = 0;

void expectNear(const char* test, float value, float expected, float tolerance = 1.0e-6f)
{
    if (! std::isfinite(value) || std::abs(value - expected) > tolerance)
    {
        std::cerr << "FAIL: " << test << " (actual " << value << ", expected " << expected << ")\n";
        ++failures;
    }
}

void expectTrue(const char* test, bool condition)
{
    if (! condition)
    {
        std::cerr << "FAIL: " << test << "\n";
        ++failures;
    }
}

void testMono()
{
    const std::array<float, 4> samples { 1.0f, -1.0f, 0.0f, 0.0f };
    const float* channels[] = { samples.data() };
    const auto levels = visual::analyseAudio(channels, 1, static_cast<int>(samples.size()));
    expectNear("mono RMS", levels.rms, std::sqrt(0.5f));
    expectNear("mono peak", levels.peak, 1.0f);
}

void testStereo()
{
    const std::array<float, 2> left { 0.25f, -0.25f };
    const std::array<float, 2> right { -0.75f, 0.75f };
    const float* channels[] = { left.data(), right.data() };
    const auto levels = visual::analyseAudio(channels, 2, 2);
    expectNear("stereo RMS average across channels", levels.rms, std::sqrt(0.3125f));
    expectNear("stereo peak maximum across channels", levels.peak, 0.75f);
}

void testSilenceAndEmpty()
{
    const std::array<float, 4> samples {};
    const float* channels[] = { samples.data() };
    auto levels = visual::analyseAudio(channels, 1, 4);
    expectNear("silence RMS", levels.rms, 0.0f);
    expectNear("silence peak", levels.peak, 0.0f);

    levels = visual::analyseAudio(channels, 1, 0);
    expectNear("zero sample RMS", levels.rms, 0.0f);
    expectNear("zero sample peak", levels.peak, 0.0f);

    levels = visual::analyseAudio(nullptr, 0, 512);
    expectNear("zero channels RMS", levels.rms, 0.0f);
    expectNear("zero channels peak", levels.peak, 0.0f);
}

void testInputUnchangedAndBlockSizes()
{
    for (int size : { 32, 64, 128, 256, 512, 1024 })
    {
        std::vector<float> left(static_cast<std::size_t>(size));
        std::vector<float> right(static_cast<std::size_t>(size));

        for (int i = 0; i < size; ++i)
        {
            left[static_cast<std::size_t>(i)] = i % 2 == 0 ? -0.25f : 0.25f;
            right[static_cast<std::size_t>(i)] = i % 2 == 0 ? -0.5f : 0.5f;
        }

        const auto beforeLeft = left;
        const auto beforeRight = right;
        const float* channels[] = { left.data(), right.data() };
        const auto levels = visual::analyseAudio(channels, 2, size);
        expectNear("block-size-independent RMS", levels.rms, std::sqrt(0.15625f));
        expectNear("block-size-independent peak", levels.peak, 0.5f);
        expectTrue("left audio buffer is unchanged", left == beforeLeft);
        expectTrue("right audio buffer is unchanged", right == beforeRight);
    }
}

void testEnvelope()
{
    expectNear("attack from zero", visual::nextEnvelope(0.0f, 1.0f, 1.0f, 0.45f), 0.35f);
    expectNear("sensitivity clamps target to one", visual::nextEnvelope(0.0f, 1.0f, 5.0f, 0.45f), 0.35f);
    expectNear("low sensitivity", visual::nextEnvelope(0.0f, 1.0f, 0.1f, 0.45f), 0.035f);
    expectNear("fastest release", visual::nextEnvelope(1.0f, 0.0f, 1.0f, 0.0f), 0.78f);
    expectNear("slowest release", visual::nextEnvelope(1.0f, 0.0f, 1.0f, 1.0f), 0.975f);
    expectNear("stable zero", visual::nextEnvelope(0.0f, 0.0f, 1.0f, 1.0f), 0.0f);
    expectNear("negative RMS clamps target to zero", visual::nextEnvelope(0.0f, -1.0f, 1.0f, 0.5f), 0.0f);
    expectNear("oversized envelope clamps to one", visual::nextEnvelope(2.0f, 1.0f, 1.0f, 0.0f), 1.0f);
}
}

int main()
{
    testMono();
    testStereo();
    testSilenceAndEmpty();
    testInputUnchangedAndBlockSizes();
    testEnvelope();

    if (failures != 0)
    {
        std::cerr << failures << " audio-analysis test(s) failed\n";
        return EXIT_FAILURE;
    }

    std::cout << "All audio-analysis tests passed\n";
    return EXIT_SUCCESS;
}
