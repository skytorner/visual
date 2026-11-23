#pragma once

#include <JuceHeader.h>
#include <array>
#include <functional>

class CueTimelineComponent final : public juce::Component
{
public:
    static constexpr int cueCount = 4;

    CueTimelineComponent();
    void paint(juce::Graphics& graphics) override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;
    void mouseMove(const juce::MouseEvent& event) override;
    bool keyPressed(const juce::KeyPress& key) override;

    void setTimelineData(double durationSeconds, double playheadSeconds,
                         const std::array<double, cueCount>& positions);
    void setActiveCue(int cueIndex);
    std::function<void(int, double)> onCuePositionChanged;

private:
    juce::Rectangle<float> trackBounds() const;
    juce::Rectangle<float> cardBounds(int cueIndex) const;
    std::array<float, cueCount> markerLabelCentres(juce::Rectangle<float> track) const;
    int cueAtPoint(juce::Point<float> point, float maximumDistance) const;
    void setCuePositionFromX(int cueIndex, float x);
    void drawTimeRuler(juce::Graphics& graphics, juce::Rectangle<float> track) const;

    std::array<double, cueCount> cuePositions {};
    double duration = 0.0;
    double playhead = 0.0;
    int selectedCue = 0;
    int activeCue = -1;
    int draggedCue = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CueTimelineComponent)
};
