#pragma once

#include "PluginProcessor.h"

namespace Theme
{
    const juce::Colour bg        { 0xff0a0e15 };
    const juce::Colour panel     { 0xff101620 };
    const juce::Colour panelHi   { 0xff161e2b };
    const juce::Colour edge      { 0xff202b3b };
    const juce::Colour teal      { 0xff22c1b8 };
    const juce::Colour tealDark  { 0xff0c4f58 };
    const juce::Colour text      { 0xffe8eef4 };
    const juce::Colour dim       { 0xff8794a6 };
    const juce::Colour purple    { 0xff7a68e0 };
    const juce::Colour orange    { 0xffe0933f };
    const juce::Colour amber     { 0xffefb85a };
    const juce::Colour curve     { 0xfff3fbff };
}

inline juce::Font mkFont (float h, bool bold = false)
{
    return juce::Font (juce::FontOptions (h, bold ? juce::Font::bold : juce::Font::plain));
}

// Botão genérico desenhado por uma função.
class FlatButton : public juce::Button
{
public:
    using Painter = std::function<void (juce::Graphics&, juce::Rectangle<float>, bool over, bool down, bool on)>;
    explicit FlatButton (const juce::String& name) : juce::Button (name) {}
    Painter painter;
    void paintButton (juce::Graphics& g, bool over, bool down) override
    {
        if (painter) painter (g, getLocalBounds().toFloat(), over, down, getToggleState());
    }
};

// Lista de alvos (barra lateral)
class TargetList : public juce::Component
{
public:
    std::function<void (int)> onSelect, onDelete;
    float s = 1.0f;

    void setItems (const juce::StringArray& n, int sel) { names = n; selected = sel; repaint(); }
    void paint (juce::Graphics& g) override;
    void mouseDown (const juce::MouseEvent& e) override;

private:
    juce::StringArray names;
    int selected = 0;
};

// Gráfico principal
class GraphView : public juce::Component, private juce::Timer
{
public:
    explicit GraphView (TonalAnalyzer& a) : an (a) { startTimerHz (30); }
    ~GraphView() override { stopTimer(); }

    void setTarget (const TargetData& t) { target = t; repaint(); }
    void setScale (float v) { s = v; }
    void setBars (bool b)   { bars = b; repaint(); }
    void setExtra (bool e)  { extra = e; repaint(); }

    void paint (juce::Graphics&) override;
    void mouseMove (const juce::MouseEvent& e) override { hoverX = e.position.x; }
    void mouseExit (const juce::MouseEvent&) override   { hoverX = -1.0f; }

private:
    void timerCallback() override { repaint(); }
    TonalAnalyzer& an;
    TargetData target;
    float s = 1.0f, hoverX = -1.0f;
    bool bars = false, extra = false;
};

// Medidores de análise: Vocal Balance / Dynamics / Stereo Width
class MetersPanel : public juce::Component, private juce::Timer
{
public:
    explicit MetersPanel (TonalAnalyzer& a) : an (a) { startTimerHz (20); }
    ~MetersPanel() override { stopTimer(); }

    void setTarget (const TargetData& t)
    {
        target = t;
        targetVocal = Targets::vocalRatioDb (t.centre);
        repaint();
    }
    void setScale (float v)  { s = v; }
    void setValues (bool v)  { showValues = v; repaint(); }
    void paint (juce::Graphics&) override;

private:
    void timerCallback() override { repaint(); }
    void drawMeter (juce::Graphics&, juce::Rectangle<float> col, int kind, float dev, bool valid) const;

    TonalAnalyzer& an;
    TargetData target;
    float targetVocal = -3.0f, s = 1.0f;
    bool showValues = false;
};

class RefProEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit RefProEditor (RefProProcessor&);
    ~RefProEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void syncTarget (bool force);
    void selectTarget (int idx);
    int  currentIndex() const;
    void showTargetMenu();
    void showSettingsMenu();
    void doCapture();

    RefProProcessor& proc;
    float s = 1.0f;

    TargetList  list;
    GraphView   graph;
    MetersPanel meters;
    FlatButton  viewCurve { "curve" }, viewBars { "bars" }, targetPill { "target" },
                captureBtn { "capture" }, settingsBtn { "settings" }, extraBtn { "extra" };

    TargetData current;
    int cachedIndex = -1, cachedVersion = -1;
    bool showExtra = false, barsView = false;
    juce::Rectangle<float> statusArea;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RefProEditor)
};
