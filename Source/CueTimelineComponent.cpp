#include "CueTimelineComponent.h"
#include "CueRouting.h"
#include "TimelineMath.h"

#include <algorithm>
#include <cmath>

namespace
{
constexpr juce::uint32 background = 0xff11151d;
constexpr juce::uint32 trackFill = 0xff263246;
const std::array<juce::Colour, CueTimelineComponent::cueCount> cueColours {
    juce::Colour(0xff42dc86), juce::Colour(0xff3bd5df),
    juce::Colour(0xffb676f4), juce::Colour(0xffffa04e)
};

juce::String formatTime(double seconds)
{
    return juce::String(juce::jmax(0.0, seconds), 2) + " s";
}

double rulerStepFor(double duration)
{
    if (duration <= 0.0)
        return 1.0;

    const auto rawStep = duration / 5.0;
    const auto magnitude = std::pow(10.0, std::floor(std::log10(rawStep)));
    const auto normalized = rawStep / magnitude;
    const auto multiplier = normalized <= 1.0 ? 1.0 : normalized <= 2.0 ? 2.0
        : normalized <= 5.0 ? 5.0 : 10.0;
    return multiplier * magnitude;
}
}

CueTimelineComponent::CueTimelineComponent()
{
    setWantsKeyboardFocus(true);
}

void CueTimelineComponent::setTimelineData(double newDuration, double newPlayhead,
                                           const std::array<double, cueCount>& positions)
{
    duration = std::isfinite(newDuration) && newDuration > 0.0 ? newDuration : 0.0;
    playhead = avsampler::timeline::clampCuePosition(newPlayhead, duration);
    for (int i = 0; i < cueCount; ++i)
        cuePositions[static_cast<std::size_t>(i)] = avsampler::timeline::clampCuePosition(
            positions[static_cast<std::size_t>(i)], duration);
    repaint();
}

void CueTimelineComponent::setActiveCue(int cueIndex)
{
    activeCue = cueIndex >= 0 && cueIndex < cueCount ? cueIndex : -1;
    repaint();
}

void CueTimelineComponent::setSelectedCue(int cueIndex)
{
    if (cueIndex < 0 || cueIndex >= cueCount || selectedCue == cueIndex)
        return;

    selectedCue = cueIndex;
    repaint();
}

void CueTimelineComponent::selectCue(int cueIndex)
{
    if (cueIndex < 0 || cueIndex >= cueCount)
        return;

    selectedCue = cueIndex;
    if (onCueSelected)
        onCueSelected(cueIndex);
    repaint();
}

juce::Rectangle<float> CueTimelineComponent::trackBounds() const
{
    return getLocalBounds().toFloat().reduced(14.0f, 0.0f).withY(55.0f).withHeight(52.0f);
}

juce::Rectangle<float> CueTimelineComponent::cardBounds(int cueIndex) const
{
    auto cards = getLocalBounds().toFloat().reduced(8.0f, 0.0f)
        .withY(static_cast<float>(getHeight()) - 45.0f).withHeight(40.0f);
    const auto width = cards.getWidth() / static_cast<float>(cueCount);
    return cards.withX(cards.getX() + width * static_cast<float>(cueIndex))
        .withWidth(width).reduced(3.0f, 0.0f);
}

std::array<float, CueTimelineComponent::cueCount> CueTimelineComponent::markerLabelCentres(
    juce::Rectangle<float> track) const
{
    std::array<float, cueCount> cueXs {};
    std::array<float, cueCount> labelXs {};
    std::array<int, cueCount> order { 0, 1, 2, 3 };
    for (int i = 0; i < cueCount; ++i)
        cueXs[static_cast<std::size_t>(i)] = static_cast<float>(avsampler::timeline::xAtTime(
            cuePositions[static_cast<std::size_t>(i)], track.getX(), track.getWidth(), duration));
    std::sort(order.begin(), order.end(), [&cueXs](int first, int second)
    {
        return cueXs[static_cast<std::size_t>(first)] < cueXs[static_cast<std::size_t>(second)];
    });

    constexpr float labelSpacing = 38.0f;
    auto previous = track.getX() - labelSpacing + 17.0f;
    for (const auto cueIndex : order)
    {
        auto& labelX = labelXs[static_cast<std::size_t>(cueIndex)];
        labelX = juce::jmax(cueXs[static_cast<std::size_t>(cueIndex)], previous + labelSpacing);
        previous = labelX;
    }

    const auto overflow = labelXs[static_cast<std::size_t>(order.back())] - (track.getRight() - 17.0f);
    if (overflow > 0.0f)
        for (auto& labelX : labelXs)
            labelX -= overflow;

    return labelXs;
}

