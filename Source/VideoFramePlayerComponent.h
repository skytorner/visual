#pragma once

#include <JuceHeader.h>

#include <functional>
#include <memory>

/** macOS video surface that pulls decoded AVFoundation frames and displays them
    through a Metal-backed native view. This is the VISUAL-014a renderer spike;
    it intentionally does not apply visual effects yet.
*/
class VideoFramePlayerComponent final : public juce::NSViewComponent
{
public:
    using LoadCallback = std::function<void(const juce::URL&, juce::Result)>;

    VideoFramePlayerComponent();
    ~VideoFramePlayerComponent() override;

    void loadAsync(const juce::URL& url, LoadCallback callback);
    void closeVideo();
    bool isVideoOpen() const;
    double getVideoDuration() const;
    juce::Rectangle<int> getVideoNativeSize() const;
    void play();
    void stop();
    bool isPlaying() const;
    void setPlayPosition(double seconds);
    double getPlayPosition() const;
    void setPlaySpeed(double speed);
    double getPlaySpeed() const;
    void setAudioVolume(float volume);
    float getAudioVolume() const;
    void setRgbSplitEnabled(bool enabled);
    void triggerRgbSplitVelocity(int velocity);

    std::function<void(const juce::String&)> onErrorOccurred;

private:
    struct Impl;
    std::unique_ptr<Impl> impl;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VideoFramePlayerComponent)
};
