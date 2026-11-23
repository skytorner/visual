#include "SamplerUiComponents.h"
#include "CueRouting.h"
#include "TimelineMath.h"

namespace
{
juce::String formatCueTime(double seconds)
{
    const auto safeSeconds = juce::jmax(0.0, seconds);
    const auto minutes = static_cast<int>(safeSeconds / 60.0);
    const auto secondsInMinute = safeSeconds - static_cast<double>(minutes * 60);
    return juce::String(minutes).paddedLeft('0', 2) + ":"
        + juce::String(secondsInMinute, 2).paddedLeft('0', 5);
}
}

SamplerIconButton::SamplerIconButton(const juce::String& name, Icon buttonIcon)
    : Button(name), icon(buttonIcon)
{
    setClickingTogglesState(buttonIcon == Icon::loop);
    setMouseCursor(juce::MouseCursor::PointingHandCursor);
}

void SamplerIconButton::setIcon(Icon buttonIcon)
{
    if (icon == buttonIcon)
        return;

    icon = buttonIcon;
    repaint();
}

void SamplerIconButton::paintButton(juce::Graphics& graphics, bool isMouseOver, bool isButtonDown)
{
    auto bounds = getLocalBounds().toFloat().reduced(1.0f);
    const auto loopIsOn = icon == Icon::loop && getToggleState();
    const auto disabled = ! isEnabled();
    const auto background = disabled ? juce::Colour(0xff191d24)
        : loopIsOn ? juce::Colour(0xff42dc86)
        : isButtonDown ? juce::Colour(0xff384456)
        : isMouseOver ? juce::Colour(0xff303a49)
        : juce::Colour(0xff202630);
    const auto foreground = disabled ? juce::Colour(0xff596271)
        : loopIsOn ? juce::Colour(0xff11151d) : juce::Colour(0xffe1e7ef);
    const auto centre = bounds.getCentre();
    const auto scale = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 40.0f;

    graphics.setColour(background);
    graphics.fillRoundedRectangle(bounds, 7.0f);
    graphics.setColour(loopIsOn ? background : juce::Colour(0xff3a4556));
    graphics.drawRoundedRectangle(bounds, 7.0f, 1.0f);
    graphics.setColour(foreground);

    if (icon == Icon::play)
    {
        juce::Path triangle;
        triangle.addTriangle(centre.x - 6.0f * scale, centre.y - 9.0f * scale,
                             centre.x - 6.0f * scale, centre.y + 9.0f * scale,
                             centre.x + 9.0f * scale, centre.y);
        graphics.fillPath(triangle);
    }
    else if (icon == Icon::pause)
    {
        graphics.fillRoundedRectangle(centre.x - 7.0f * scale, centre.y - 9.0f * scale,
                                      5.0f * scale, 18.0f * scale, 1.5f * scale);
        graphics.fillRoundedRectangle(centre.x + 2.0f * scale, centre.y - 9.0f * scale,
                                      5.0f * scale, 18.0f * scale, 1.5f * scale);
    }
    else if (icon == Icon::eject)
    {
        juce::Path triangle;
        triangle.addTriangle(centre.x, centre.y - 9.0f * scale,
                             centre.x - 10.0f * scale, centre.y + 3.0f * scale,
                             centre.x + 10.0f * scale, centre.y + 3.0f * scale);
        graphics.fillPath(triangle);
        graphics.fillRoundedRectangle(centre.x - 10.0f * scale, centre.y + 6.0f * scale,
                                      20.0f * scale, 3.0f * scale, 1.0f * scale);
    }
    else
    {
        const auto stroke = juce::PathStrokeType(2.1f * scale,
            juce::PathStrokeType::curved, juce::PathStrokeType::rounded);
        juce::Path upperArc;
        upperArc.startNewSubPath(centre.x - 9.0f * scale, centre.y - 2.0f * scale);
        upperArc.cubicTo(centre.x - 9.0f * scale, centre.y - 12.0f * scale,
                         centre.x + 8.0f * scale, centre.y - 12.0f * scale,
                         centre.x + 8.0f * scale, centre.y - 2.0f * scale);
        graphics.strokePath(upperArc, stroke);
        juce::Path upperArrow;
        upperArrow.addTriangle(centre.x + 8.0f * scale, centre.y - 2.0f * scale,
                               centre.x + 3.0f * scale, centre.y - 5.0f * scale,
                               centre.x + 12.0f * scale, centre.y - 7.0f * scale);
        graphics.fillPath(upperArrow);

        juce::Path lowerArc;
        lowerArc.startNewSubPath(centre.x + 9.0f * scale, centre.y + 2.0f * scale);
        lowerArc.cubicTo(centre.x + 9.0f * scale, centre.y + 12.0f * scale,
                         centre.x - 8.0f * scale, centre.y + 12.0f * scale,
                         centre.x - 8.0f * scale, centre.y + 2.0f * scale);
        graphics.strokePath(lowerArc, stroke);
        juce::Path lowerArrow;
        lowerArrow.addTriangle(centre.x - 8.0f * scale, centre.y + 2.0f * scale,
                               centre.x - 3.0f * scale, centre.y + 5.0f * scale,
                               centre.x - 12.0f * scale, centre.y + 7.0f * scale);
        graphics.fillPath(lowerArrow);
    }
}

