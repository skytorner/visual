#import <AVFoundation/AVFoundation.h>
#import <CoreVideo/CoreVideo.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <QuartzCore/QuartzCore.h>

#include "VideoFramePlayerComponent.h"
#include "VideoEffectMath.h"

#include <cmath>

@interface AVSamplerMetalVideoView : MTKView <MTKViewDelegate>
@property(nonatomic, strong) AVPlayer* player;
@property(nonatomic, strong) AVPlayerItem* playerItem;
@property(nonatomic, strong) AVPlayerItemVideoOutput* videoOutput;
@property(nonatomic, strong) id<MTLCommandQueue> commandQueue;
@property(nonatomic, strong) id<MTLRenderPipelineState> pipeline;
@property(nonatomic, strong) id<MTLTexture> currentFrameTexture;
@property(nonatomic, assign) CVMetalTextureCacheRef textureCache;
@property(nonatomic, assign) CVMetalTextureRef currentVideoTexture;
@property(nonatomic, assign) CGSize nativeVideoSize;
@property(nonatomic, assign) double playSpeed;
@property(nonatomic, assign) float audioVolume;
@property(nonatomic, assign) BOOL rgbSplitEnabled;
@property(nonatomic, assign) float rgbSplitIntensity;
@property(nonatomic, assign) CFTimeInterval rgbSplitStartTime;
@property(nonatomic, assign) BOOL flashEnabled;
@property(nonatomic, assign) int flashVelocity;
@property(nonatomic, assign) CFTimeInterval flashStartTime;
@property(nonatomic, assign) NSUInteger loadGeneration;
@property(nonatomic, copy) NSString* rendererSetupError;

- (void)loadURL:(NSURL*)url completion:(void (^)(NSError*))completion;
- (void)closeVideo;
- (void)shutdown;
- (void)seekToSeconds:(double)seconds;
- (void)setRgbSplitEnabled:(BOOL)enabled;
- (void)triggerRgbSplitVelocity:(int)velocity;
- (void)setFlashEnabled:(BOOL)enabled;
- (void)triggerFlashVelocity:(int)velocity;
@end

@implementation AVSamplerMetalVideoView

