# AV Sampler — Play video like an instrument

AV Sampler is a macOS-first MIDI video instrument for music production. It plays one local video, lets four MIDI notes trigger editable Cue Points, and offers video speed via MIDI CC1 plus a velocity-triggered RGB Split effect. It does not generate audio.

## Current maturity

AV Sampler is an internally tested prototype, **not a released or distributable alpha package**. The AU has been manually exercised in Logic Pro, including Cue playback, project save/reopen, video looping, CC1 control, RGB Split and fluid playback in a loop. These are user-reported host checks; no numeric latency, frame-rate or CPU/GPU benchmark is claimed.

The currently qualified target is **macOS Audio Unit in Logic Pro**. AV Sampler VST3, Windows and other DAWs are not qualified. The AV Sampler target is only added by CMake on Apple platforms.

## Features present on `main`

- One local video with Play/Pause, Loop, duration and media replacement/relink controls.
- Four draggable Cue Points, mapped to MIDI notes 36, 38, 40 and 41 (shown as C1, D1, E1 and F1 in the tested Logic setup; octave labels vary between hosts).
- MIDI CC1 controls video speed from 0.5× to 1.5×. The endpoints have been manually confirmed; CC1=64 neutral behavior and Logic save/reopen are still listed for verification.
- RGB Split triggered by Cue NoteOn velocity. Its enabled/disabled state is saved with the host project.
- Metal frame presentation on macOS using AVFoundation and MetalKit.
- Host-project state includes media path, Cue positions, Loop, requested Play/Pause, selected Cue, speed controller value and RGB Split enablement. Video files remain external to the project and must be relinked if moved.

See [product scope](docs/project/VISUAL_PRODUCT.md), [architecture](docs/project/VISUAL_ARCHITECTURE.md), [backlog](docs/project/VISUAL_BACKLOG.md) and [QA evidence](docs/project/VISUAL_QA_CHECKLIST.md).

## Build and install for local Logic QA (macOS)

Requirements: macOS, Xcode Command Line Tools, CMake 3.22+, Git and network access for the first JUCE FetchContent setup. From the repository root:

```bash
make status
make install-avsampler-au BUILD_DIR=/private/tmp/visual-build BUILD_TYPE=Release
```

Close Logic Pro before installing. The command builds the AU and copies it to `~/Library/Audio/Plug-Ins/Components/AVSampler.component`, then applies an ad hoc local signature and verifies it. Reopen Logic, rescan Audio Units if needed, and insert AVSampler as a software instrument. This is a **developer QA install**, not a ready-to-share alpha installer. It requires a local build environment and does not notarize the plugin.

In Logic, load a local video, place MIDI notes 36/38/40/41 in a region, and play it. Drag Cue markers to change their trigger times. Send MIDI CC1 to change playback speed; Cue velocity controls RGB Split intensity. The plugin state is saved by the host, while the video file itself remains external.

## Limitations and open qualification

- No Closed Alpha package or automated AV Sampler release workflow exists yet.
- Supported macOS versions, codec/container matrix and minimum hardware are not formally declared.
- Project state stores a media path, not video bytes; cross-machine project portability is not guaranteed.
- Media relink/replacement preservation, Logic CC1 neutral/save-reopen, playback with the editor closed, and numeric responsiveness/performance measurements need explicit release evidence.
- `pluginval` has previously exited before testing this AU in the local environment. A later manual app run is awaiting its complete logs; no new PASS is inferred from a restart or partial run.
- Frame-accurate or sample-accurate video triggering and a guaranteed latency are not claimed.

## Repository map

- `Source/AVSampler*` and `Source/VideoFramePlayerComponent.*`: AV Sampler instrument and macOS video renderer.
- `Source/Plugin*`: separate legacy audio-reactive image effect; preserve its plugin identity and project state.
- `tests/`: C++ unit and processor tests.
- `scripts/` and `.github/workflows/`: existing release tooling, currently for `AudioReactiveImagePOC`, not an AV Sampler alpha package.
- `docs/project/`: versioned product, architecture, decisions, backlog, QA and release records.

## Development commands

```bash
make help
make test
make plugin
make install-avsampler-au BUILD_DIR=/private/tmp/visual-build BUILD_TYPE=Release
```

`make test` runs the fast C++ tests; `make plugin` builds the JUCE targets. `make install-avsampler-au` is explicit and must be run for each Logic QA build. `make package` still packages the legacy AudioReactiveImagePOC only; it is not an AV Sampler distribution command.

---

## AudioReactiveImagePOC — legacy audio effect