void FxControlBar::paint(juce::Graphics& graphics)
{
    auto bounds = getLocalBounds().toFloat();
    graphics.fillAll(juce::Colour(0xff171b22));
    graphics.setColour(juce::Colour(0xff303744));
    graphics.drawRoundedRectangle(bounds.reduced(0.5f), 8.0f, 1.0f);

    auto heading = bounds.reduced(10.0f, 0.0f).removeFromLeft(112.0f);
    graphics.setColour(juce::Colour(0xffd4dbe7));
    graphics.setFont(juce::FontOptions(12.0f).withStyle("Bold"));
    graphics.drawFittedText("FX / PAD CONTROLS", heading.toNearestInt(),
                            juce::Justification::centredLeft, 1);

    auto placeholders = bounds.reduced(10.0f, 8.0f);
    placeholders.removeFromLeft(112.0f);
    constexpr std::array<const char*, 4> names { "LEVEL", "SPEED", "COLOR", "GLITCH" };
    const auto cellWidth = placeholders.getWidth() / static_cast<float>(names.size());
    for (int i = 0; i < static_cast<int>(names.size()); ++i)
    {
        auto cell = placeholders.withX(placeholders.getX() + cellWidth * static_cast<float>(i))
            .withWidth(cellWidth).reduced(3.0f, 0.0f);
        graphics.setColour(juce::Colour(0xff20252d));
        graphics.fillRoundedRectangle(cell, 5.0f);
        graphics.setColour(juce::Colour(0xff3a414c));
        graphics.drawRoundedRectangle(cell, 5.0f, 1.0f);
        graphics.setColour(juce::Colour(0xff8b96a6));
        graphics.setFont(juce::FontOptions(10.0f).withStyle("Bold"));
        graphics.drawFittedText(names[static_cast<std::size_t>(i)],
            cell.withY(cell.getY() + 5.0f).withHeight(15.0f).toNearestInt(),
            juce::Justification::centred, 1);
        graphics.setColour(juce::Colour(0xff697382));
        graphics.setFont(10.0f);
        graphics.drawFittedText("Coming soon", cell.withY(cell.getY() + 21.0f)
            .withHeight(14.0f).toNearestInt(), juce::Justification::centred, 1);
    }
}

AssignedPadsComponent::AssignedPadsComponent()
    : juce::Component("AVSampler Assigned Pads")
{
    setMouseCursor(juce::MouseCursor::PointingHandCursor);
}

void AssignedPadsComponent::setCueData(const std::array<double, cueCount>& positions, int selectedCueIndex)
{
    cuePositions = positions;
    setSelectedCue(selectedCueIndex);
    repaint();
}

void AssignedPadsComponent::setSelectedCue(int cueIndex)
{
    if (cueIndex < 0 || cueIndex >= cueCount || selectedCue == cueIndex)
        return;

    selectedCue = cueIndex;
    repaint();
}

juce::Rectangle<float> AssignedPadsComponent::padBounds(int cueIndex) const
{
    auto row = getLocalBounds().toFloat().reduced(5.0f, 0.0f)
        .withY(22.0f).withHeight(juce::jmax(0.0f, static_cast<float>(getHeight()) - 26.0f));
    const auto cellWidth = row.getWidth() / static_cast<float>(cueCount);
    return row.withX(row.getX() + cellWidth * static_cast<float>(cueIndex))
        .withWidth(cellWidth).reduced(3.0f, 0.0f);
}

void AssignedPadsComponent::paint(juce::Graphics& graphics)
{
    graphics.fillAll(juce::Colour(0xff11151d));
    graphics.setColour(juce::Colour(0xffd4dbe7));
    graphics.setFont(juce::FontOptions(12.0f).withStyle("Bold"));
    graphics.drawText("ASSIGNED PADS", 10, 1, 150, 18, juce::Justification::centredLeft);

    for (int i = 0; i < cueCount; ++i)
    {
        const auto index = static_cast<std::size_t>(i);
        const auto colour = avsampler::ui::cueColours[index];
        const auto pad = padBounds(i);
        graphics.setColour(juce::Colour(0xff1b2636));
        graphics.fillRoundedRectangle(pad, 7.0f);
        graphics.setColour(selectedCue == i ? colour : colour.withAlpha(0.55f));
        graphics.drawRoundedRectangle(pad, 7.0f, selectedCue == i ? 2.0f : 1.0f);

        const auto label = juce::String("CUE ")
            + juce::String::charToString(static_cast<juce::juce_wchar>('A' + i));
        const auto padInt = pad.toNearestInt();
        graphics.setColour(colour);
        graphics.setFont(juce::FontOptions(10.0f).withStyle("Bold"));
        graphics.drawText(label, padInt.withY(padInt.getY() + 2).withHeight(12),
                          juce::Justification::centred);
        graphics.setFont(juce::FontOptions(16.0f).withStyle("Bold"));
        graphics.drawText(avsampler::timeline::midiNoteName(
                              avsampler::defaultCueMidiNotes[index]),
                          padInt.withY(padInt.getY() + 14).withHeight(19),
                          juce::Justification::centred);
        graphics.setColour(juce::Colour(0xffc1cad8));
        graphics.setFont(11.0f);
        graphics.drawText(formatCueTime(cuePositions[index]),
                          padInt.withY(padInt.getY() + 34).withHeight(15),
                          juce::Justification::centred);
    }
}

void AssignedPadsComponent::mouseDown(const juce::MouseEvent& event)
{
    for (int i = 0; i < cueCount; ++i)
    {
        if (padBounds(i).contains(event.position))
        {
            setSelectedCue(i);
            if (onCueSelected)
                onCueSelected(i);
            return;
        }
    }
}
