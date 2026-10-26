#include "PluginEditor.h"

using juce::Colour;
using juce::Rectangle;

//==============================================================================
//  Ícones
//==============================================================================
namespace Icons
{
    static void mic (juce::Graphics& g, Rectangle<float> r, Colour c, float th)
    {
        g.setColour (c);
        const auto cx = r.getCentreX(), w = r.getWidth() * 0.32f;
        g.drawRoundedRectangle (cx - w * 0.5f, r.getY() + r.getHeight() * 0.05f, w, r.getHeight() * 0.50f, w * 0.5f, th);
        juce::Path stand;
        stand.startNewSubPath (cx - r.getWidth() * 0.30f, r.getY() + r.getHeight() * 0.42f);
        stand.quadraticTo (cx - r.getWidth() * 0.30f, r.getY() + r.getHeight() * 0.80f, cx, r.getY() + r.getHeight() * 0.80f);
        stand.quadraticTo (cx + r.getWidth() * 0.30f, r.getY() + r.getHeight() * 0.80f, cx + r.getWidth() * 0.30f, r.getY() + r.getHeight() * 0.42f);
        g.strokePath (stand, juce::PathStrokeType (th));
        g.drawLine (cx, r.getY() + r.getHeight() * 0.80f, cx, r.getBottom() - 1.0f, th);
    }

    static void gauge (juce::Graphics& g, Rectangle<float> r, Colour c, float th)
    {
        g.setColour (c);
        const auto cx = r.getCentreX(), cy = r.getCentreY() + r.getHeight() * 0.12f;
        juce::Path p;
        p.addCentredArc (cx, cy, r.getWidth() * 0.46f, r.getHeight() * 0.46f, 0.0f, -2.2f, 2.2f, true);
        g.strokePath (p, juce::PathStrokeType (th));
        g.drawLine (cx, cy, cx + r.getWidth() * 0.24f, cy - r.getHeight() * 0.22f, th);
        g.fillEllipse (cx - th, cy - th, th * 2.0f, th * 2.0f);
    }

    static void stereo (juce::Graphics& g, Rectangle<float> r, Colour c, float th)
    {
        g.setColour (c);
        const float d = r.getHeight() * 0.62f;
        g.drawEllipse (r.getCentreX() - d * 0.85f, r.getCentreY() - d * 0.5f, d, d, th);
        g.drawEllipse (r.getCentreX() - d * 0.15f, r.getCentreY() - d * 0.5f, d, d, th);
    }

    static void arrow (juce::Graphics& g, Rectangle<float> r, bool up, Colour c, float th)
    {
        g.setColour (c);
        const float cx = r.getCentreX(), t = r.getY() + 2.0f, b = r.getBottom() - 2.0f;
        g.drawLine (cx, t, cx, b, th);
        const float tip = up ? t : b, dir = up ? 1.0f : -1.0f, h = r.getHeight() * 0.22f;
        g.drawLine (cx, tip, cx - h, tip + dir * h, th);
        g.drawLine (cx, tip, cx + h, tip + dir * h, th);
    }

    static void bars (juce::Graphics& g, Rectangle<float> r, bool dense, Colour c, float th)
    {
        g.setColour (c);
        const int n = 5;
        for (int i = 0; i < n; ++i)
        {
            const float x = r.getX() + r.getWidth() * (0.1f + 0.8f * (float) i / (float) (n - 1));
            const float hgt = dense ? r.getHeight() * 0.50f : r.getHeight() * (i % 2 == 0 ? 0.28f : 0.72f);
            g.drawLine (x, r.getCentreY() - hgt * 0.5f, x, r.getCentreY() + hgt * 0.5f, th);
        }
    }

    static void circles (juce::Graphics& g, Rectangle<float> r, int count, Colour c, float th)
    {
        g.setColour (c);
        const float d = r.getHeight() * 0.50f;
        if (count == 1)
            g.drawEllipse (r.getCentreX() - d * 0.5f, r.getCentreY() - d * 0.5f, d, d, th);
        else
        {
            g.drawEllipse (r.getCentreX() - d - 1.0f, r.getCentreY() - d * 0.5f, d, d, th);
            g.drawEllipse (r.getCentreX() + 1.0f,     r.getCentreY() - d * 0.5f, d, d, th);
        }
    }

    static void gear (juce::Graphics& g, Rectangle<float> r, Colour c, float th)
    {
        g.setColour (c);
        const auto cx = r.getCentreX(), cy = r.getCentreY();
        const float R = r.getHeight() * 0.30f;
        g.drawEllipse (cx - R, cy - R, R * 2.0f, R * 2.0f, th);
        g.drawEllipse (cx - R * 0.38f, cy - R * 0.38f, R * 0.76f, R * 0.76f, th);
        for (int i = 0; i < 8; ++i)
        {
            const float a = juce::MathConstants<float>::twoPi * (float) i / 8.0f;
            g.drawLine (cx + std::cos (a) * R, cy + std::sin (a) * R,
                        cx + std::cos (a) * R * 1.35f, cy + std::sin (a) * R * 1.35f, th * 1.6f);
        }
    }

