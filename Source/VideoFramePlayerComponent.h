#pragma once

#include <JuceHeader.h>

#include <functional>
#include <memory>

/** Platform-selected video surface. macOS uses the AVFoundation/Metal renderer;
    Windows currently uses JUCE VideoComponent for the playback/control spike.
*/
class VideoFramePlayerComponent final : public juce::Component
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
    bool isRgbSplitAvailable() const noexcept;
    void setFlashEnabled(bool enabled);
    void triggerFlashVelocity(int velocity);
    bool isFlashAvailable() const noexcept;
    void setZoomPunchEnabled(bool enabled);
    void triggerZoomPunchVelocity(int velocity);
    bool isZoomPunchAvailable() const noexcept;
    void resized() override;

    std::function<void(const juce::String&)> onErrorOccurred;

private:
    struct Impl;
    std::unique_ptr<Impl> impl;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VideoFramePlayerComponent)
};