`AudioReactiveImagePOC` is the separate legacy JUCE/C++ audio effect, with its existing plugin identity and saved projects. Insert it on an audio track: it passes the incoming audio through unchanged, analyzes global RMS/peak/envelope, and animates a user-loaded PNG/JPG image from the detected audio energy. Current release version: `1.0.0`. It is not the AV Sampler MIDI instrument.

## Features

- Stereo audio effect/analyzer.
- Stereo passthrough: no destructive processing and no added latency.
- Global RMS, peak, and attack/release-smoothed envelope.
- UI thread animation driven from lock-free `std::atomic<float>` audio metrics.
- Upload Image button for PNG/JPG/JPEG files.
- Placeholder when no image is loaded.
- Sliders:
  - Sensitivity
  - Smoothing
  - Min Scale
  - Max Scale
  - Rotation Amount
  - Opacity Amount
- Reset button.
- State persistence for parameters and local image path.
- VST3 build by default.
- AU on macOS and Standalone target are enabled when supported by JUCE/CMake.

## Project Structure

```txt
CMakeLists.txt
Source/
  AVSamplerProcessor.h/.cpp
  AVSamplerEditor.h/.cpp
  VideoFramePlayerComponent.h/.mm
  CueTimelineComponent.h/.cpp
  SamplerUiComponents.h/.cpp
  PluginProcessor.h/.cpp       # legacy AudioReactiveImagePOC
  PluginEditor.h/.cpp          # legacy AudioReactiveImagePOC
tests/
  CueRoutingTests.cpp
  TimelineMathTests.cpp
  VideoEffectMathTests.cpp
  AVSamplerCoreTests.cpp
  AudioReactiveImageProcessorTests.cpp
README.md
```

## Prerequisites macOS

- macOS with Xcode Command Line Tools:

```bash
xcode-select --install
```

- CMake 3.22 or newer:

```bash
brew install cmake
```

- A DAW for testing, for example REAPER, Ableton Live, or Logic Pro.

Notes:

- The CMake project fetches JUCE automatically from GitHub with `FetchContent`.
- VST3 and Standalone are enabled for all platforms supported by JUCE.
- AU is enabled only on Apple platforms.

## Prerequisites Windows

- Visual Studio 2022 with the "Desktop development with C++" workload.
- CMake 3.22 or newer.
- Git.
- A DAW for testing, for example REAPER or Ableton Live.

Optional but useful:

- Ninja build system.

## Clone And Build

From this repository root:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

## Fast development commands (macOS)

Use the repository's tracked `Makefile` instead of retyping CMake commands every time. Run these from the repository root:

```bash
make               # C++ DSP tests + JUCE plugin build (AU, VST3, Standalone)
make test          # Only fast C++ DSP tests
make plugin        # Only JUCE plugin build
make status        # Print branch and commit SHA used for QA
make help          # Show all shortcuts
```

The first build may fetch JUCE and take longer. Later invocations reuse `build/` and `build-tests/` incrementally. No Makefile change is needed when switching branches; run `git fetch` / `git switch` as usual.

To test in a DAW, **close Logic Pro / REAPER first**, then deliberately run:

```bash
make install       # Build and install legacy AU/VST3 plus AV Sampler AU (macOS)
make install-au    # AU only, for Logic Pro
make install-vst3  # VST3 only, for REAPER
```

Installation never runs automatically with `make` and copies only to the current user's plugin folders. Restart the DAW after installation. These AU/VST3 bundle paths below refer to the legacy AudioReactiveImagePOC; AV Sampler is a separate AU installed by `make install-avsampler-au`. The shortcuts do **not** run pluginval. For a macOS ZIP, `make package` builds and invokes the existing AudioReactiveImagePOC-only packaging script, writing into `dist/`. On Windows continue using the CMake/PowerShell commands below.

## Package A Shareable macOS Release ZIP

After building, create a local ZIP that can be shared by download:

```bash
bash scripts/package-macos-release.sh
```

The package is generated at:

```txt
dist/AudioReactiveImagePOC-v1.0.0-macos.zip
```

`dist/` is intentionally ignored by Git. Commit source code, scripts, and release notes; upload the ZIP separately as the downloadable release asset.

## Package A Shareable Windows Release ZIP

Windows binaries must be built on Windows with Visual Studio. From a Windows PowerShell terminal:

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release
.\scripts\package-windows-release.ps1
```

The package is generated at:

```txt
dist\AudioReactiveImagePOC-v1.0.0-windows.zip
```

The ZIP contains the Windows VST3 and, when generated by JUCE/CMake, the Standalone `.exe`.

You can also generate both macOS and Windows release ZIPs through GitHub Actions:

```txt
Actions > Build release packages > Run workflow
```

The Windows artifact uploaded by the workflow is:

```txt
AudioReactiveImagePOC-v1.0.0-windows.zip
```

The default JUCE tag is configured in `CMakeLists.txt`:

```cmake
set(JUCE_GIT_TAG "8.0.8" CACHE STRING "JUCE git tag or branch to fetch")
```

You can override it:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DJUCE_GIT_TAG=master
cmake --build build --config Release
```