- (instancetype)initWithFrame:(CGRect)frame device:(id<MTLDevice>)device
{
    self = [super initWithFrame:frame device:device];
    if (self == nil)
        return nil;

    self.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
    self.colorPixelFormat = MTLPixelFormatBGRA8Unorm;
    self.framebufferOnly = YES;
    self.preferredFramesPerSecond = 60;
    self.paused = NO;
    self.enableSetNeedsDisplay = NO;
    self.delegate = self;
    self.playSpeed = 1.0;
    self.audioVolume = 0.0f;
    self.clearColor = MTLClearColorMake(0.0, 0.0, 0.0, 1.0);

    if (device == nil)
    {
        self.rendererSetupError = @"No Metal device is available";
        return self;
    }

    self.commandQueue = [device newCommandQueue];
    if (self.commandQueue == nil)
    {
        self.rendererSetupError = @"Could not create a Metal command queue";
        return self;
    }

    NSDictionary* cacheAttributes = @{};
    CVReturn cacheResult = CVMetalTextureCacheCreate(kCFAllocatorDefault,
                                                      (__bridge CFDictionaryRef)cacheAttributes,
                                                      (id<MTLDevice> _Nonnull)device,
                                                      nullptr,
                                                      &_textureCache);
    if (cacheResult != kCVReturnSuccess)
    {
        self.rendererSetupError = @"Could not create the Metal pixel buffer cache";
        return self;
    }

    static const char* shaderSource = R"METAL(
        #include <metal_stdlib>
        using namespace metal;
        struct VertexOut { float4 position [[position]]; float2 uv; };
        vertex VertexOut avs_vertex(uint id [[vertex_id]], constant float2& fit [[buffer(0)]]) {
            constexpr float2 positions[6] = {
                float2(-1.0, -1.0), float2( 1.0, -1.0), float2(-1.0,  1.0),
                float2(-1.0,  1.0), float2( 1.0, -1.0), float2( 1.0,  1.0)
            };
            constexpr float2 coordinates[6] = {
                float2(0.0, 1.0), float2(1.0, 1.0), float2(0.0, 0.0),
                float2(0.0, 0.0), float2(1.0, 1.0), float2(1.0, 0.0)
            };
            VertexOut output;
            output.position = float4(positions[id] * fit, 0.0, 1.0);
            output.uv = coordinates[id];
            return output;
        }
        fragment float4 avs_fragment(VertexOut input [[stage_in]],
                                     constant float2& effects [[buffer(0)]],
                                     texture2d<float> frame [[texture(0)]]) {
            constexpr sampler frameSampler(coord::normalized, address::clamp_to_edge, filter::linear);
            float4 color;
            if (effects.x > 0.0) {
                const float2 redUv = clamp(input.uv - float2(effects.x, 0.0), float2(0.0), float2(1.0));
                const float2 greenUv = clamp(input.uv, float2(0.0), float2(1.0));
                const float2 blueUv = clamp(input.uv + float2(effects.x, 0.0), float2(0.0), float2(1.0));
                const float4 redSample = frame.sample(frameSampler, redUv);
                const float4 greenSample = frame.sample(frameSampler, greenUv);
                const float4 blueSample = frame.sample(frameSampler, blueUv);
                color = float4(redSample.r, greenSample.g, blueSample.b, greenSample.a);
            }
            else {
                color = frame.sample(frameSampler, input.uv);
            }
            if (effects.y > 0.0)
                color.rgb = mix(color.rgb, float3(1.0), clamp(effects.y, 0.0, 0.65));
            return color;
        }
    )METAL";

    NSError* shaderError = nil;
    NSString* metalSource = [NSString stringWithUTF8String:shaderSource];
    id<MTLLibrary> library = [device newLibraryWithSource:metalSource options:nil error:&shaderError];
    if (library == nil)
    {
        self.rendererSetupError = shaderError.localizedDescription;
        return self;
    }

    MTLRenderPipelineDescriptor* descriptor = [[MTLRenderPipelineDescriptor alloc] init];
    descriptor.vertexFunction = [library newFunctionWithName:@"avs_vertex"];
    descriptor.fragmentFunction = [library newFunctionWithName:@"avs_fragment"];
    descriptor.colorAttachments[0].pixelFormat = self.colorPixelFormat;
    self.pipeline = [device newRenderPipelineStateWithDescriptor:descriptor error:&shaderError];
    if (self.pipeline == nil)
        self.rendererSetupError = shaderError.localizedDescription;
    return self;
}

- (void)loadURL:(NSURL*)url completion:(void (^)(NSError*))completion
{
    [self closeVideo];
    const auto generation = ++self.loadGeneration;

    if (self.rendererSetupError != nil)
    {
        completion([NSError errorWithDomain:@"AVSamplerMetal"
                                       code:1
                                   userInfo:@{NSLocalizedDescriptionKey: self.rendererSetupError}]);
        return;
    }

    NSDictionary* outputSettings = @{
        (id)kCVPixelBufferPixelFormatTypeKey: @(kCVPixelFormatType_32BGRA)
    };
    AVURLAsset* asset = [AVURLAsset URLAssetWithURL:url options:nil];
    [asset loadValuesAsynchronouslyForKeys:@[@"tracks", @"duration"] completionHandler:^{
        NSError* keyError = nil;
        AVKeyValueStatus status = [asset statusOfValueForKey:@"tracks" error:&keyError];
        if (status != AVKeyValueStatusLoaded)
        {
            dispatch_async(dispatch_get_main_queue(), ^{
                if (generation != self.loadGeneration)
                    return;
                completion(keyError != nil ? keyError : [NSError errorWithDomain:@"AVSamplerVideo"
                                                                              code:1
                                                                          userInfo:@{NSLocalizedDescriptionKey: @"Could not read video tracks"}]);
            });
            return;
        }

        [asset loadTracksWithMediaType:AVMediaTypeVideo completionHandler:^(NSArray<AVAssetTrack*>* tracks, NSError* trackError) {
            dispatch_async(dispatch_get_main_queue(), ^{
                if (generation != self.loadGeneration)
                    return;

                if (trackError != nil)
                {
                    completion(trackError);
                    return;
                }

                AVAssetTrack* videoTrack = tracks.firstObject;
                if (videoTrack == nil)
                {
                    completion([NSError errorWithDomain:@"AVSamplerVideo"
                                                    code:2
                                                userInfo:@{NSLocalizedDescriptionKey: @"No video track found"}]);
                    return;
                }

                CGSize transformedSize = CGSizeApplyAffineTransform(videoTrack.naturalSize,
                                                                       videoTrack.preferredTransform);
                self.nativeVideoSize = CGSizeMake(fabs(transformedSize.width), fabs(transformedSize.height));

                self.playerItem = [AVPlayerItem playerItemWithAsset:asset];
                self.videoOutput = [[AVPlayerItemVideoOutput alloc] initWithPixelBufferAttributes:outputSettings];
                [self.playerItem addOutput:self.videoOutput];
                self.player = [AVPlayer playerWithPlayerItem:self.playerItem];
                self.player.volume = self.audioVolume;
                self.player.actionAtItemEnd = AVPlayerActionAtItemEndPause;
                completion(nil);
            });
        }];
    }];
}

