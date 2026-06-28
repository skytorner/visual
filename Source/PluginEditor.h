#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class AudioReactiveImagePOCAudioProcessorEditor final
    : public juce::AudioProcessorEditor,
      private juce::Timer
{
public:
    explicit AudioReactiveImagePOCAudioProcessorEditor(AudioReactiveImagePOCAudioProcessor&);
    ~AudioReactiveImagePOCAudioProcessorEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    void timerCallback() override;
    void configureSlider(juce::Slider& slider, juce::Label& label, const juce::String& text);
    void openImageChooser();
    void loadImageFromPath(const juce::String& path);
    void resetControlsAndImage();
    void resetParameter(const char* parameterId, float defaultValue);
    juce::Rectangle<int> getImageArea() const;

    AudioReactiveImagePOCAudioProcessor& audioProcessor;

    juce::TextButton uploadButton { "Upload Image" };
    juce::TextButton resetButton { "Reset" };

    juce::Slider sensitivitySlider;
    juce::Slider smoothingSlider;
    juce::Slider minScaleSlider;
    juce::Slider maxScaleSlider;
    juce::Slider rotationSlider;
    juce::Slider opacitySlider;

    juce::Label sensitivityLabel;
    juce::Label smoothingLabel;
    juce::Label minScaleLabel;
    juce::Label maxScaleLabel;
    juce::Label rotationLabel;
    juce::Label opacityLabel;
    juce::Label meterLabel;

    std::unique_ptr<SliderAttachment> sensitivityAttachment;
    std::unique_ptr<SliderAttachment> smoothingAttachment;
    std::unique_ptr<SliderAttachment> minScaleAttachment;
    std::unique_ptr<SliderAttachment> maxScaleAttachment;
    std::unique_ptr<SliderAttachment> rotationAttachment;
    std::unique_ptr<SliderAttachment> opacityAttachment;

    std::unique_ptr<juce::FileChooser> fileChooser;

    juce::Image loadedImage;
    juce::String imageMessage { "Upload an image to start" };

    float displayedEnvelope = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioReactiveImagePOCAudioProcessorEditor)
};

