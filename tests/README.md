# Visual Audio DSP Tests — VISUAL-004

These lightweight C++17 tests compile without JUCE and exercise the very same analysis helpers called by `AudioReactiveImagePOC::processBlock()`:

- Mono and stereo RMS/peak calculations; sample/channel averaging.
- Silence, empty buffers, six block sizes (32–1024 samples).
- Read-only audio analysis, including exact source-buffer comparisons.
- Envelope attack/release and sensitivity/smoothing extremes.

## Run locally (from the repository root)

```bash
cmake -S tests -B build-tests -DCMAKE_BUILD_TYPE=Release
cmake --build build-tests --config Release
ctest --test-dir build-tests --output-on-failure
```

The standalone test CMake project does not fetch JUCE and does not change the plugin build. CI runs the same command on Linux.

## What this does NOT prove

- The **full plugin** audio passthrough and bus layout, including host-specific mono/stereo negotiation.
- Saved-state / image-path restoration in JUCE; previous pluginval state checks apply only to the earlier build.
- Mac AU/VST3 compilation of this change; re-run the normal macOS build and pluginval before merging.
- Real-time attack/release invariance across different sample rates and buffer durations (VISUAL-005).
- Native MIDI input, currently out of scope.

Keep this gap visible in `VISUAL_QA_CHECKLIST.md`; do not call the PR fully verified before the macOS host checks.