## Generated Plugin Locations

JUCE/CMake places plugin artifacts under the build directory. Typical paths:

### macOS

```txt
build/AudioReactiveImagePOC_artefacts/Release/VST3/AudioReactiveImagePOC.vst3
build/AudioReactiveImagePOC_artefacts/Release/AU/AudioReactiveImagePOC.component
build/AudioReactiveImagePOC_artefacts/Release/Standalone/AudioReactiveImagePOC.app
```

### Windows

```txt
build/AudioReactiveImagePOC_artefacts/Release/VST3/AudioReactiveImagePOC.vst3
build/AudioReactiveImagePOC_artefacts/Release/Standalone/AudioReactiveImagePOC.exe
```

## Install Locally In A DAW

### macOS VST3

Copy:

```txt
build/AudioReactiveImagePOC_artefacts/Release/VST3/AudioReactiveImagePOC.vst3
```

To:

```txt
~/Library/Audio/Plug-Ins/VST3/
```

Or system-wide:

```txt
/Library/Audio/Plug-Ins/VST3/
```

### macOS AU

Copy:

```txt
build/AudioReactiveImagePOC_artefacts/Release/AU/AudioReactiveImagePOC.component
```

To:

```txt
~/Library/Audio/Plug-Ins/Components/
```

Or system-wide:

```txt
/Library/Audio/Plug-Ins/Components/
```

If Logic Pro does not detect the AU immediately:

```bash
killall -9 AudioComponentRegistrar
```

Then restart Logic Pro.

### Windows VST3

Copy:

```txt
build/AudioReactiveImagePOC_artefacts/Release/VST3/AudioReactiveImagePOC.vst3
```

To:

```txt
C:\Program Files\Common Files\VST3\
```

Then rescan plugins in your DAW.

## Test In REAPER

1. Copy the VST3 into the local VST3 folder.
2. Open REAPER.
3. Go to `Options > Preferences > Plug-ins > VST`.
4. Click `Re-scan`.
5. Insert `AudioReactiveImagePOC` on an audio track.
6. Play audio through the track.
7. Click `Upload Image` and choose a PNG/JPG.
8. Confirm the image pulses with the audio level.
9. Move the sliders and confirm the animation intensity changes.

## Test In Ableton Live

1. Copy the VST3 into the platform VST3 folder.
2. Open Ableton Live.
3. Go to `Settings/Preferences > Plug-Ins`.
4. Enable VST3 plug-ins and rescan.
5. Drop `AudioReactiveImagePOC` on an audio track.
6. Load an image and play audio.
7. Confirm audio remains audible and the image reacts to level changes.

## Test In Logic Pro

Logic Pro uses AU, not VST3.

1. Build on macOS.
2. Copy `AudioReactiveImagePOC.component` into `~/Library/Audio/Plug-Ins/Components/`.
3. Restart Logic Pro or reset the AudioComponent registrar.
4. Open `Logic Pro > Settings > Plug-In Manager`.
5. Confirm `AudioReactiveImagePOC` is validated.
6. Insert it as an Audio FX on a track.
7. Load an image and play audio.

## Design Notes

- The audio thread only reads samples, calculates RMS/peak/envelope, and stores results in atomics.
- The plugin does not allocate, load images, touch UI objects, or lock in `processBlock`.
- Image loading happens on the message/UI thread via JUCE `FileChooser`.
- The audio buffer is left untouched except for clearing extra output channels if a host exposes more outputs than inputs.
- The plugin reports zero tail and zero latency.

## Limitations V1

- No FFT or frequency-band animation.
- No video export.
- No embedded image data in plugin state; only the local image path is saved.
- If the image file is moved or deleted, the plugin shows a clean missing-file message.
- No custom preset browser.
- No GPU shader pipeline.
- No Elementary Audio dependency. JUCE is sufficient for this V1 because the POC only needs plugin hosting glue, audio passthrough analysis, image loading, and UI rendering.

## Roadmap V2

- Optional state embedding for small images or project-local asset copy.
- Frequency-band driven animation modes.
- Beat/onset detection.
- Preset management.
- Drag-and-drop image loading.
- Additional transforms: blur, displacement, crop, color tint.
- More robust metering display.
- DAW automation names and parameter grouping polish.
- Optional GPU/OpenGL or shader-based rendering if V1 UI rendering becomes limiting.
