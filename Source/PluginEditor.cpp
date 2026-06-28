#include "PluginEditor.h"

namespace
{
constexpr int editorWidth = 760;
constexpr int editorHeight = 560;
constexpr int topBarHeight = 48;
constexpr int controlsHeight = 150;
constexpr int controlLabelHeight = 20;
constexpr int controlSliderHeight = 38;
constexpr int controlGap = 8;
}

AudioReactiveImagePOCAudioProcessorEditor::AudioReactiveImagePOCAudioProcessorEditor(
    AudioReactiveImagePOCAudioProcessor& p)
    : AudioProcessorEditor(&p),
      audioProcessor(p)
{
    setSize(editorWidth, editorHeight);

    addAndMakeVisible(uploadButton);
    addAndMakeVisible(resetButton);
    addAndMakeVisible(meterLabel);

    uploadButton.onClick = [this] { openImageChooser(); };
    resetButton.onClick = [this] { resetControlsAndImage(); };

    configureSlider(sensitivitySlider, sensitivityLabel, "Sensitivity");
    configureSlider(smoothingSlider, smoothingLabel, "Smoothing");
    configureSlider(minScaleSlider, minScaleLabel, "Min Scale");
    configureSlider(maxScaleSlider, maxScaleLabel, "Max Scale");
    configureSlider(rotationSlider, rotationLabel, "Rotation");
    configureSlider(opacitySlider, opacityLabel, "Opacity");

    auto& state = audioProcessor.getValueTreeState();

    sensitivityAttachment = std::make_unique<SliderAttachment>(
        state, AudioReactiveImagePOCAudioProcessor::sensitivityParamId, sensitivitySlider);
    smoothingAttachment = std::make_unique<SliderAttachment>(
        state, AudioReactiveImagePOCAudioProcessor::smoothingParamId, smoothingSlider);
    minScaleAttachment = std::make_unique<SliderAttachment>(
        state, AudioReactiveImagePOCAudioProcessor::minScaleParamId, minScaleSlider);
    maxScaleAttachment = std::make_unique<SliderAttachment>(
        state, AudioReactiveImagePOCAudioProcessor::maxScaleParamId, maxScaleSlider);
    rotationAttachment = std::make_unique<SliderAttachment>(
        state, AudioReactiveImagePOCAudioProcessor::rotationAmountParamId, rotationSlider);
    opacityAttachment = std::make_unique<SliderAttachment>(
        state, AudioReactiveImagePOCAudioProcessor::opacityAmountParamId, opacitySlider);

    meterLabel.setJustificationType(juce::Justification::centredRight);
    meterLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);

    loadImageFromPath(audioProcessor.getImagePath());
    startTimerHz(60);
}

void AudioReactiveImagePOCAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff111217));

    auto bounds = getLocalBounds();

    g.setColour(juce::Colour(0xff1d2028));
    g.fillRect(bounds.removeFromTop(topBarHeight));

    g.setColour(juce::Colour(0xff242936));
    g.fillRect(getImageArea());

    g.setColour(juce::Colour(0xff303646));
    g.drawRect(getImageArea(), 1);

    if (! loadedImage.isValid())
    {
        g.setColour(juce::Colours::lightgrey);
        g.setFont(18.0f);
        g.drawFittedText(imageMessage, getImageArea().reduced(24), juce::Justification::centred, 3);
        return;
    }

    const auto& state = audioProcessor.getValueTreeState();
    const auto envelope = juce::jlimit(0.0f, 1.0f, displayedEnvelope);
    const auto minScale = state.getRawParameterValue(AudioReactiveImagePOCAudioProcessor::minScaleParamId)->load();
    const auto maxScale = state.getRawParameterValue(AudioReactiveImagePOCAudioProcessor::maxScaleParamId)->load();
    const auto rotationDegrees =
        state.getRawParameterValue(AudioReactiveImagePOCAudioProcessor::rotationAmountParamId)->load();
    const auto opacityAmount =
        state.getRawParameterValue(AudioReactiveImagePOCAudioProcessor::opacityAmountParamId)->load();

    const auto scale = juce::jmap(envelope, minScale, juce::jmax(minScale, maxScale));
    const auto opacity = juce::jlimit(0.1f, 1.0f, 1.0f - (opacityAmount * 0.45f) + (envelope * opacityAmount * 0.45f));
    const auto rotation = juce::degreesToRadians(rotationDegrees * envelope);

    const auto imageArea = getImageArea().toFloat().reduced(18.0f);
    const auto centre = imageArea.getCentre();
    const auto placement = juce::RectanglePlacement(juce::RectanglePlacement::centred
                                                   | juce::RectanglePlacement::onlyReduceInSize);
    const auto target = placement.appliedTo(loadedImage.getBounds().toFloat(), imageArea);

    juce::Graphics::ScopedSaveState saveState(g);
    g.addTransform(juce::AffineTransform::translation(-centre.x, -centre.y)
                       .scaled(scale)
                       .rotated(rotation)
                       .translated(centre.x, centre.y));

    g.setOpacity(opacity);
    g.drawImage(loadedImage, target);

    if (opacityAmount > 0.0f)
    {
        g.setColour(juce::Colours::white.withAlpha(envelope * opacityAmount * 0.18f));
        g.fillRect(target);
    }
}

void AudioReactiveImagePOCAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(12);
    auto top = bounds.removeFromTop(32);

    uploadButton.setBounds(top.removeFromLeft(150));
    top.removeFromLeft(8);
    resetButton.setBounds(top.removeFromLeft(90));
    meterLabel.setBounds(top);

    bounds.removeFromTop(10);
    auto controls = bounds.removeFromBottom(controlsHeight);

    auto firstRow = controls.removeFromTop(controlLabelHeight + controlSliderHeight);
    auto secondRow = controls.removeFromTop(controlLabelHeight + controlSliderHeight);

    auto layoutSlider = [](juce::Rectangle<int> cell, juce::Label& label, juce::Slider& slider)
    {
        cell = cell.reduced(controlGap / 2, 0);
        label.setBounds(cell.removeFromTop(controlLabelHeight));
        slider.setBounds(cell);
    };

    auto layoutRow = [&layoutSlider](juce::Rectangle<int> row,
                                      juce::Label& labelA,
                                      juce::Slider& sliderA,
                                      juce::Label& labelB,
                                      juce::Slider& sliderB,
                                      juce::Label& labelC,
                                      juce::Slider& sliderC)
    {
        const auto cellWidth = row.getWidth() / 3;
        layoutSlider(row.removeFromLeft(cellWidth), labelA, sliderA);
        layoutSlider(row.removeFromLeft(cellWidth), labelB, sliderB);
        layoutSlider(row, labelC, sliderC);
    };

    layoutRow(firstRow,
              sensitivityLabel, sensitivitySlider,
              smoothingLabel, smoothingSlider,
              minScaleLabel, minScaleSlider);

    layoutRow(secondRow,
              maxScaleLabel, maxScaleSlider,
              rotationLabel, rotationSlider,
              opacityLabel, opacitySlider);
}

void AudioReactiveImagePOCAudioProcessorEditor::timerCallback()
{
    displayedEnvelope += (audioProcessor.getEnvelopeLevel() - displayedEnvelope) * 0.22f;

    meterLabel.setText("RMS "
                           + juce::String(audioProcessor.getRmsLevel(), 3)
                           + "   Peak "
                           + juce::String(audioProcessor.getPeakLevel(), 3)
                           + "   Env "
                           + juce::String(audioProcessor.getEnvelopeLevel(), 3),
                       juce::dontSendNotification);

    repaint(getImageArea().expanded(2));
}

void AudioReactiveImagePOCAudioProcessorEditor::configureSlider(juce::Slider& slider,
                                                                juce::Label& label,
                                                                const juce::String& text)
{
    addAndMakeVisible(slider);
    addAndMakeVisible(label);

    slider.setSliderStyle(juce::Slider::LinearHorizontal);
    slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 72, 22);

    label.setText(text, juce::dontSendNotification);
    label.setColour(juce::Label::textColourId, juce::Colours::whitesmoke);
}

void AudioReactiveImagePOCAudioProcessorEditor::openImageChooser()
{
    fileChooser = std::make_unique<juce::FileChooser>(
        "Choose an image",
        juce::File::getSpecialLocation(juce::File::userPicturesDirectory),
        "*.png;*.jpg;*.jpeg");

    fileChooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        [this](const juce::FileChooser& chooser)
        {
            const auto file = chooser.getResult();

            if (file.existsAsFile())
            {
                audioProcessor.setImagePath(file.getFullPathName());
                loadImageFromPath(file.getFullPathName());
            }
        });
}

void AudioReactiveImagePOCAudioProcessorEditor::loadImageFromPath(const juce::String& path)
{
    loadedImage = {};

    if (path.isEmpty())
    {
        imageMessage = "Upload an image to start";
        repaint();
        return;
    }

    const juce::File file(path);

    if (! file.existsAsFile())
    {
        imageMessage = "Image file not found. Upload a new image.";
        repaint();
        return;
    }

    auto image = juce::ImageFileFormat::loadFrom(file);

    if (! image.isValid())
    {
        imageMessage = "Unsupported or unreadable image. Use PNG or JPG.";
        repaint();
        return;
    }

    loadedImage = image;
    imageMessage = {};
    repaint();
}

void AudioReactiveImagePOCAudioProcessorEditor::resetControlsAndImage()
{
    resetParameter(AudioReactiveImagePOCAudioProcessor::sensitivityParamId, 1.0f);
    resetParameter(AudioReactiveImagePOCAudioProcessor::smoothingParamId, 0.45f);
    resetParameter(AudioReactiveImagePOCAudioProcessor::minScaleParamId, 0.92f);
    resetParameter(AudioReactiveImagePOCAudioProcessor::maxScaleParamId, 1.35f);
    resetParameter(AudioReactiveImagePOCAudioProcessor::rotationAmountParamId, 6.0f);
    resetParameter(AudioReactiveImagePOCAudioProcessor::opacityAmountParamId, 0.25f);

    audioProcessor.setImagePath({});
    audioProcessor.resetVisualState();
    displayedEnvelope = 0.0f;
    loadImageFromPath({});
}

void AudioReactiveImagePOCAudioProcessorEditor::resetParameter(const char* parameterId, float defaultValue)
{
    if (auto* parameter = audioProcessor.getValueTreeState().getParameter(parameterId))
    {
        parameter->beginChangeGesture();
        parameter->setValueNotifyingHost(parameter->convertTo0to1(defaultValue));
        parameter->endChangeGesture();
    }
}

juce::Rectangle<int> AudioReactiveImagePOCAudioProcessorEditor::getImageArea() const
{
    auto area = getLocalBounds().reduced(12);
    area.removeFromTop(topBarHeight);
    area.removeFromBottom(controlsHeight + 14);
    return area;
}
