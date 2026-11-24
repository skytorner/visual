#include "VideoFramePlayerComponent.h"

struct VideoFramePlayerComponent::Impl
{
    explicit Impl(VideoFramePlayerComponent& ownerToUse)
        : owner(ownerToUse)
    {
        owner.addAndMakeVisible(videoSurface);
    }

    VideoFramePlayerComponent& owner;
    juce::VideoComponent videoSurface { false };
    juce::URL currentURL;
    double playSpeed = 1.0;
    float audioVolume = 0.0f;
};

VideoFramePlayerComponent::VideoFramePlayerComponent()
    : impl(std::make_unique<Impl>(*this))
{
}

VideoFramePlayerComponent::~VideoFramePlayerComponent() = default;

void VideoFramePlayerComponent::resized()
{
    if (impl != nullptr)
        impl->videoSurface.setBounds(getLocalBounds());
}

void VideoFramePlayerComponent::loadAsync(const juce::URL& url, LoadCallback callback)
{
    if (! url.isLocalFile())
    {
        callback(url, juce::Result::fail("Only local video files are supported"));
        return;
    }

    const auto file = url.getLocalFile();
    if (! file.existsAsFile())
    {
        callback(url, juce::Result::fail("Video file does not exist"));
        return;
    }

    impl->currentURL = url;
    impl->videoSurface.loadAsync(url, [this, url, callback = std::move(callback)]
        (const juce::URL&, juce::Result result) mutable
    {
        if (result.failed() && onErrorOccurred)
            onErrorOccurred(result.getErrorMessage());
        callback(url, result);
    });
}

void VideoFramePlayerComponent::closeVideo()
{
    impl->videoSurface.closeVideo();
    impl->currentURL = {};
}

bool VideoFramePlayerComponent::isVideoOpen() const
{
    return impl->videoSurface.isVideoOpen();
}

double VideoFramePlayerComponent::getVideoDuration() const
{
    return isVideoOpen() ? impl->videoSurface.getVideoDuration() : 0.0;
}

juce::Rectangle<int> VideoFramePlayerComponent::getVideoNativeSize() const
{
    return isVideoOpen() ? impl->videoSurface.getVideoNativeSize() : juce::Rectangle<int>();
}

void VideoFramePlayerComponent::play()
{
    impl->videoSurface.setPlaySpeed(impl->playSpeed);
    impl->videoSurface.play();
}

void VideoFramePlayerComponent::stop()
{
    impl->videoSurface.stop();
}

bool VideoFramePlayerComponent::isPlaying() const
{
    return impl->videoSurface.isPlaying();
}

void VideoFramePlayerComponent::setPlayPosition(double seconds)
{
    impl->videoSurface.setPlayPosition(seconds);
}

double VideoFramePlayerComponent::getPlayPosition() const
{
    return impl->videoSurface.getPlayPosition();
}

void VideoFramePlayerComponent::setPlaySpeed(double speed)
{
    impl->playSpeed = juce::jlimit(0.25, 4.0, speed);
    impl->videoSurface.setPlaySpeed(impl->playSpeed);
}

double VideoFramePlayerComponent::getPlaySpeed() const
{
    return impl->playSpeed;
}

void VideoFramePlayerComponent::setAudioVolume(float volume)
{
    impl->audioVolume = juce::jlimit(0.0f, 1.0f, volume);
    impl->videoSurface.setAudioVolume(impl->audioVolume);
}

float VideoFramePlayerComponent::getAudioVolume() const
{
    return impl->audioVolume;
}

void VideoFramePlayerComponent::setRgbSplitEnabled(bool enabled)
{
    juce::ignoreUnused(enabled);
}

void VideoFramePlayerComponent::triggerRgbSplitVelocity(int velocity)
{
    juce::ignoreUnused(velocity);
}

bool VideoFramePlayerComponent::isRgbSplitAvailable() const noexcept
{
    return false;
}

void VideoFramePlayerComponent::setFlashEnabled(bool enabled)
{
    juce::ignoreUnused(enabled);
}

void VideoFramePlayerComponent::triggerFlashVelocity(int velocity)
{
    juce::ignoreUnused(velocity);
}

bool VideoFramePlayerComponent::isFlashAvailable() const noexcept
{
    return false;
}

void VideoFramePlayerComponent::setZoomPunchEnabled(bool enabled)
{
    juce::ignoreUnused(enabled);
}

void VideoFramePlayerComponent::triggerZoomPunchVelocity(int velocity)
{
    juce::ignoreUnused(velocity);
}

bool VideoFramePlayerComponent::isZoomPunchAvailable() const noexcept
{
    return false;
}

void VideoFramePlayerComponent::setColorBoostEnabled(bool enabled)
{
    juce::ignoreUnused(enabled);
}

void VideoFramePlayerComponent::triggerColorBoostVelocity(int velocity)
{
    juce::ignoreUnused(velocity);
}

bool VideoFramePlayerComponent::isColorBoostAvailable() const noexcept
{
    return false;
}

void VideoFramePlayerComponent::setVideoEchoEnabled(bool enabled)
{
    juce::ignoreUnused(enabled);
}

void VideoFramePlayerComponent::triggerVideoEchoVelocity(int velocity)
{
    juce::ignoreUnused(velocity);
}

bool VideoFramePlayerComponent::isVideoEchoAvailable() const noexcept
{
    return false;
}