- (void)closeVideo
{
    ++self.loadGeneration;
    [self.player pause];
    if (self.playerItem != nil && self.videoOutput != nil)
        [self.playerItem removeOutput:self.videoOutput];
    self.player = nil;
    self.playerItem = nil;
    self.videoOutput = nil;
    self.currentFrameTexture = nil;
    self.rgbSplitIntensity = 0.0f;
    self.nativeVideoSize = CGSizeZero;
    if (_currentVideoTexture != nullptr)
    {
        CFRelease(_currentVideoTexture);
        _currentVideoTexture = nullptr;
    }
    if (_textureCache != nullptr)
        CVMetalTextureCacheFlush(_textureCache, 0);
}

- (void)setRgbSplitEnabled:(BOOL)enabled
{
    _rgbSplitEnabled = enabled;
    if (! enabled)
        self.rgbSplitIntensity = 0.0f;
}

- (void)triggerRgbSplitVelocity:(int)velocity
{
    if (! self.rgbSplitEnabled || velocity <= 0)
        return;

    self.rgbSplitIntensity = avsampler::videoEffects::rgbSplitIntensityForVelocity(velocity);
    self.rgbSplitStartTime = CACurrentMediaTime();
}

- (void)setFlashEnabled:(BOOL)enabled
{
    _flashEnabled = enabled;
    if (! enabled)
        self.flashVelocity = 0;
}

- (void)triggerFlashVelocity:(int)velocity
{
    if (! self.flashEnabled || velocity <= 0)
        return;

    self.flashVelocity = juce::jlimit(1, 127, velocity);
    self.flashStartTime = CACurrentMediaTime();
}

- (void)shutdown
{
    [self closeVideo];
    if (_textureCache != nullptr)
    {
        CFRelease(_textureCache);
        _textureCache = nullptr;
    }
}

- (void)seekToSeconds:(double)seconds
{
    if (self.player == nil)
        return;

    const auto time = CMTimeMakeWithSeconds(juce::jmax(0.0, seconds), 600);
    [self.player seekToTime:time toleranceBefore:kCMTimeZero toleranceAfter:kCMTimeZero];
}