    static void curveIcon (juce::Graphics& g, Rectangle<float> r, Colour c, float th)
    {
        g.setColour (c);
        juce::Path p;
        p.startNewSubPath (r.getX() + r.getWidth() * 0.12f, r.getY() + r.getHeight() * 0.35f);
        p.cubicTo (r.getX() + r.getWidth() * 0.40f, r.getY() + r.getHeight() * 0.20f,
                   r.getX() + r.getWidth() * 0.55f, r.getY() + r.getHeight() * 0.80f,
                   r.getX() + r.getWidth() * 0.88f, r.getY() + r.getHeight() * 0.70f);
        g.strokePath (p, juce::PathStrokeType (th * 1.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    static void barsIcon (juce::Graphics& g, Rectangle<float> r, Colour c)
    {
        g.setColour (c);
        const float hs[] = { 0.65f, 0.9f, 0.55f, 0.35f };
        for (int i = 0; i < 4; ++i)
        {
            const float w = r.getWidth() * 0.14f;
            const float x = r.getX() + r.getWidth() * (0.16f + 0.19f * (float) i);
            const float h = r.getHeight() * 0.6f * hs[i];
            g.fillRoundedRectangle (x, r.getCentreY() + r.getHeight() * 0.3f - h, w, h, 1.5f);
        }
    }
}

//==============================================================================
//  TargetList
//==============================================================================
void TargetList::paint (juce::Graphics& g)
{
    const float rowH = 40.0f * s;
    for (int i = 0; i < names.size(); ++i)
    {
        auto row = Rectangle<float> (0.0f, (float) i * rowH, (float) getWidth(), rowH);
        if (i == selected)
        {
            g.setGradientFill (juce::ColourGradient (Theme::tealDark, row.getX(), 0.0f,
                                                     Theme::panelHi, row.getRight(), 0.0f, false));
            g.fillRoundedRectangle (row.reduced (2.0f * s, 2.0f * s), 6.0f * s);
        }
        g.setColour (i == selected ? Theme::text : Theme::text.withAlpha (0.82f));
        g.setFont (mkFont (15.0f * s, i == selected));
        g.drawText (names[i], row.withTrimmedLeft (14.0f * s).withTrimmedRight (34.0f * s), juce::Justification::centredLeft);

        // coração (alvo embutido) / ponto (captura)
        const auto ic = Rectangle<float> (row.getRight() - 28.0f * s, row.getCentreY() - 7.0f * s, 14.0f * s, 14.0f * s);
        if (i == 0)
        {
            g.setColour (Theme::teal);
            juce::Path h;
            h.addEllipse (ic.getX(), ic.getY(), ic.getWidth() * 0.55f, ic.getHeight() * 0.55f);
            h.addEllipse (ic.getX() + ic.getWidth() * 0.45f, ic.getY(), ic.getWidth() * 0.55f, ic.getHeight() * 0.55f);
            h.addTriangle (ic.getX() + 0.5f, ic.getY() + ic.getHeight() * 0.38f,
                           ic.getRight() - 0.5f, ic.getY() + ic.getHeight() * 0.38f,
                           ic.getCentreX(), ic.getBottom());
            g.fillPath (h);
        }
        else
        {
            g.setColour (Theme::dim);
            g.drawEllipse (ic.reduced (3.0f * s), 1.3f);
        }
    }
}

void TargetList::mouseDown (const juce::MouseEvent& e)
{
    const int idx = (int) (e.position.y / (40.0f * s));
    if (idx < 0 || idx >= names.size()) return;

    if (e.mods.isPopupMenu())
    {
        if (idx == 0) return;
        juce::PopupMenu m;
        m.addItem (1, "Delete " + names[idx]);
        m.showMenuAsync (juce::PopupMenu::Options(), [this, idx] (int r) { if (r == 1 && onDelete) onDelete (idx); });
    }
    else if (onSelect)
        onSelect (idx);
}

//==============================================================================
//  Helpers de parâmetros (com gestos para automação do host)
//==============================================================================
static void setParam (juce::AudioProcessorValueTreeState& a, const juce::String& id, float plain)
{
    if (auto* p = a.getParameter (id)) p->setValueNotifyingHost (p->convertTo0to1 (plain));
}
static void gesture (juce::AudioProcessorValueTreeState& a, const juce::String& id, bool begin)
{
    if (auto* p = a.getParameter (id)) { if (begin) p->beginChangeGesture(); else p->endChangeGesture(); }
}
static void setParamG (juce::AudioProcessorValueTreeState& a, const juce::String& id, float plain)
{
    gesture (a, id, true); setParam (a, id, plain); gesture (a, id, false);
}
static void setBandType (juce::AudioProcessorValueTreeState& a, int band, int type)
{
    setParamG (a, RefProProcessor::bandId (band, "type"), (float) type);
    if (type == Eq::LowCut || type == Eq::HighCut) setParamG (a, RefProProcessor::bandId (band, "q"), 0.71f);
    if (! Eq::typeHasGain (type))                  setParamG (a, RefProProcessor::bandId (band, "gain"), 0.0f);
}
static void removeBand (juce::AudioProcessorValueTreeState& a, int band)
{
    setParamG (a, RefProProcessor::bandId (band, "on"), 0.0f);
}

//==============================================================================
//  GraphView
//==============================================================================
static juce::String fmtFreq (float f)
{
    return f >= 1000.0f ? juce::String (f / 1000.0f, 2) + " kHz" : juce::String ((int) std::round (f)) + " Hz";
}


juce::Rectangle<float> GraphView::plotRect() const
{
    return getLocalBounds().toFloat().withTrimmedTop (48.0f * s).withTrimmedBottom (26.0f * s).reduced (1.0f, 0.0f);
}
float GraphView::xOf (float f) const
{
    const auto r = plotRect();
    return r.getX() + r.getWidth() * juce::jlimit (0.0f, 1.02f, rp::freqToX01 (f));
}
float GraphView::yOf (float db) const
{
    const auto r = plotRect();
    return r.getY() + r.getHeight() * (rp::dbTop - db) / (rp::dbTop - rp::dbBottom);
}
float GraphView::fOf (float x) const
{
    const auto r = plotRect();
    return rp::fMin * std::pow (10.0f, 3.0f * juce::jlimit (0.0f, 1.0f, (x - r.getX()) / r.getWidth()));
}
float GraphView::dbOf (float y) const
{
    const auto r = plotRect();
    return rp::dbTop - (rp::dbTop - rp::dbBottom) * (y - r.getY()) / r.getHeight();
}

juce::Point<float> GraphView::getNodePosition (int band) const
{
    const auto prm = proc.getBandParams (band);
    const auto r = plotRect();
    const float y = Eq::typeHasGain (prm.type) ? yOf (prm.gain) : yOf (0.0f);
    return { xOf (prm.freq), juce::jlimit (r.getY() + 4.0f, r.getBottom() - 4.0f, y) };
}

int GraphView::hitNode (juce::Point<float> p) const
{
    int best = -1; float bd = 14.0f * s;
    for (int b = 0; b < Eq::numBands; ++b)
    {
        if (! proc.getBandParams (b).on) continue;
        const float d = p.getDistanceFrom (getNodePosition (b));
        if (d < bd) { bd = d; best = b; }
    }
    return best;
}

void GraphView::select (int b)
{
    selected = b;
    if (onSelect) onSelect (b);
    repaint();
}

void GraphView::mouseDown (const juce::MouseEvent& e)
{
    const int n = hitNode (e.position);

    if (e.mods.isPopupMenu())
    {
        if (n < 0) return;
        select (n);
        const auto prm = proc.getBandParams (n);
        const auto names = Eq::typeNames();
        juce::PopupMenu m;
        for (int i = 0; i < names.size(); ++i) m.addItem (i + 1, names[i], true, i == prm.type);
        m.addSeparator();
        m.addItem (100, "Delete band");
        m.showMenuAsync (juce::PopupMenu::Options(), [this, n] (int r)
        {
            if (r >= 1 && r <= Eq::numTypes) setBandType (proc.apvts, n, r - 1);
            else if (r == 100) { removeBand (proc.apvts, n); select (-1); }
            if (onNodeMoved) onNodeMoved();
        });
        return;
    }

    if (n >= 0)
    {
        select (n);
        dragBand = n;
        gesture (proc.apvts, RefProProcessor::bandId (n, "freq"), true);
        gesture (proc.apvts, RefProProcessor::bandId (n, "gain"), true);
    }
    else
        select (-1);
}

void GraphView::mouseDrag (const juce::MouseEvent& e)
{
    if (dragBand < 0) return;
    const auto prm = proc.getBandParams (dragBand);
    setParam (proc.apvts, RefProProcessor::bandId (dragBand, "freq"), juce::jlimit (20.0f, 20000.0f, fOf (e.position.x)));
    if (Eq::typeHasGain (prm.type))
        setParam (proc.apvts, RefProProcessor::bandId (dragBand, "gain"), juce::jlimit (-18.0f, 18.0f, dbOf (e.position.y)));
    if (onNodeMoved) onNodeMoved();
}

void GraphView::mouseUp (const juce::MouseEvent&)
{
    if (dragBand < 0) return;
    gesture (proc.apvts, RefProProcessor::bandId (dragBand, "freq"), false);
    gesture (proc.apvts, RefProProcessor::bandId (dragBand, "gain"), false);
    dragBand = -1;
}

void GraphView::mouseDoubleClick (const juce::MouseEvent& e)
{
    const int n = hitNode (e.position);
    if (n >= 0) { removeBand (proc.apvts, n); select (-1); return; }
    if (! plotRect().contains (e.position)) return;

    for (int b = 0; b < Eq::numBands; ++b)
        if (! proc.getBandParams (b).on)
        {
            setParamG (proc.apvts, RefProProcessor::bandId (b, "type"), (float) Eq::Bell);
            setParamG (proc.apvts, RefProProcessor::bandId (b, "freq"), fOf (e.position.x));
            setParamG (proc.apvts, RefProProcessor::bandId (b, "gain"), juce::jlimit (-18.0f, 18.0f, dbOf (e.position.y)));
            setParamG (proc.apvts, RefProProcessor::bandId (b, "q"), 1.0f);
            setParamG (proc.apvts, RefProProcessor::bandId (b, "on"), 1.0f);
            select (b);
            return;
        }
}

void GraphView::mouseWheelMove (const juce::MouseEvent& e, const juce::MouseWheelDetails& w)
{
    const int n = hitNode (e.position);
    if (n < 0) return;
    const auto prm = proc.getBandParams (n);
    setParamG (proc.apvts, RefProProcessor::bandId (n, "q"), juce::jlimit (0.1f, 10.0f, prm.q * std::exp (w.deltaY * 0.6f)));
    if (onNodeMoved) onNodeMoved();
}

void GraphView::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    g.setColour (Theme::panel);
    g.fillRoundedRectangle (b, 10.0f * s);
    g.setColour (Theme::edge);
    g.drawRoundedRectangle (b.reduced (0.5f), 10.0f * s, 1.0f);

    const auto plot = b.withTrimmedTop (48.0f * s).withTrimmedBottom (26.0f * s).reduced (1.0f, 0.0f);
    auto X = [&] (float f) { return plot.getX() + plot.getWidth() * juce::jlimit (0.0f, 1.02f, rp::freqToX01 (f)); };
    auto Y = [&] (float db) { return plot.getY() + plot.getHeight() * (rp::dbTop - db) / (rp::dbTop - rp::dbBottom); };

    // ---- grade ----
    g.setFont (mkFont (12.0f * s));
    for (int db = 8; db >= -12; db -= 4)
    {
        const float y = Y ((float) db);
        g.setColour (db == 0 ? Theme::teal.withAlpha (0.35f) : Theme::edge.withAlpha (0.55f));
        g.drawHorizontalLine ((int) y, plot.getX(), plot.getRight());
        g.setColour (Theme::dim);
        g.drawText (juce::String (db), Rectangle<float> (plot.getRight() - 46.0f * s, y - 8.0f * s, 38.0f * s, 16.0f * s),
                    juce::Justification::centredRight);
    }
    g.setColour (Theme::dim);
    g.drawText ("dB", Rectangle<float> (plot.getRight() - 46.0f * s, b.getY() + 8.0f * s, 38.0f * s, 16.0f * s), juce::Justification::centredRight);

    struct FL { float f; const char* t; };
    static const FL fl[] = { {20,"20"},{50,"50"},{100,"100"},{200,"200"},{500,"500"},{1000,"1k"},{2000,"2k"},{5000,"5k"},{10000,"10k"} };
    for (auto& x : fl)
    {
        g.setColour (Theme::edge.withAlpha (0.35f));
        g.drawVerticalLine ((int) X (x.f), plot.getY(), plot.getBottom());
        g.setColour (Theme::dim);
        const float cx = juce::jlimit (b.getX() + 14.0f * s, b.getRight() - 40.0f * s, X (x.f));
        g.drawText (x.t, Rectangle<float> (cx - 20.0f * s, plot.getBottom() + 4.0f * s, 40.0f * s, 16.0f * s), juce::Justification::centred);
    }
    g.drawText ("Hz", Rectangle<float> (plot.getRight() - 40.0f * s, plot.getBottom() + 4.0f * s, 34.0f * s, 16.0f * s), juce::Justification::centredRight);

    // ---- regiões ----
    static const float xo[] = { 200.0f, 1800.0f, 7000.0f };
    static const float lim[] = { 20.0f, 200.0f, 1800.0f, 7000.0f, 24000.0f };
    static const char* names[] = { "Low", "Low-Mid", "High-Mid", "High" };
    for (float f : xo)
    {
        g.setColour (Theme::edge.brighter (0.25f));
        g.drawVerticalLine ((int) X (f), b.getY() + 6.0f * s, plot.getBottom());
    }

    const bool has = an.hasData.load();
    for (int r = 0; r < 4; ++r)
    {
        const float x0 = X (lim[r]), x1 = std::min (X (lim[r + 1]), plot.getRight());
        g.setColour (Theme::text.withAlpha (0.9f));
        g.setFont (mkFont (13.0f * s));
        g.drawText (names[r], Rectangle<float> (x0, b.getY() + 8.0f * s, x1 - x0, 18.0f * s), juce::Justification::centred);

        if (has)
        {
            float sum = 0.0f; int cnt = 0;
            for (int i = 0; i < rp::numBands; ++i)
            {
                const float f = rp::bandFreq (i);
                if (f >= lim[r] && f < lim[r + 1])
                { sum += an.avgDb[(size_t) i].load() - target.centre[(size_t) i]; ++cnt; }
            }
            if (cnt > 0)
            {
                const float dev = sum / (float) cnt;
                g.setColour (std::abs (dev) <= 1.5f ? Theme::teal : Theme::amber);
                g.setFont (mkFont (12.0f * s));
                g.drawText ((dev >= 0 ? "+" : "") + juce::String (dev, 1) + " dB",
                            Rectangle<float> (x0, b.getY() + 26.0f * s, x1 - x0, 16.0f * s), juce::Justification::centred);
            }
        }
    }

    // ---- curvas (recortadas à área de plotagem) ----
    {
        juce::Graphics::ScopedSaveState ss (g);
        g.reduceClipRegion (plot.toNearestInt());

        // faixa-alvo
        juce::Path band, mid;
        for (int i = 0; i < rp::numBands; ++i)
        {
            const float x = X (rp::bandFreq (i));
            const float y = Y (target.centre[(size_t) i] + target.tol[(size_t) i]);
            i == 0 ? band.startNewSubPath (x, y) : band.lineTo (x, y);
        }
        for (int i = rp::numBands - 1; i >= 0; --i)
            band.lineTo (X (rp::bandFreq (i)), Y (target.centre[(size_t) i] - target.tol[(size_t) i]));
        band.closeSubPath();
        for (int i = 0; i < rp::numBands; ++i)
        {
            const float x = X (rp::bandFreq (i)), y = Y (target.centre[(size_t) i]);
            i == 0 ? mid.startNewSubPath (x, y) : mid.lineTo (x, y);
        }
        g.setGradientFill (juce::ColourGradient (Theme::teal.withAlpha (0.46f), 0.0f, plot.getY(),
                                                 Theme::teal.withAlpha (0.14f), 0.0f, plot.getBottom(), false));
        g.fillPath (band);
        g.setColour (Theme::teal.withAlpha (0.55f));
        g.strokePath (band, juce::PathStrokeType (1.0f * s));
        g.setColour (Theme::teal.withAlpha (0.45f));
        g.strokePath (mid, juce::PathStrokeType (1.2f * s));

        if (has)
        {
            if (bars)
            {
                const float y0 = Y (0.0f);
                for (int i = 0; i + 2 < rp::numBands; i += 3)
                {
                    const float v = an.avgDb[(size_t) (i + 1)].load();
                    const float c = target.centre[(size_t) (i + 1)], t = target.tol[(size_t) (i + 1)];
                    const float xa = X (rp::bandFreq (i) * 0.97f), xb = X (rp::bandFreq (i + 2) * 1.03f);
                    g.setColour ((std::abs (v - c) <= t ? Theme::teal : Theme::amber).withAlpha (0.75f));
                    g.fillRect (Rectangle<float> (xa + 0.5f, std::min (y0, Y (v)), xb - xa - 1.0f, std::abs (Y (v) - y0)));
                }
            }
            else
            {
                if (extra)
                {
                    juce::Path f;
                    for (int i = 0; i < rp::numBands; ++i)
                    {
                        const float x = X (rp::bandFreq (i)), y = Y (an.fastDb[(size_t) i].load());
                        i == 0 ? f.startNewSubPath (x, y) : f.lineTo (x, y);
                    }
                    g.setColour (Theme::amber.withAlpha (0.55f));
                    g.strokePath (f, juce::PathStrokeType (1.2f * s));
                }

                juce::Path p;
                for (int i = 0; i < rp::numBands; ++i)
                {
                    const float x = X (rp::bandFreq (i)), y = Y (an.avgDb[(size_t) i].load());
                    i == 0 ? p.startNewSubPath (x, y) : p.lineTo (x, y);
                }
                g.setColour (Theme::curve.withAlpha (0.14f));
                g.strokePath (p, juce::PathStrokeType (6.0f * s, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
                g.setColour (Theme::curve);
                g.strokePath (p, juce::PathStrokeType (2.2f * s, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            }
        }

        // ---- curva do equalizador ----
        {
            const bool eqOn = proc.apvts.getRawParameterValue ("eqOn")->load() > 0.5f;
            const double sr = proc.getSampleRate() > 0.0 ? proc.getSampleRate() : 48000.0;
            std::array<Eq::Coeffs, Eq::numBands> cf {};
            bool any = false;
            for (int k = 0; k < Eq::numBands; ++k)
            {
                const auto prm = proc.getBandParams (k);
                if (Eq::isActive (prm)) { cf[(size_t) k] = Eq::make (prm, sr); any = true; }
                else cf[(size_t) k] = Eq::Coeffs {};
            }
            if (any)
            {
                juce::Path c, fill;
                const int pts = 240;
                for (int i = 0; i < pts; ++i)
                {
                    const float f = rp::fMin * std::pow (1000.0f, (float) i / (float) (pts - 1));
                    double db = 0.0;
                    for (int k = 0; k < Eq::numBands; ++k)
                        if (Eq::isActive (proc.getBandParams (k))) db += Eq::magnitudeDb (cf[(size_t) k], f, sr);
                    const float x = X (f), y = Y ((float) db);
                    i == 0 ? c.startNewSubPath (x, y) : c.lineTo (x, y);
                }
                fill = c;
                fill.lineTo (X (20000.0f), Y (0.0f));
                fill.lineTo (X (20.0f), Y (0.0f));
                fill.closeSubPath();
                const auto col = eqOn ? Theme::amber : Theme::dim;
                g.setColour (col.withAlpha (0.10f));
                g.fillPath (fill);
                g.setColour (col.withAlpha (eqOn ? 0.95f : 0.6f));
                g.strokePath (c, juce::PathStrokeType (1.7f * s, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            }
        }
    }

    {
        juce::Graphics::ScopedSaveState ss2 (g);
        g.reduceClipRegion (plot.toNearestInt());

        // ---- EQ dinâmico: curva do corte atual + barras por banda ----
        if (proc.apvts.getRawParameterValue ("dynOn")->load() > 0.5f)
        {
            const double sr = proc.getSampleRate() > 0.0 ? proc.getSampleRate() : 48000.0;
            std::array<Eq::Coeffs, RefProProcessor::dynBands> dc {};
            std::array<float, RefProProcessor::dynBands> gdb {};
            bool any = false;
            for (int j = 0; j < RefProProcessor::dynBands; ++j)
            {
                gdb[(size_t) j] = proc.dynGainDb[(size_t) j].load();
                if (gdb[(size_t) j] < -0.05f)
                {
                    Eq::Params pp; pp.on = true; pp.type = Eq::Bell; pp.freq = RefProProcessor::dynFreq (j); pp.gain = gdb[(size_t) j]; pp.q = 4.3f;
                    dc[(size_t) j] = Eq::make (pp, sr);
                    any = true;
                }
            }
            // faixa inferior com os cortes (0 dB no topo, -12 dB no fundo)
            const float stripH = 44.0f * s;
            auto strip = juce::Rectangle<float> (plot.getX(), plot.getBottom() - stripH, plot.getWidth(), stripH);
            g.setColour (Theme::bg.withAlpha (0.55f));
            g.fillRect (strip);
            g.setColour (Theme::edge.brighter (0.2f));
            g.drawHorizontalLine ((int) strip.getY(), strip.getX(), strip.getRight());
            g.setColour (Theme::dim);
            g.setFont (mkFont (11.0f * s));
            g.drawText ("Dynamic cut (dB)", juce::Rectangle<float> (strip.getX() + 6.0f * s, strip.getY() + 2.0f * s, 120.0f * s, 14.0f * s), juce::Justification::centredLeft);
            float worst = 0.0f;
            for (int j = 0; j < RefProProcessor::dynBands; ++j)
            {
                const float fc = RefProProcessor::dynFreq (j), gn = gdb[(size_t) j];
                worst = std::min (worst, gn);
                if (gn > -0.05f) continue;
                const float xa = X (fc * 0.8909f), xb = X (fc * 1.1225f);
                const float h = juce::jlimit (0.0f, 1.0f, -gn / 12.0f) * (stripH - 4.0f * s);
                g.setColour (Theme::orange.withAlpha (0.85f));
                g.fillRect (juce::Rectangle<float> (xa + 0.5f, strip.getY() + 2.0f * s, std::max (1.0f, xb - xa - 1.0f), h));
            }
            if (worst < -0.05f)
            {
                g.setColour (Theme::orange);
                g.drawText (juce::String (worst, 1) + " dB", juce::Rectangle<float> (strip.getRight() - 150.0f * s, strip.getY() + 2.0f * s, 140.0f * s, 14.0f * s), juce::Justification::centredRight);
            }
            if (any)
            {
                juce::Path c;
                const int pts = 240;
                for (int i = 0; i < pts; ++i)
                {
                    const float f = rp::fMin * std::pow (1000.0f, (float) i / (float) (pts - 1));
                    double db = 0.0;
                    for (int j = 0; j < RefProProcessor::dynBands; ++j)
                        if (gdb[(size_t) j] < -0.05f) db += Eq::magnitudeDb (dc[(size_t) j], f, sr);
                    const float x = X (f), y = Y ((float) db);
                    i == 0 ? c.startNewSubPath (x, y) : c.lineTo (x, y);
                }
                g.setColour (Theme::orange.withAlpha (0.9f));
                g.strokePath (c, juce::PathStrokeType (1.5f * s, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            }
        }
    }

    // ---- nós do equalizador ----
    {
        const bool eqOn = proc.apvts.getRawParameterValue ("eqOn")->load() > 0.5f;
        for (int k = 0; k < Eq::numBands; ++k)
        {
            if (! proc.getBandParams (k).on) continue;
            const auto pos = getNodePosition (k);
            const bool sel = (k == selected);
            const float r = (sel ? 7.5f : 6.0f) * s;
            if (sel)
            {
                g.setColour (Theme::teal.withAlpha (0.25f));
                g.fillEllipse (pos.x - r * 1.9f, pos.y - r * 1.9f, r * 3.8f, r * 3.8f);
            }
            g.setColour (Theme::bg);
            g.fillEllipse (pos.x - r - 1.5f, pos.y - r - 1.5f, (r + 1.5f) * 2.0f, (r + 1.5f) * 2.0f);
            g.setColour (! eqOn ? Theme::dim : (sel ? Theme::text : Theme::teal));
            g.fillEllipse (pos.x - r, pos.y - r, r * 2.0f, r * 2.0f);
            g.setColour (Theme::bg);
            g.setFont (mkFont (9.0f * s, true));
            g.drawText (juce::String (k + 1), juce::Rectangle<float> (pos.x - r, pos.y - r, r * 2.0f, r * 2.0f), juce::Justification::centred);
        }
    }

    if (! has)
    {
        g.setColour (Theme::dim);
        g.setFont (mkFont (15.0f * s));
        g.drawText ("Play audio to see your tonal balance", plot.reduced (20.0f * s), juce::Justification::centred);
    }

    // ---- hover ----
    if (hoverX >= plot.getX() && hoverX <= plot.getRight() && has && dragBand < 0)
    {
        const float f = rp::fMin * std::pow (10.0f, 3.0f * (hoverX - plot.getX()) / plot.getWidth());
        const int idx = juce::jlimit (0, rp::numBands - 1, (int) std::round (12.0f * std::log2 (f / rp::fMin)));
        g.setColour (Theme::text.withAlpha (0.35f));
        g.drawVerticalLine ((int) hoverX, plot.getY(), plot.getBottom());

        const float me = an.avgDb[(size_t) idx].load(), tg = target.centre[(size_t) idx];
        const juce::String txt = fmtFreq (f) + "   You " + juce::String (me, 1) + "   Target " + juce::String (tg, 1) + " dB";
        const float w = 250.0f * s;
        auto box = Rectangle<float> (juce::jlimit (plot.getX() + 4.0f, plot.getRight() - w - 4.0f, hoverX + 10.0f * s),
                                     plot.getBottom() - 30.0f * s, w, 22.0f * s);
        g.setColour (Theme::bg.withAlpha (0.88f));
        g.fillRoundedRectangle (box, 5.0f * s);
        g.setColour (Theme::text);
        g.setFont (mkFont (12.5f * s));
        g.drawText (txt, box, juce::Justification::centred);
    }
}

//==============================================================================
//  MetersPanel  (kind: 0 = Vocal Balance, 1 = Dynamics, 2 = Stereo Width)
//==============================================================================
void MetersPanel::drawMeter (juce::Graphics& g, Rectangle<float> col, int kind, float dev, bool valid, const juce::String& hint) const
{
    const Colour accent = kind == 0 ? Theme::purple : kind == 1 ? Theme::orange : Theme::teal;
    const char* label   = kind == 0 ? "Vocal Balance" : kind == 1 ? "Dynamics" : "Stereo Width";
    const float th = 1.5f * s;

    // título + ícone
    g.setFont (mkFont (15.0f * s));
    juce::GlyphArrangement ga;
    ga.addLineOfText (mkFont (15.0f * s), label, 0.0f, 0.0f);
    const float tw = ga.getBoundingBox (0, -1, true).getWidth();
    const float cx = col.getCentreX(), ty = col.getY() + 14.0f * s;
    auto iconR = Rectangle<float> (cx - tw * 0.5f - 30.0f * s, ty, 24.0f * s, 24.0f * s);
    if (kind == 0) Icons::mic (g, iconR, Theme::dim, th);
    if (kind == 1) Icons::gauge (g, iconR, Theme::dim, th);
    if (kind == 2) Icons::stereo (g, iconR, Theme::dim, th);
    g.setColour (Theme::text);
    g.drawText (label, Rectangle<float> (cx - tw * 0.5f - 2.0f * s, ty, tw + 8.0f * s, 24.0f * s), juce::Justification::centredLeft);

    // trilho
    const float endW = 26.0f * s;
    auto track = Rectangle<float> (col.getX() + 22.0f * s + endW, col.getY() + 62.0f * s,
                                   col.getWidth() - 44.0f * s - endW * 2.0f, 16.0f * s);
    auto left  = Rectangle<float> (col.getX() + 18.0f * s, track.getCentreY() - 12.0f * s, endW, 24.0f * s);
    auto right = Rectangle<float> (track.getRight() + 6.0f * s, track.getCentreY() - 12.0f * s, endW, 24.0f * s);
    if (kind == 0) { Icons::arrow (g, left, false, Theme::dim, th);  Icons::arrow (g, right, true, Theme::dim, th); }
    if (kind == 1) { Icons::bars (g, left, true, Theme::dim, th);    Icons::bars (g, right, false, Theme::dim, th); }
    if (kind == 2) { Icons::circles (g, left, 1, Theme::dim, th);    Icons::circles (g, right, 2, Theme::dim, th); }

    g.setColour (Theme::bg);
    g.fillRoundedRectangle (track, track.getHeight() * 0.5f);
    g.setColour (Theme::edge);
    g.drawRoundedRectangle (track, track.getHeight() * 0.5f, 1.0f);

    const float zoneHalf = (1.5f / 6.0f) * track.getWidth() * 0.5f;
    auto zone = Rectangle<float> (track.getCentreX() - zoneHalf, track.getY() + 2.0f, zoneHalf * 2.0f, track.getHeight() - 4.0f);
    g.setGradientFill (juce::ColourGradient (accent.withAlpha (0.15f), zone.getX(), 0.0f,
                                             accent.withAlpha (0.15f), zone.getRight(), 0.0f, false));
    juce::ColourGradient gr (accent.withAlpha (0.10f), zone.getX(), 0.0f, accent.withAlpha (0.10f), zone.getRight(), 0.0f, false);
    gr.addColour (0.5, accent.withAlpha (0.75f));
    g.setGradientFill (gr);
    g.fillRoundedRectangle (zone, 3.0f * s);
    g.setColour (accent.withAlpha (0.9f));
    g.drawVerticalLine ((int) zone.getX(), zone.getY(), zone.getBottom());
    g.drawVerticalLine ((int) zone.getRight(), zone.getY(), zone.getBottom());

    if (valid)
    {
        const float pos = juce::jlimit (-1.0f, 1.0f, dev / 6.0f);
        const float x = track.getCentreX() + pos * (track.getWidth() * 0.5f - 8.0f * s);
        const float d = 18.0f * s;
        g.setColour (Colour (0x66000000));
        g.fillEllipse (x - d * 0.5f, track.getCentreY() - d * 0.5f + 1.5f, d, d);
        g.setColour (std::abs (dev) <= 1.5f ? Theme::text : Theme::amber);
        g.fillEllipse (x - d * 0.5f, track.getCentreY() - d * 0.5f, d, d);
        g.setColour (Theme::bg);
        g.drawEllipse (x - d * 0.5f, track.getCentreY() - d * 0.5f, d, d, 1.5f);

        if (showValues)
        {
            g.setColour (Theme::dim);
            g.setFont (mkFont (12.5f * s));
            g.drawText ((dev >= 0 ? "+" : "") + juce::String (dev, 1) + " dB vs target",
                        Rectangle<float> (col.getX(), track.getBottom() + 12.0f * s, col.getWidth(), 18.0f * s),
                        juce::Justification::centred);
        }
    }
    else if (hint.isNotEmpty())
    {
        g.setColour (Theme::dim);
        g.setFont (mkFont (12.5f * s));
        g.drawText (hint, Rectangle<float> (col.getX(), track.getBottom() + 12.0f * s, col.getWidth(), 18.0f * s),
                    juce::Justification::centred);
    }
}

void MetersPanel::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    g.setColour (Theme::panel);
    g.fillRoundedRectangle (b, 10.0f * s);
    g.setColour (Theme::edge);
    g.drawRoundedRectangle (b.reduced (0.5f), 10.0f * s, 1.0f);

    const bool valid = an.hasData.load();
    const bool vocalSeen = an.vocalConf.load() >= 0.12f;
    const float devVocal = an.vocalDb.load() - target.vocalDb;
    const float devDyn   = an.avgPsr.load() - target.psr;
    const float devWid   = an.avgWidthDb.load() - target.widthDb;

    const float w = b.getWidth() / 3.0f;
    drawMeter (g, Rectangle<float> (b.getX(),            b.getY(), w, b.getHeight()), 0, devVocal, valid && vocalSeen,
               valid && ! vocalSeen ? "No vocal detected" : "");
    drawMeter (g, Rectangle<float> (b.getX() + w,        b.getY(), w, b.getHeight()), 1, devDyn,   valid, "");
    drawMeter (g, Rectangle<float> (b.getX() + w * 2.0f, b.getY(), w, b.getHeight()), 2, devWid,   valid, "");
}

//==============================================================================
//  BandPanel
//==============================================================================
BandPanel::BandPanel (RefProProcessor& p) : proc (p)
{
    addAndMakeVisible (onBtn);
    addAndMakeVisible (delBtn);
    addAndMakeVisible (typeBox);
    typeBox.addItemList (Eq::typeNames(), 1);
    typeBox.setJustificationType (juce::Justification::centredLeft);

    for (auto* sl : { &freq, &gain, &q })
    {
        addAndMakeVisible (*sl);
        sl->setSliderStyle (juce::Slider::LinearBar);
        sl->setTextBoxStyle (juce::Slider::TextBoxLeft, false, 80, 20);
        sl->setTextBoxIsEditable (true);
        sl->setColour (juce::Slider::trackColourId, Theme::tealDark.withAlpha (0.55f));
    }

    onBtn.setClickingTogglesState (true);
    onBtn.painter = [this] (juce::Graphics& g, Rectangle<float> r, bool over, bool, bool on)
    {
        const auto c = r.withSizeKeepingCentre (18.0f * s, 18.0f * s);
        g.setColour (on ? Theme::teal : Theme::dim);
        juce::Path arc;
        arc.addCentredArc (c.getCentreX(), c.getCentreY() + 1.0f, c.getWidth() * 0.42f, c.getHeight() * 0.42f, 0.0f, 0.55f, juce::MathConstants<float>::twoPi - 0.55f, true);
        g.strokePath (arc, juce::PathStrokeType (1.8f * s, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.drawLine (c.getCentreX(), c.getY(), c.getCentreX(), c.getCentreY() - 1.0f, 1.8f * s);
        if (over) { g.setColour (Theme::text.withAlpha (0.1f)); g.fillEllipse (r.reduced (2.0f)); }
    };
    delBtn.painter = [this] (juce::Graphics& g, Rectangle<float> r, bool over, bool, bool)
    {
        const auto c = r.withSizeKeepingCentre (12.0f * s, 12.0f * s);
        g.setColour (over ? Theme::text : Theme::dim);
        g.drawLine (c.getX(), c.getY(), c.getRight(), c.getBottom(), 1.6f * s);
        g.drawLine (c.getX(), c.getBottom(), c.getRight(), c.getY(), 1.6f * s);
    };
    delBtn.onClick = [this] { if (onDelete) onDelete(); };
    setVisible (false);
}

BandPanel::~BandPanel()
{
    aOn.reset(); aType.reset(); aFreq.reset(); aGain.reset(); aQ.reset();
}

void BandPanel::setBand (int b)
{
    aOn.reset(); aType.reset(); aFreq.reset(); aGain.reset(); aQ.reset();
    band = b;
    if (b < 0) { setVisible (false); return; }

    auto& a = proc.apvts;
    aOn   = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>   (a, RefProProcessor::bandId (b, "on"),   onBtn);
    aType = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (a, RefProProcessor::bandId (b, "type"), typeBox);
    aFreq = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>   (a, RefProProcessor::bandId (b, "freq"), freq);
    aGain = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>   (a, RefProProcessor::bandId (b, "gain"), gain);
    aQ    = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>   (a, RefProProcessor::bandId (b, "q"),    q);
    setVisible (true);
    refresh();
}

void BandPanel::refresh()
{
    if (band < 0) return;
    const bool hasGain = Eq::typeHasGain (proc.getBandParams (band).type);
    if (gain.isEnabled() != hasGain) { gain.setEnabled (hasGain); gain.setAlpha (hasGain ? 1.0f : 0.35f); }
}

void BandPanel::resized()
{
    onBtn.setBounds   (juce::Rectangle<float> (6.0f * s, 24.0f * s, 30.0f * s, 30.0f * s).toNearestInt());
    typeBox.setBounds (juce::Rectangle<float> (42.0f * s, 24.0f * s, 112.0f * s, 30.0f * s).toNearestInt());
    freq.setBounds    (juce::Rectangle<float> (162.0f * s, 24.0f * s, 104.0f * s, 30.0f * s).toNearestInt());
    gain.setBounds    (juce::Rectangle<float> (272.0f * s, 24.0f * s, 96.0f * s, 30.0f * s).toNearestInt());
    q.setBounds       (juce::Rectangle<float> (374.0f * s, 24.0f * s, 52.0f * s, 30.0f * s).toNearestInt());
    delBtn.setBounds  (juce::Rectangle<float> (getWidth() - 28.0f * s, 4.0f * s, 24.0f * s, 20.0f * s).toNearestInt());
}

void BandPanel::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    g.setColour (Theme::bg.withAlpha (0.94f));
    g.fillRoundedRectangle (b, 8.0f * s);
    g.setColour (Theme::teal.withAlpha (0.55f));
    g.drawRoundedRectangle (b.reduced (0.5f), 8.0f * s, 1.0f);

    g.setColour (Theme::dim);
    g.setFont (mkFont (11.5f * s));
    g.drawText ("Band " + juce::String (band + 1), Rectangle<float> (10.0f * s, 4.0f * s, 80.0f * s, 16.0f * s), juce::Justification::centredLeft);
    const char* labels[] = { "Freq", "Gain", "Q" };
    Rectangle<int> rs[] = { freq.getBounds(), gain.getBounds(), q.getBounds() };
    for (int i = 0; i < 3; ++i)
        g.drawText (labels[i], Rectangle<float> ((float) rs[i].getX(), 4.0f * s, (float) rs[i].getWidth(), 16.0f * s), juce::Justification::centred);
}

//==============================================================================
//  DynPanel
//==============================================================================
DynPanel::DynPanel (RefProProcessor& p)
{
    for (auto* sl : { &amount, &maxCut, &tol })
    {
        addAndMakeVisible (*sl);
        sl->setSliderStyle (juce::Slider::LinearBar);
        sl->setTextBoxStyle (juce::Slider::TextBoxLeft, false, 80, 20);
        sl->setTextBoxIsEditable (true);
        sl->setColour (juce::Slider::trackColourId, Theme::orange.withAlpha (0.35f));
    }
    addAndMakeVisible (speed);
    speed.addItemList ({ "Fast", "Medium", "Slow" }, 1);
    aAmount = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>   (p.apvts, "dynAmount", amount);
    aMax    = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>   (p.apvts, "dynMax", maxCut);
    aTol    = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>   (p.apvts, "dynTol", tol);
    aSpeed  = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (p.apvts, "dynSpeed", speed);
    setVisible (false);
}

DynPanel::~DynPanel() { aAmount.reset(); aMax.reset(); aTol.reset(); aSpeed.reset(); }

void DynPanel::resized()
{
    const float w = (float) getWidth(), gap = 8.0f * s, y = 18.0f * s, h = (float) getHeight() - y - 2.0f * s;
    const float cw = (w - gap * 3.0f) / 4.0f;
    amount.setBounds (juce::Rectangle<float> (0.0f, y, cw, h).toNearestInt());
    maxCut.setBounds (juce::Rectangle<float> (cw + gap, y, cw, h).toNearestInt());
    tol.setBounds    (juce::Rectangle<float> ((cw + gap) * 2.0f, y, cw, h).toNearestInt());
    speed.setBounds  (juce::Rectangle<float> ((cw + gap) * 3.0f, y, cw, h).toNearestInt());
}

void DynPanel::paint (juce::Graphics& g)
{
    g.setColour (Theme::dim);
    g.setFont (mkFont (11.5f * s));
    const char* labels[] = { "Amount", "Max Cut", "Tolerance", "Speed" };
    juce::Rectangle<int> rs[] = { amount.getBounds(), maxCut.getBounds(), tol.getBounds(), speed.getBounds() };
    for (int i = 0; i < 4; ++i)
        g.drawText (labels[i], juce::Rectangle<float> ((float) rs[i].getX(), 0.0f, (float) rs[i].getWidth(), 16.0f * s), juce::Justification::centred);
}

//==============================================================================
//  Editor
//==============================================================================
RefProEditor::RefProEditor (RefProProcessor& p)
    : juce::AudioProcessorEditor (&p), proc (p), graph (p), bandPanel (p), dynPanel (p), meters (p.analyzer)
{
    setLookAndFeel (&laf);
    addAndMakeVisible (list);
    addAndMakeVisible (graph);
    addAndMakeVisible (bandPanel);
    addAndMakeVisible (dynPanel);
    addAndMakeVisible (meters);
    for (auto* b : { &viewCurve, &viewBars, &targetPill, &captureBtn, &settingsBtn, &extraBtn, &eqBtn, &dynBtn })
        addAndMakeVisible (*b);

    graph.onSelect = [this] (int b) { selectBand (b); };
    graph.onNodeMoved = [this] { layoutBandPanel(); };
    bandPanel.onDelete = [this]
    {
        if (selectedBand >= 0) setParamG (proc.apvts, RefProProcessor::bandId (selectedBand, "on"), 0.0f);
        selectBand (-1);
    };

    dynBtn.setClickingTogglesState (true);
    dynAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, "dynOn", dynBtn);
    dynBtn.setTooltip ("Automatically attenuate peaks that exceed the reference curve");
    dynBtn.painter = [this] (juce::Graphics& g, Rectangle<float> r, bool over, bool, bool on)
    {
        g.setColour (on ? Theme::orange.withAlpha (0.28f) : (over ? Theme::panelHi.brighter (0.15f) : Theme::panelHi));
        g.fillRoundedRectangle (r, r.getHeight() * 0.5f);
        g.setColour (on ? Theme::orange : Theme::edge.brighter (0.25f));
        g.drawRoundedRectangle (r.reduced (0.5f), r.getHeight() * 0.5f, 1.2f);
        g.setColour (on ? Theme::orange : Theme::dim);
        g.fillEllipse (r.getX() + 16.0f * s, r.getCentreY() - 4.5f * s, 9.0f * s, 9.0f * s);
        g.setColour (on ? Theme::text : Theme::dim);
        g.setFont (mkFont (15.0f * s));
        g.drawText ("Dynamic", r.withTrimmedLeft (20.0f * s), juce::Justification::centred);
    };

    eqBtn.setClickingTogglesState (true);
    eqAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, "eqOn", eqBtn);
    eqBtn.setTooltip ("Turn the 10-band equalizer on/off");
    eqBtn.painter = [this] (juce::Graphics& g, Rectangle<float> r, bool over, bool, bool on)
    {
        g.setColour (on ? Theme::tealDark : (over ? Theme::panelHi.brighter (0.15f) : Theme::panelHi));
        g.fillRoundedRectangle (r, r.getHeight() * 0.5f);
        g.setColour (on ? Theme::teal : Theme::edge.brighter (0.25f));
        g.drawRoundedRectangle (r.reduced (0.5f), r.getHeight() * 0.5f, 1.2f);
        g.setColour (on ? Theme::teal : Theme::dim);
        g.fillEllipse (r.getX() + 16.0f * s, r.getCentreY() - 4.5f * s, 9.0f * s, 9.0f * s);
        g.setColour (on ? Theme::text : Theme::dim);
        g.setFont (mkFont (15.0f * s));
        g.drawText ("Equalizer", r.withTrimmedLeft (20.0f * s), juce::Justification::centred);
    };

    list.onSelect = [this] (int i) { selectTarget (i); };
    list.onDelete = [this] (int i)
    {
        const int sel = currentIndex();
        proc.deleteCapture (i);
        selectTarget (sel == i ? 0 : (sel > i ? sel - 1 : sel));
    };

    viewCurve.setClickingTogglesState (false);
    viewBars.setClickingTogglesState (false);
    viewCurve.setToggleState (true, juce::dontSendNotification);
    viewCurve.onClick = [this] { barsView = false; graph.setBars (false); viewCurve.setToggleState (true, juce::dontSendNotification); viewBars.setToggleState (false, juce::dontSendNotification); };
    viewBars.onClick  = [this] { barsView = true;  graph.setBars (true);  viewCurve.setToggleState (false, juce::dontSendNotification); viewBars.setToggleState (true, juce::dontSendNotification); };
    viewCurve.setTooltip ("Curve view");
    viewBars.setTooltip ("Bars view");

    viewCurve.painter = [this] (juce::Graphics& g, Rectangle<float> r, bool over, bool, bool on)
    {
        if (on) { g.setColour (Theme::panelHi.brighter (0.15f)); g.fillRoundedRectangle (r.reduced (2.0f), 14.0f * s); }
        Icons::curveIcon (g, r, (on || over) ? Theme::text : Theme::dim, 1.4f * s);
    };
    viewBars.painter = [this] (juce::Graphics& g, Rectangle<float> r, bool over, bool, bool on)
    {
        if (on) { g.setColour (Theme::panelHi.brighter (0.15f)); g.fillRoundedRectangle (r.reduced (2.0f), 14.0f * s); }
        Icons::barsIcon (g, r, (on || over) ? Theme::text : Theme::dim);
    };

    targetPill.onClick = [this] { showTargetMenu(); };
    targetPill.painter = [this] (juce::Graphics& g, Rectangle<float> r, bool over, bool, bool)
    {
        g.setGradientFill (juce::ColourGradient (Theme::tealDark.brighter (over ? 0.2f : 0.0f), r.getX(), 0.0f,
                                                 Theme::tealDark.darker (0.3f), r.getRight(), 0.0f, false));
        g.fillRoundedRectangle (r, r.getHeight() * 0.5f);
        g.setColour (Theme::teal.withAlpha (0.8f));
        g.drawRoundedRectangle (r.reduced (0.5f), r.getHeight() * 0.5f, 1.2f);
        g.setColour (Theme::text);
        g.setFont (mkFont (15.0f * s));
        g.drawText (current.name, r, juce::Justification::centred);
        // seta
        juce::Path a;
        a.startNewSubPath (r.getRight() - 30.0f * s, r.getCentreY() - 3.0f * s);
        a.lineTo (r.getRight() - 25.0f * s, r.getCentreY() + 2.0f * s);
        a.lineTo (r.getRight() - 20.0f * s, r.getCentreY() - 3.0f * s);
        g.strokePath (a, juce::PathStrokeType (1.5f * s));
        // alvo
        g.drawEllipse (r.getX() + 16.0f * s, r.getCentreY() - 8.0f * s, 16.0f * s, 16.0f * s, 1.3f);
        g.drawEllipse (r.getX() + 20.0f * s, r.getCentreY() - 4.0f * s, 8.0f * s, 8.0f * s, 1.3f);
    };

    captureBtn.onClick = [this] { doCapture(); };
    captureBtn.setTooltip ("Save the current average curve as a new target");
    captureBtn.painter = [this] (juce::Graphics& g, Rectangle<float> r, bool over, bool down, bool)
    {
        g.setColour (over ? Theme::panelHi.brighter (0.15f) : Theme::panelHi);
        g.fillRoundedRectangle (r, r.getHeight() * 0.5f);
        g.setColour (Theme::edge.brighter (0.2f));
        g.drawRoundedRectangle (r.reduced (0.5f), r.getHeight() * 0.5f, 1.0f);
        g.setColour (down ? Theme::amber : Theme::text);
        g.fillEllipse (r.getX() + 18.0f * s, r.getCentreY() - 5.0f * s, 10.0f * s, 10.0f * s);
        g.setFont (mkFont (15.0f * s));
        g.drawText ("Capture", r.withTrimmedLeft (22.0f * s), juce::Justification::centred);
    };

    settingsBtn.onClick = [this] { showSettingsMenu(); };
    settingsBtn.painter = [] (juce::Graphics& g, Rectangle<float> r, bool over, bool, bool)
    {
        Icons::gear (g, r, over ? Theme::text : Theme::dim, 1.4f);
    };

    extraBtn.onClick = [this]
    {
        showExtra = ! showExtra;
        extraBtn.setToggleState (showExtra, juce::dontSendNotification);
        graph.setExtra (showExtra);
        meters.setValues (showExtra);
    };
    extraBtn.painter = [this] (juce::Graphics& g, Rectangle<float> r, bool over, bool, bool on)
    {
        g.setColour (on ? Theme::tealDark.brighter (0.2f) : (over ? Theme::tealDark.brighter (0.1f) : Theme::tealDark));
        g.fillRoundedRectangle (r, r.getHeight() * 0.5f);
        g.setColour (Theme::teal.withAlpha (on ? 1.0f : 0.7f));
        g.drawRoundedRectangle (r.reduced (0.5f), r.getHeight() * 0.5f, 1.2f);
        g.setColour (Theme::text);
        g.setFont (mkFont (15.0f * s));
        g.drawText ("Extra Meters", r, juce::Justification::centred);
    };

    setResizable (true, true);
    setResizeLimits (840, 555, 1680, 1110);
    getConstrainer()->setFixedAspectRatio (1120.0 / 740.0);
    setSize (1120, 740);

    syncTarget (true);
    startTimerHz (10);
}

RefProEditor::~RefProEditor()
{
    stopTimer();
    eqAttach.reset();
    dynAttach.reset();
    setLookAndFeel (nullptr);
}

int RefProEditor::currentIndex() const
{
    return juce::jlimit (0, proc.getNumTargets() - 1, (int) proc.apvts.getRawParameterValue ("target")->load());
}

void RefProEditor::syncTarget (bool force)
{
    const int idx = currentIndex(), ver = proc.targetsVersion.load();
    if (! force && idx == cachedIndex && ver == cachedVersion) return;
    cachedIndex = idx; cachedVersion = ver;

    juce::StringArray names;
    for (int i = 0; i < proc.getNumTargets(); ++i) names.add (proc.getTarget (i).name);
    list.setItems (names, idx);

    current = proc.getTarget (idx);
    graph.setTarget (current);
    meters.setTarget (current);
    targetPill.repaint();
}

void RefProEditor::selectTarget (int idx)
{
    if (auto* prm = proc.apvts.getParameter ("target"))
    {
        prm->beginChangeGesture();
        prm->setValueNotifyingHost (prm->convertTo0to1 ((float) juce::jlimit (0, RefProProcessor::maxCaptures, idx)));
        prm->endChangeGesture();
    }
    syncTarget (true);
}

void RefProEditor::selectBand (int b)
{
    selectedBand = b;
    graph.setSelected (b);
    bandPanel.setBand (b);
    layoutBandPanel();
}

void RefProEditor::layoutBandPanel()
{
    if (selectedBand < 0) { bandPanel.setVisible (false); return; }

    const float w = 452.0f * s, h = 60.0f * s;
    const auto gb = graph.getBounds().toFloat();
    const auto node = graph.getNodePosition (selectedBand) + gb.getPosition();
    const float x = juce::jlimit (gb.getX() + 6.0f * s, gb.getRight() - w - 6.0f * s, node.x - w * 0.5f);
    float y = node.y + 20.0f * s;
    if (y + h > gb.getBottom() - 30.0f * s) y = node.y - 20.0f * s - h;
    y = juce::jmax (gb.getY() + 48.0f * s, y);
    bandPanel.setScale (s);
    bandPanel.setBounds (juce::Rectangle<float> (x, y, w, h).toNearestInt());
    bandPanel.setVisible (true);
    bandPanel.toFront (false);
}

void RefProEditor::resetEq()
{
    selectBand (-1);
    for (int b = 0; b < Eq::numBands; ++b)
        for (const char* id : { "on", "type", "freq", "gain", "q" })
            if (auto* prm = proc.apvts.getParameter (RefProProcessor::bandId (b, id)))
            {
                prm->beginChangeGesture();
                prm->setValueNotifyingHost (prm->getDefaultValue());
                prm->endChangeGesture();
            }
}

void RefProEditor::showTargetMenu()
{
    juce::PopupMenu m;
    for (int i = 0; i < proc.getNumTargets(); ++i)
        m.addItem (i + 1, proc.getTarget (i).name, true, i == currentIndex());
    m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&targetPill),
                     [this] (int r) { if (r > 0) selectTarget (r - 1); });
}

void RefProEditor::showSettingsMenu()
{
    juce::PopupMenu avg;
    const int cur = (int) proc.apvts.getRawParameterValue ("avg")->load();
    const char* labels[] = { "Short (3 s)", "Medium (10 s)", "Long (30 s)", "Infinite (whole song)" };
    for (int i = 0; i < 4; ++i) avg.addItem (10 + i, labels[i], true, i == cur);

    const int post = (int) proc.apvts.getRawParameterValue ("anPost")->load();
    juce::PopupMenu pt;
    pt.addItem (20, "Pre-EQ (input)", true, post == 0);
    pt.addItem (21, "Post-EQ (output)", true, post == 1);

    juce::PopupMenu m;
    m.addSubMenu ("Averaging", avg);
    m.addSubMenu ("Analysis point", pt);
    m.addSeparator();
    m.addItem (1, "Reset analysis");
    m.addItem (2, "Reset equalizer");

    m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&settingsBtn), [this] (int r)
    {
        if (r == 1) proc.analyzer.requestReset();
        else if (r == 2) resetEq();
        else if (r == 20 || r == 21)
        {
            setParamG (proc.apvts, "anPost", (float) (r - 20));
            proc.analyzer.requestReset();
        }
        else if (r >= 10 && r <= 13)
            if (auto* prm = proc.apvts.getParameter ("avg"))
            {
                prm->beginChangeGesture();
                prm->setValueNotifyingHost (prm->convertTo0to1 ((float) (r - 10)));
                prm->endChangeGesture();
            }
    });
}

void RefProEditor::doCapture()
{
    using R = RefProProcessor::CaptureResult;
    switch (proc.captureCurrent())
    {
        case R::ok:     selectTarget (proc.getNumTargets() - 1); break;
        case R::noData: juce::AlertWindow::showMessageBoxAsync (juce::MessageBoxIconType::InfoIcon, "RefPro SILVER",
                            "Nothing to capture yet. Play some audio first."); break;
        case R::full:   juce::AlertWindow::showMessageBoxAsync (juce::MessageBoxIconType::WarningIcon, "RefPro SILVER",
                            "Capture limit reached (8). Right-click a capture in the list to delete it."); break;
    }
}

void RefProEditor::timerCallback()
{
    syncTarget (false);
    if (selectedBand >= 0) bandPanel.refresh();
    const bool dynOn = proc.apvts.getRawParameterValue ("dynOn")->load() > 0.5f;
    if (dynPanel.isVisible() != dynOn) { dynPanel.setVisible (dynOn); repaint (statusArea.toNearestInt()); }
    repaint (statusArea.toNearestInt());
}

void RefProEditor::resized()
{
    s = (float) getWidth() / 1120.0f;
    list.s = s; graph.setScale (s); meters.setScale (s); bandPanel.setScale (s);

    auto W = (float) getWidth(), H = (float) getHeight();
    const float m = 12.0f * s, sideW = 218.0f * s;
    const float x0 = m + sideW + 10.0f * s;

    list.setBounds (juce::Rectangle<float> (m + 4.0f * s, m + 74.0f * s, sideW - 8.0f * s, 40.0f * s * 9.0f).toNearestInt());

    // cabeçalho
    const float hy = m, hh = 44.0f * s;
    viewCurve.setBounds (juce::Rectangle<float> (x0 + 236.0f * s, hy, 46.0f * s, hh).toNearestInt());
    viewBars.setBounds  (juce::Rectangle<float> (x0 + 282.0f * s, hy, 46.0f * s, hh).toNearestInt());
    settingsBtn.setBounds (juce::Rectangle<float> (W - m - 46.0f * s, hy, 46.0f * s, hh).toNearestInt());
    captureBtn.setBounds (juce::Rectangle<float> (W - m - 46.0f * s - 148.0f * s, hy + 3.0f * s, 140.0f * s, hh - 6.0f * s).toNearestInt());
    const float pillX = x0 + 348.0f * s;
    targetPill.setBounds (juce::Rectangle<float> (pillX, hy + 1.0f * s, captureBtn.getX() - pillX - 10.0f * s, hh - 2.0f * s).toNearestInt());

    const float gy = hy + hh + 10.0f * s;
    const float gH = 440.0f * s;
    graph.setBounds (juce::Rectangle<float> (x0, gy, W - m - x0, gH).toNearestInt());

    const float ry = gy + gH + 10.0f * s;
    const float by = ry + 7.0f * s;
    extraBtn.setBounds (juce::Rectangle<float> (x0, by, 150.0f * s, 36.0f * s).toNearestInt());
    eqBtn.setBounds    (juce::Rectangle<float> (x0 + 158.0f * s, by, 124.0f * s, 36.0f * s).toNearestInt());
    dynBtn.setBounds   (juce::Rectangle<float> (x0 + 290.0f * s, by, 116.0f * s, 36.0f * s).toNearestInt());
    const float px = x0 + 416.0f * s;
    dynPanel.setScale (s);
    dynPanel.setBounds (juce::Rectangle<float> (px, ry, W - m - px - 26.0f * s, 50.0f * s).toNearestInt());
    statusArea = juce::Rectangle<float> (x0 + 416.0f * s, ry, W - m - x0 - 416.0f * s, 50.0f * s);

    const float my = ry + 50.0f * s + 10.0f * s;
    meters.setBounds (juce::Rectangle<float> (x0, my, W - m - x0, H - m - my).toNearestInt());
    juce::ignoreUnused (H);
    layoutBandPanel();
}

void RefProEditor::paint (juce::Graphics& g)
{
    g.fillAll (Theme::bg);

    const float m = 12.0f * s, sideW = 218.0f * s;
    auto side = Rectangle<float> (m, m, sideW, (float) getHeight() - 2.0f * m);
    g.setColour (Theme::panel);
    g.fillRoundedRectangle (side, 10.0f * s);
    g.setColour (Theme::edge);
    g.drawRoundedRectangle (side.reduced (0.5f), 10.0f * s, 1.0f);

    g.setColour (Theme::text);
    g.setFont (mkFont (20.0f * s, true));
    g.drawText ("Target", Rectangle<float> (side.getX() + 14.0f * s, side.getY() + 14.0f * s, 120.0f * s, 28.0f * s), juce::Justification::centredLeft);
    g.setColour (Theme::edge);
    g.drawHorizontalLine ((int) (side.getY() + 56.0f * s), side.getX() + 10.0f * s, side.getRight() - 10.0f * s);

    g.setColour (Theme::dim);
    g.setFont (mkFont (12.0f * s));
    g.drawFittedText ("Capture stores the current average curve as a new target. Right-click a capture to delete it.",
                      juce::Rectangle<int> ((int) (side.getX() + 14.0f * s), (int) (side.getBottom() - 62.0f * s),
                                            (int) (side.getWidth() - 28.0f * s), (int) (52.0f * s)),
                      juce::Justification::bottomLeft, 4);

    // logotipo
    const float x0 = m + sideW + 10.0f * s, hy = m, hh = 44.0f * s;
    const float cx = x0 + 22.0f * s, cy = hy + hh * 0.5f, R = 17.0f * s;
    g.setColour (Theme::teal);
    g.drawEllipse (cx - R, cy - R, R * 2.0f, R * 2.0f, 2.0f * s);
    g.fillEllipse (cx - 5.0f * s, cy - 4.0f * s, 10.0f * s, 10.0f * s);
    g.setColour (Theme::text);
    g.fillEllipse (cx + 6.0f * s, cy - 11.0f * s, 5.0f * s, 5.0f * s);

    g.setColour (Theme::text);
    g.setFont (mkFont (19.0f * s, true));
    g.drawText ("REFPRO", Rectangle<float> (x0 + 48.0f * s, hy + 2.0f * s, 170.0f * s, 22.0f * s), juce::Justification::centredLeft);
    g.setColour (Theme::teal);
    g.drawText ("SILVER", Rectangle<float> (x0 + 48.0f * s, hy + 22.0f * s, 170.0f * s, 20.0f * s), juce::Justification::centredLeft);

    // grupo dos botões de visualização
    auto grp = Rectangle<float> (x0 + 236.0f * s, hy + 2.0f * s, 92.0f * s, hh - 4.0f * s);
    g.setColour (Theme::panel);
    g.fillRoundedRectangle (grp, grp.getHeight() * 0.5f);
    g.setColour (Theme::edge.brighter (0.2f));
    g.drawRoundedRectangle (grp.reduced (0.5f), grp.getHeight() * 0.5f, 1.0f);

    // status
    if (! statusArea.isEmpty())
    {
        static const char* avgNames[] = { "3 s", "10 s", "30 s", "whole song" };
        const int a = juce::jlimit (0, 3, (int) proc.apvts.getRawParameterValue ("avg")->load());
        const bool sig = proc.analyzer.signalActive.load();
        g.setFont (mkFont (13.0f * s));
        g.setColour (Theme::dim);
        if (! dynPanel.isVisible())
        g.drawText (juce::String ("Averaging: ") + avgNames[a] + (proc.apvts.getRawParameterValue ("anPost")->load() > 0.5f ? "  |  Post-EQ" : "  |  Pre-EQ"), statusArea.withTrimmedRight (30.0f * s), juce::Justification::centredRight);
        g.setColour (sig ? Theme::teal : Theme::edge.brighter (0.3f));
        g.fillEllipse (statusArea.getRight() - 18.0f * s, statusArea.getCentreY() - 4.5f * s, 9.0f * s, 9.0f * s);
    }
}
