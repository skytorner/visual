#pragma once

#include <JuceHeader.h>
#include <array>
#include <functional>

namespace avsampler::ui
{
inline const std::array<juce::Colour, 4> cueColours {
    juce::Colour(0xff42dc86), juce::Colour(0xff3bd5df),
    juce::Colour(0xffb676f4), juce::Colour(0xffffa04e)
};
}

class SamplerIconButton final : public juce::Button
{
public:
    enum class Icon { play, pause, loop, eject };

    SamplerIconButton(const juce::String& name, Icon buttonIcon);
    void setIcon(Icon buttonIcon);
    void paintButton(juce::Graphics& graphics, bool isMouseOver, bool isButtonDown) override;

private:
    Icon icon;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SamplerIconButton)
};

class FxControlBar final : public juce::Component
{
public:
    FxControlBar();
    void setSpeedMultiplier(double multiplier);
    void setRgbSplitEnabled(bool enabled);
    void setLastMidiVelocity(int velocity);
    void paint(juce::Graphics& graphics) override;
    void resized() override;

    std::function<void(bool)> onRgbSplitEnabledChanged;

private:
    double speedMultiplier = 1.0;
    bool rgbSplitEnabled = true;
    int lastMidiVelocity = 0;
    juce::TextButton rgbSplitButton { "RGB SPLIT" };
};

class AssignedPadsComponent final : public juce::Component
{
public:
    static constexpr int cueCount = 4;

    AssignedPadsComponent();
    void setCueData(const std::array<double, cueCount>& positions, int selectedCueIndex);
    void setSelectedCue(int cueIndex);
    void paint(juce::Graphics& graphics) override;
    void mouseDown(const juce::MouseEvent& event) override;

    std::function<void(int)> onCueSelected;

private:
    juce::Rectangle<float> padBounds(int cueIndex) const;
    std::array<double, cueCount> cuePositions {};
    int selectedCue = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AssignedPadsComponent)
};