- (void)drawInMTKView:(MTKView*)view
{
    MTLRenderPassDescriptor* pass = view.currentRenderPassDescriptor;
    id<CAMetalDrawable> drawable = view.currentDrawable;
    id<MTLCommandBuffer> commandBuffer = [self.commandQueue commandBuffer];
    if (pass == nil || drawable == nil || commandBuffer == nil)
        return;

    if (self.player != nil && self.videoOutput != nil && _textureCache != nullptr)
    {
        const CMTime itemTime = self.player.currentTime;
        if ([self.videoOutput hasNewPixelBufferForItemTime:itemTime])
        {
            CMTime displayTime = kCMTimeInvalid;
            CVPixelBufferRef pixelBuffer = [self.videoOutput copyPixelBufferForItemTime:itemTime
                                                                      itemTimeForDisplay:&displayTime];
            if (pixelBuffer != nullptr)
            {
                CVMetalTextureRef newTexture = nullptr;
                const auto width = CVPixelBufferGetWidth(pixelBuffer);
                const auto height = CVPixelBufferGetHeight(pixelBuffer);
                const CVReturn textureResult = CVMetalTextureCacheCreateTextureFromImage(
                    kCFAllocatorDefault, _textureCache, pixelBuffer, nullptr,
                    MTLPixelFormatBGRA8Unorm, width, height, 0, &newTexture);
                CVPixelBufferRelease(pixelBuffer);

                if (textureResult == kCVReturnSuccess && newTexture != nullptr)
                {
                    if (_currentVideoTexture != nullptr)
                        CFRelease(_currentVideoTexture);
                    _currentVideoTexture = newTexture;
                    self.currentFrameTexture = CVMetalTextureGetTexture(newTexture);
                }
                else if (newTexture != nullptr)
                {
                    CFRelease(newTexture);
                }
            }
        }
    }

    pass.colorAttachments[0].clearColor = MTLClearColorMake(0.0, 0.0, 0.0, 1.0);
    id<MTLRenderCommandEncoder> encoder = [commandBuffer renderCommandEncoderWithDescriptor:pass];
    if (encoder == nil)
        return;

    if (self.pipeline != nil && self.currentFrameTexture != nil)
    {
        const double videoAspect = self.currentFrameTexture.width > 0 && self.currentFrameTexture.height > 0
            ? static_cast<double>(self.currentFrameTexture.width)
                / static_cast<double>(self.currentFrameTexture.height) : 1.0;
        const double viewAspect = view.drawableSize.height > 0.0
            ? view.drawableSize.width / view.drawableSize.height : videoAspect;
        vector_float2 fit = { 1.0f, 1.0f };
        if (videoAspect > viewAspect)
            fit.y = static_cast<float>(viewAspect / videoAspect);
        else
            fit.x = static_cast<float>(videoAspect / viewAspect);

        [encoder setRenderPipelineState:self.pipeline];
        [encoder setVertexBytes:&fit length:sizeof(fit) atIndex:0];
        const auto currentTime = CACurrentMediaTime();
        const auto rgbSplitElapsedSeconds = currentTime - self.rgbSplitStartTime;
        const auto rgbSplitEnvelope = avsampler::videoEffects::rgbSplitEnvelopeForElapsedSeconds(
            rgbSplitElapsedSeconds);
        const auto offsetUv = self.rgbSplitEnabled
            ? avsampler::videoEffects::rgbSplitMaximumOffsetUv(self.rgbSplitIntensity, rgbSplitEnvelope) : 0.0;
        const auto flashElapsedSeconds = currentTime - self.flashStartTime;
        const auto flashWhiteMix = self.flashEnabled
            ? avsampler::videoEffects::flashWhiteMixForVelocityAndElapsedSeconds(
                self.flashVelocity, flashElapsedSeconds) : 0.0;
        const float effectValues[2] {
            static_cast<float>(offsetUv), static_cast<float>(flashWhiteMix)
        };
        [encoder setFragmentBytes:effectValues length:sizeof(effectValues) atIndex:0];
        [encoder setFragmentTexture:self.currentFrameTexture atIndex:0];
        [encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:6];
    }

    [encoder endEncoding];
    [commandBuffer presentDrawable:drawable];
    [commandBuffer commit];
}

- (void)mtkView:(MTKView*)view drawableSizeWillChange:(CGSize)size
{
    juce::ignoreUnused(view, size);
}

@end

struct VideoFramePlayerComponent::Impl
{
    explicit Impl(VideoFramePlayerComponent& ownerToUse)
        : owner(ownerToUse)
    {
        owner.addAndMakeVisible(videoSurface);
        id<MTLDevice> device = MTLCreateSystemDefaultDevice();
        view = [[AVSamplerMetalVideoView alloc] initWithFrame:NSZeroRect device:device];
        videoSurface.setView((__bridge void*)view);
    }

