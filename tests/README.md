# Visual Audio DSP Tests — VISUAL-004

These lightweight C++17 tests compile without JUCE and exercise the very same analysis helpers called by `AudioReactiveImagePOC::processBlock()`:

- Mono and stereo RMS/peak calculations; sample/channel averaging.
- Silence, empty buffers, six block sizes (32–1024 samples).
- Read-only audio analysis, including exact source-buffer comparisons.
- Envelope attack/release and sensitivity/smoothing extremes.

## Daily shortcut (macOS)

Run `make test` from the repository root for the tests alone, or just `make` to run tests and compile all plugin formats. See the main README for `make install` and other shortcuts.

## Run locally (from the repository root)

```bash
cmake -S tests -B build-tests -DCMAKE_BUILD_TYPE=Release
cmake --build build-tests --config Release
ctest --test-dir build-tests --output-on-failure
```

The standalone test CMake project does not fetch JUCE and does not change the plugin build. CI runs the same command on Linux.

On macOS, the repository's root CMake project also builds `AudioReactiveImageProcessorTests`. This JUCE-linked integration test passes real mono/stereo buffers through `processBlock()` and checks bit-identical samples, reported RMS/peak levels, and restoration of APVTS parameters plus `imagePath`.

## What this does NOT prove

- Host-specific mono/stereo negotiation and end-to-end DAW output are not proven by the processor test.
- Pluginval on the newly built AU/VST3 remains a separate check.
- Real-time attack/release invariance across different sample rates and buffer durations (VISUAL-005).
- Native MIDI input, currently out of scope.

Keep this gap visible in `VISUAL_QA_CHECKLIST.md`; do not call the PR fully verified before the macOS host checks.