void CueTimelineComponent::drawTimeRuler(juce::Graphics& graphics, juce::Rectangle<float> track) const
{
    if (duration <= 0.0)
        return;

    const auto step = rulerStepFor(duration);
    graphics.setColour(juce::Colour(0xff9ca8ba));
    graphics.setFont(12.0f);
    for (double time = 0.0; time <= duration + step * 0.001; time += step)
    {
        const auto x = avsampler::timeline::xAtTime(time, track.getX(), track.getWidth(), duration);
        graphics.drawVerticalLine(static_cast<int>(std::round(x)), track.getBottom() + 3.0f,
                                  track.getBottom() + 8.0f);
        auto label = juce::String(time, time < 10.0 ? 1 : 0) + " s";
        auto labelBounds = juce::Rectangle<float>(static_cast<float>(x - 26.0),
                                                   track.getBottom() + 9.0f, 52.0f, 16.0f);
        if (time <= 0.001)
            labelBounds.setX(track.getX());
        else if (time >= duration - step * 0.001)
            labelBounds.setX(track.getRight() - labelBounds.getWidth());
        graphics.drawFittedText(label, labelBounds.toNearestInt(), juce::Justification::centred, 1);
    }

}

void CueTimelineComponent::paint(juce::Graphics& graphics)
{
    auto bounds = getLocalBounds().toFloat();
    graphics.fillAll(juce::Colour(background));

    graphics.setColour(juce::Colour(0xffd4dbe7));
    graphics.setFont(juce::FontOptions(14.0f).withStyle("Bold"));
    graphics.drawText("TIMELINE", 10, 4, 120, 24, juce::Justification::centredLeft);
    graphics.setColour(juce::Colour(0xff8793a6));
    graphics.setFont(12.0f);
    auto summary = juce::String("4 CUE POINTS");
    if (duration > 0.0)
        summary += "  ·  " + juce::String(duration, 2) + " s";
    graphics.drawText(summary, getWidth() - 190, 4, 180, 24, juce::Justification::centredRight);

    const auto track = trackBounds();
    graphics.setColour(duration > 0.0 ? juce::Colour(trackFill) : juce::Colour(0xff202630));
    graphics.fillRoundedRectangle(track, 7.0f);
    graphics.setColour(juce::Colour(0xff384458));
    graphics.drawRoundedRectangle(track, 7.0f, 1.0f);

    if (duration <= 0.0)
    {
        graphics.setColour(juce::Colour(0xff8b96a8));
        graphics.setFont(13.0f);
        graphics.drawFittedText("Load a video to position Cue Points", track.toNearestInt(),
                                juce::Justification::centred, 1);
    }
    else
    {
        for (int i = 0; i < cueCount; ++i)
        {
            auto segment = track.withX(track.getX() + track.getWidth() * static_cast<float>(i) / cueCount)
                .withWidth(track.getWidth() / cueCount);
            graphics.setColour(cueColours[static_cast<std::size_t>(i)].withAlpha(0.10f));
            graphics.fillRect(segment.reduced(1.0f));
        }

        drawTimeRuler(graphics, track);

        const auto labelCentres = markerLabelCentres(track);
        for (int i = 0; i < cueCount; ++i)
        {
            const auto x = static_cast<float>(avsampler::timeline::xAtTime(
                cuePositions[static_cast<std::size_t>(i)], track.getX(), track.getWidth(), duration));
            const auto colour = cueColours[static_cast<std::size_t>(i)];
            const auto labelCentreX = labelCentres[static_cast<std::size_t>(i)];
            auto marker = juce::Rectangle<float>(labelCentreX - 17.0f, 27.0f, 34.0f, 26.0f);
            graphics.setColour(colour);
            graphics.fillRoundedRectangle(marker, 6.0f);
            graphics.setColour(juce::Colours::black.withAlpha(0.9f));
            graphics.setFont(juce::FontOptions(18.0f).withStyle("Bold"));
            graphics.drawText(juce::String::charToString(static_cast<juce::juce_wchar>('A' + i)),
                              marker.toNearestInt(), juce::Justification::centred);
            graphics.setColour(colour);
            if (std::abs(labelCentreX - x) > 1.0f)
                graphics.drawLine(labelCentreX, marker.getBottom(), x, marker.getBottom(), 1.5f);
            graphics.drawVerticalLine(static_cast<int>(std::round(x)), marker.getBottom(), track.getBottom());
            graphics.fillEllipse(x - 4.0f, track.getCentreY() - 4.0f, 8.0f, 8.0f);
            if (i == selectedCue || i == activeCue)
            {
                graphics.setColour(i == activeCue ? juce::Colours::white : colour.brighter(0.35f));
                graphics.drawRoundedRectangle(marker.expanded(2.0f), 7.0f, i == activeCue ? 2.0f : 1.5f);
            }
        }

        const auto playheadX = static_cast<float>(avsampler::timeline::xAtTime(
            playhead, track.getX(), track.getWidth(), duration));
        graphics.setColour(juce::Colours::white);
        graphics.drawVerticalLine(static_cast<int>(std::round(playheadX)), 26.0f, track.getBottom() + 2.0f);
        juce::Path playheadTriangle;
        playheadTriangle.addTriangle(playheadX - 7.0f, 25.0f, playheadX + 7.0f, 25.0f,
                                     playheadX, 33.0f);
        graphics.fillPath(playheadTriangle);
    }

    for (int i = 0; i < cueCount; ++i)
    {
        auto card = cardBounds(i);
        const auto colour = cueColours[static_cast<std::size_t>(i)];
        graphics.setColour(juce::Colour(0xff1b2636));
        graphics.fillRoundedRectangle(card, 7.0f);
        graphics.setColour(i == selectedCue ? colour : colour.withAlpha(0.6f));
        graphics.drawRoundedRectangle(card, 7.0f, i == selectedCue ? 2.0f : 1.0f);
        graphics.setColour(colour);
        graphics.setFont(juce::FontOptions(16.0f).withStyle("Bold"));
        graphics.drawText(juce::String(avsampler::timeline::midiNoteName(
                              avsampler::defaultCueMidiNotes[static_cast<std::size_t>(i)])),
                          static_cast<int>(card.getX()), static_cast<int>(card.getY() + 3.0f),
                          static_cast<int>(card.getWidth()), 19, juce::Justification::centred);
        graphics.setColour(juce::Colour(0xffc1cad8));
        graphics.setFont(12.0f);
        graphics.drawText(formatTime(cuePositions[static_cast<std::size_t>(i)]),
                          static_cast<int>(card.getX()), static_cast<int>(card.getY() + 22.0f),
                          static_cast<int>(card.getWidth()), 16, juce::Justification::centred);
    }
    juce::ignoreUnused(bounds);
}

