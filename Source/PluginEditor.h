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

// Cores do tema para menus, combo box e sliders
class RefLookAndFeel : public juce::LookAndFeel_V4
{
public:
    RefLookAndFeel()
    {
        setColour (juce::PopupMenu::backgroundColourId, Theme::panelHi);
        setColour (juce::PopupMenu::textColourId, Theme::text);
        setColour (juce::PopupMenu::highlightedBackgroundColourId, Theme::tealDark);
        setColour (juce::PopupMenu::highlightedTextColourId, Theme::text);
        setColour (juce::PopupMenu::headerTextColourId, Theme::dim);
        setColour (juce::ComboBox::backgroundColourId, Theme::bg);
        setColour (juce::ComboBox::textColourId, Theme::text);
        setColour (juce::ComboBox::outlineColourId, Theme::edge.brighter (0.2f));
        setColour (juce::ComboBox::arrowColourId, Theme::dim);
        setColour (juce::Slider::trackColourId, Theme::tealDark);
        setColour (juce::Slider::backgroundColourId, Theme::bg);
        setColour (juce::Slider::textBoxTextColourId, Theme::text);
        setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
        setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour (juce::TextEditor::backgroundColourId, Theme::bg);
        setColour (juce::TextEditor::textColourId, Theme::text);
        setColour (juce::TextEditor::highlightColourId, Theme::tealDark);
        setColour (juce::TextEditor::outlineColourId, Theme::teal);
        setColour (juce::CaretComponent::caretColourId, Theme::teal);
    }
};

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

// Gráfico principal (curva tonal, alvo All-Purpose e nós do equalizador)
class GraphView : public juce::Component, private juce::Timer
{
public:
    explicit GraphView (RefProProcessor& p) : proc (p), an (p.analyzer) { startTimerHz (30); }
    ~GraphView() override { stopTimer(); }

    std::function<void (int)> onSelect;      // banda selecionada (-1 = nenhuma)
    std::function<void()>     onNodeMoved;

    void setTarget (const TargetData& t) { target = t; repaint(); }
    void setScale (float v) { s = v; }
    void setBars (bool b)   { bars = b; repaint(); }
    void setExtra (bool e)  { extra = e; repaint(); }
    void setSelected (int b) { selected = b; repaint(); }
    juce::Point<float> getNodePosition (int band) const;

    void paint (juce::Graphics&) override;
    void mouseMove (const juce::MouseEvent& e) override { hoverX = e.position.x; }
    void mouseExit (const juce::MouseEvent&) override   { hoverX = -1.0f; }
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseDoubleClick (const juce::MouseEvent&) override;
    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails&) override;

private:
    void timerCallback() override { repaint(); }
    juce::Rectangle<float> plotRect() const;
    float xOf (float f) const;
    float yOf (float db) const;
    float fOf (float x) const;
    float dbOf (float y) const;
    int  hitNode (juce::Point<float> p) const;
    void select (int b);

    RefProProcessor& proc;
    TonalAnalyzer& an;
    TargetData target;
    float s = 1.0f, hoverX = -1.0f;
    bool bars = false, extra = false;
    int selected = -1, dragBand = -1;
};

// Painel de edição da banda selecionada (tipo / freq / ganho / Q)
class BandPanel : public juce::Component
{
public:
    explicit BandPanel (RefProProcessor& p);
    ~BandPanel() override;

    std::function<void()> onDelete;

    void setBand (int b);
    void setScale (float v) { s = v; }
    void refresh();
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    RefProProcessor& proc;
    int band = -1;
    float s = 1.0f;

    FlatButton onBtn { "on" }, delBtn { "del" };
    juce::ComboBox typeBox;
    juce::Slider freq, gain, q;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> aOn;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> aType;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> aFreq, aGain, aQ;
};

// Controles do EQ dinâmico automático
class DynPanel : public juce::Component
{
public:
    explicit DynPanel (RefProProcessor& p);
    ~DynPanel() override;
    void setScale (float v) { s = v; }
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    float s = 1.0f;
    juce::Slider amount, maxCut, tol;
    juce::ComboBox speed;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> aAmount, aMax, aTol;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> aSpeed;
};

// Medidores de análise: Vocal Balance / Dynamics / Stereo Width
class MetersPanel : public juce::Component, private juce::Timer
{
public:
    explicit MetersPanel (TonalAnalyzer& a) : an (a) { startTimerHz (20); }
    ~MetersPanel() override { stopTimer(); }

    void setTarget (const TargetData& t) { target = t; repaint(); }
    void setScale (float v)  { s = v; }
    void setValues (bool v)  { showValues = v; repaint(); }
    void paint (juce::Graphics&) override;

private:
    void timerCallback() override { repaint(); }
    void drawMeter (juce::Graphics&, juce::Rectangle<float> col, int kind, float dev, bool valid, const juce::String& hint) const;

    TonalAnalyzer& an;
    TargetData target;
    float s = 1.0f;
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
    void selectBand (int b);
    void layoutBandPanel();
    void resetEq();

    RefLookAndFeel laf;
    RefProProcessor& proc;
    float s = 1.0f;

    TargetList  list;
    GraphView   graph;
    BandPanel   bandPanel;
    DynPanel    dynPanel;
    MetersPanel meters;
    FlatButton  viewCurve { "curve" }, viewBars { "bars" }, targetPill { "target" },
                captureBtn { "capture" }, settingsBtn { "settings" }, extraBtn { "extra" }, eqBtn { "eq" }, dynBtn { "dyn" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> eqAttach, dynAttach;

    TargetData current;
    int cachedIndex = -1, cachedVersion = -1, selectedBand = -1;
    bool showExtra = false, barsView = false;
    juce::Rectangle<float> statusArea;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RefProEditor)
};