    ~Impl()
    {
        videoSurface.setView(nullptr);
        [view shutdown];
        view.delegate = nil;
        view = nil;
    }

    VideoFramePlayerComponent& owner;
    juce::NSViewComponent videoSurface;
    __strong AVSamplerMetalVideoView* view = nil;
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
    AVSamplerMetalVideoView* view = impl->view;
    const auto audioVolume = impl->audioVolume;
    const auto filePath = url.getLocalFile().getFullPathName();
    NSString* nativePath = [NSString stringWithUTF8String:filePath.toRawUTF8()];
    if (nativePath == nil)
    {
        callback(url, juce::Result::fail("Could not convert the video path"));
        return;
    }
    NSURL* nativeURL = [NSURL fileURLWithPath:nativePath];
    [view loadURL:nativeURL
       completion:^(NSError* error)
    {
        if (error != nil)
        {
            callback(url, juce::Result::fail(juce::String::fromUTF8(error.localizedDescription.UTF8String)));
            return;
        }

        view.player.volume = audioVolume;
        callback(url, juce::Result::ok());
    }];
}

void VideoFramePlayerComponent::closeVideo()
{
    [impl->view closeVideo];
    impl->currentURL = {};
}

bool VideoFramePlayerComponent::isVideoOpen() const { return impl->view.playerItem != nil; }

double VideoFramePlayerComponent::getVideoDuration() const
{
    if (impl->view.playerItem == nil)
        return 0.0;
    const auto seconds = CMTimeGetSeconds(impl->view.playerItem.duration);
    return std::isfinite(seconds) ? seconds : 0.0;
}

juce::Rectangle<int> VideoFramePlayerComponent::getVideoNativeSize() const
{
    const auto size = impl->view.nativeVideoSize;
    return { juce::roundToInt(size.width), juce::roundToInt(size.height) };
}

void VideoFramePlayerComponent::play()
{
    if (impl->view.player != nil)
        [impl->view.player playImmediatelyAtRate:static_cast<float>(impl->playSpeed)];
}

void VideoFramePlayerComponent::stop() { [impl->view.player pause]; }
bool VideoFramePlayerComponent::isPlaying() const { return impl->view.player.rate != 0.0f; }

void VideoFramePlayerComponent::setPlayPosition(double seconds)
{
    [impl->view seekToSeconds:seconds];
}

double VideoFramePlayerComponent::getPlayPosition() const
{
    if (impl->view.player == nil)
        return 0.0;
    const auto seconds = CMTimeGetSeconds(impl->view.player.currentTime);
    return std::isfinite(seconds) ? seconds : 0.0;
}

void VideoFramePlayerComponent::setPlaySpeed(double speed)
{
    impl->playSpeed = juce::jlimit(0.25, 4.0, speed);
    if (isPlaying())
        impl->view.player.rate = static_cast<float>(impl->playSpeed);
}

double VideoFramePlayerComponent::getPlaySpeed() const { return impl->playSpeed; }

void VideoFramePlayerComponent::setAudioVolume(float volume)
{
    impl->audioVolume = juce::jlimit(0.0f, 1.0f, volume);
    impl->view.player.volume = impl->audioVolume;
}

float VideoFramePlayerComponent::getAudioVolume() const { return impl->audioVolume; }

void VideoFramePlayerComponent::setRgbSplitEnabled(bool enabled)
{
    [impl->view setRgbSplitEnabled:enabled];
}

void VideoFramePlayerComponent::triggerRgbSplitVelocity(int velocity)
{
    [impl->view triggerRgbSplitVelocity:velocity];
}

bool VideoFramePlayerComponent::isRgbSplitAvailable() const noexcept { return true; }

void VideoFramePlayerComponent::setFlashEnabled(bool enabled)
{
    [impl->view setFlashEnabled:enabled];
}

void VideoFramePlayerComponent::triggerFlashVelocity(int velocity)
{
    [impl->view triggerFlashVelocity:velocity];
}

bool VideoFramePlayerComponent::isFlashAvailable() const noexcept { return true; }