int CueTimelineComponent::cueAtPoint(juce::Point<float> point, float maximumDistance) const
{
    const auto track = trackBounds();
    const auto labelCentres = markerLabelCentres(track);
    auto nearestCue = -1;
    auto nearestDistance = maximumDistance;
    for (int i = 0; i < cueCount; ++i)
    {
        const auto x = static_cast<float>(avsampler::timeline::xAtTime(
            cuePositions[static_cast<std::size_t>(i)], track.getX(), track.getWidth(), duration));
        const auto marker = juce::Rectangle<float>(labelCentres[static_cast<std::size_t>(i)] - 17.0f,
                                                    24.0f, 34.0f, 32.0f).expanded(2.0f);
        if (marker.contains(point))
            return i;
        const auto distance = std::abs(point.x - x);
        if (point.y >= 25.0f && point.y <= track.getBottom() && distance <= nearestDistance)
        {
            nearestDistance = distance;
            nearestCue = i;
        }
    }
    return nearestCue;
}

void CueTimelineComponent::setCuePositionFromX(int cueIndex, float x)
{
    if (cueIndex < 0 || cueIndex >= cueCount || duration <= 0.0)
        return;

    const auto track = trackBounds();
    const auto newPosition = avsampler::timeline::timeAtX(x, track.getX(), track.getWidth(), duration);
    cuePositions[static_cast<std::size_t>(cueIndex)] = newPosition;
    if (onCuePositionChanged)
        onCuePositionChanged(cueIndex, newPosition);
    repaint();
}

void CueTimelineComponent::mouseDown(const juce::MouseEvent& event)
{
    const auto point = event.position;
    for (int i = 0; i < cueCount; ++i)
    {
        if (cardBounds(i).contains(point))
        {
            selectCue(i);
            grabKeyboardFocus();
            return;
        }
    }

    if (duration <= 0.0)
        return;

    const auto hitCue = cueAtPoint(point, 20.0f);
    if (hitCue >= 0)
    {
        selectCue(hitCue);
        draggedCue = hitCue;
        grabKeyboardFocus();
    }
}

void CueTimelineComponent::mouseDrag(const juce::MouseEvent& event)
{
    if (draggedCue >= 0)
        setCuePositionFromX(draggedCue, event.position.x);
}

void CueTimelineComponent::mouseUp(const juce::MouseEvent& event)
{
    if (draggedCue >= 0)
        setCuePositionFromX(draggedCue, event.position.x);
    draggedCue = -1;
}

void CueTimelineComponent::mouseMove(const juce::MouseEvent& event)
{
    auto overCard = false;
    for (int i = 0; i < cueCount; ++i)
        overCard = overCard || cardBounds(i).contains(event.position);
    setMouseCursor(overCard || cueAtPoint(event.position, 20.0f) >= 0
        ? juce::MouseCursor::PointingHandCursor : juce::MouseCursor::NormalCursor);
}

bool CueTimelineComponent::keyPressed(const juce::KeyPress& key)
{
    if (duration <= 0.0)
        return false;

    const auto frameCount = key.getModifiers().isShiftDown() ? 10 : 1;
    auto direction = 0;
    if (key.getKeyCode() == juce::KeyPress::leftKey)
        direction = -1;
    else if (key.getKeyCode() == juce::KeyPress::rightKey)
        direction = 1;
    else
        return false;

    const auto index = static_cast<std::size_t>(selectedCue);
    cuePositions[index] = avsampler::timeline::nudgeByFrames(
        cuePositions[index], 30.0, direction * frameCount, duration);
    if (onCuePositionChanged)
        onCuePositionChanged(selectedCue, cuePositions[index]);
    repaint();
    return true;
}
