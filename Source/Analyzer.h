#pragma once

#include "Common.h"

// Motor de análise (roda na thread de áudio, publica resultados via atomics).
//  - Curva tonal: FFT 16384 pts, bandas de 1/3 de oitava espaçadas em 1/12 de oitava,
//    potência em escala de Q constante, média exponencial + normalização (média 0 dB).
//  - Vocal Balance: energia da faixa vocal (300 Hz-3.4 kHz) / energia total.
//  - Dynamics: PSR = pico / RMS de curto prazo (3 s), suavizado.
//  - Stereo Width: energia Side / Mid, suavizada.
class TonalAnalyzer
{
public:
    static constexpr int fftOrder = 14;
    static constexpr int fftSize  = 1 << fftOrder;
    static constexpr int hopSize  = fftSize / 4;
    static constexpr int shortTermBlocks = 30;   // 30 x 100 ms = 3 s

    TonalAnalyzer()
        : fft (fftOrder),
          window ((size_t) fftSize, juce::dsp::WindowingFunction<float>::hann, false),
          fifo ((size_t) fftSize, 0.0f),
          fftData ((size_t) fftSize * 2, 0.0f)
    {
        for (auto& v : avgDb)  v.store (0.0f);
        for (auto& v : fastDb) v.store (0.0f);
        doReset();
    }

    // thread de mensagem / preparação
    void prepare (double sr)
    {
        sampleRate = sr;
        subLen     = juce::jmax (1, (int) (sr * 0.1));
        hopSeconds = (double) hopSize / sr;
        subSeconds = 0.1;
        doReset();
    }

    void requestReset()                       { resetFlag.store (true); }
    void setAveragingSeconds (float seconds)  { tau.store (seconds); }   // 0 = infinito

    // thread de áudio
    void process (const float* l, const float* r, int n)
    {
        if (resetFlag.exchange (false)) doReset();

        for (int i = 0; i < n; ++i)
        {
            const float L = l[i], R = r[i];
            const float m = 0.5f * (L + R), s = 0.5f * (L - R);

            fifo[(size_t) fifoPos] = m;
            fifoPos = (fifoPos + 1) & (fftSize - 1);
            if (++hopCount >= hopSize) { hopCount = 0; analyseFrame(); }

            subMid  += m * m;
            subSide += s * s;
            subLR   += 0.5f * (L * L + R * R);
            subPeak  = std::max (subPeak, std::max (std::abs (L), std::abs (R)));
            if (++subCount >= subLen) finishSubBlock();
        }
    }

    // resultados (leitura em qualquer thread)
    std::array<std::atomic<float>, rp::numBands> avgDb, fastDb;
    std::atomic<float> avgPsr { 11.0f }, avgWidthDb { -8.0f }, vocalRatioDb { -3.0f };
    std::atomic<bool>  hasData { false }, signalActive { false };

private:
    void doReset()
    {
        std::fill (fifo.begin(), fifo.end(), 0.0f);
        fifoPos = hopCount = subCount = ringPos = ringFilled = 0;
        subMid = subSide = subLR = subPeak = 0.0f;
        avgPow.fill (0.0f);
        fastPow.fill (0.0f);
        avgCount = fastCount = psrCount = widthCount = 0;
        psrAvg = 11.0f; midAvg = sideAvg = 0.0f;
        ringE.fill (0.0f); ringPk.fill (0.0f);
        active = false;
        for (auto& v : avgDb)  v.store (0.0f);
        for (auto& v : fastDb) v.store (0.0f);
        hasData.store (false);
        signalActive.store (false);
    }

    static float alphaFor (double dt, float tauSec, int& count)
    {
        ++count;
        const float running = 1.0f / (float) count;           // média corrida no início / modo infinito
        if (tauSec <= 0.0f) return running;
        return std::max (running, 1.0f - std::exp (-(float) dt / tauSec));
    }

    void publish (const std::array<float, rp::numBands>& pow,
                  std::array<std::atomic<float>, rp::numBands>& out)
    {
        std::array<float, rp::numBands> db {}, sm {};
        for (int i = 0; i < rp::numBands; ++i)
            db[(size_t) i] = 10.0f * std::log10 (pow[(size_t) i] + 1.0e-20f);

        float mean = 0.0f;
        for (int i = 0; i < rp::numBands; ++i)
        {
            float sum = 0.0f; int cnt = 0;
            for (int k = std::max (0, i - 2); k <= std::min (rp::numBands - 1, i + 2); ++k)
            { sum += db[(size_t) k]; ++cnt; }
            sm[(size_t) i] = sum / (float) cnt;
            mean += sm[(size_t) i];
        }
        mean /= (float) rp::numBands;

        for (int i = 0; i < rp::numBands; ++i)
            out[(size_t) i].store (sm[(size_t) i] - mean);
    }

    void analyseFrame()
    {
        if (! active) return;

        for (int i = 0; i < fftSize; ++i)
            fftData[(size_t) i] = fifo[(size_t) ((fifoPos + i) & (fftSize - 1))];
        window.multiplyWithWindowingTable (fftData.data(), (size_t) fftSize);
        std::fill (fftData.begin() + fftSize, fftData.end(), 0.0f);
        fft.performFrequencyOnlyForwardTransform (fftData.data(), true);

        const int   maxBin = fftSize / 2;
        const float binHz  = (float) sampleRate / (float) fftSize;
        const float norm   = 1.0f / ((float) fftSize * 0.5f);

        auto power = [&] (int k)
        {
            const float mag = fftData[(size_t) juce::jlimit (0, maxBin, k)] * norm;
            return mag * mag;
        };

        std::array<float, rp::numBands> frame {};
        for (int i = 0; i < rp::numBands; ++i)
        {
            const float fc  = rp::bandFreq (i);
            const float lo  = fc * 0.890899f / binHz;     // 2^(-1/6)
            const float hi  = std::min (fc * 1.122462f / binHz, (float) maxBin);   // 2^(+1/6)
            const float wid = std::max (hi - lo, 0.0f);

            float density;
            if (wid < 1.0f)
            {
                const float b = std::min (fc / binHz, (float) maxBin - 1.0f);
                const int   i0 = (int) b;
                const float fr = b - (float) i0;
                density = power (i0) * (1.0f - fr) + power (i0 + 1) * fr;
            }
            else
            {
                const int k0 = (int) std::ceil (lo), k1 = (int) std::floor (hi);
                float sum = 0.0f; int cnt = 0;
                for (int k = k0; k <= k1; ++k) { sum += power (k); ++cnt; }
                density = cnt > 0 ? sum / (float) cnt : power ((int) lo);
            }
            frame[(size_t) i] = density * std::max (wid, 0.05f);   // potência da banda (Q constante)
        }

        const float t = tau.load();
        const float aAvg  = alphaFor (hopSeconds, t, avgCount);
        const float aFast = alphaFor (hopSeconds, 0.4f, fastCount);
        for (int i = 0; i < rp::numBands; ++i)
        {
            avgPow[(size_t) i]  += aAvg  * (frame[(size_t) i] - avgPow[(size_t) i]);
            fastPow[(size_t) i] += aFast * (frame[(size_t) i] - fastPow[(size_t) i]);
        }

        publish (avgPow, avgDb);
        publish (fastPow, fastDb);

        double pv = 0.0, pt = 0.0;
        for (int i = 0; i < rp::numBands; ++i)
        {
            pt += avgPow[(size_t) i];
            if (rp::isVocalBand (i)) pv += avgPow[(size_t) i];
        }
        vocalRatioDb.store ((float) (10.0 * std::log10 ((pv + 1e-20) / (pt + 1e-20))));
        hasData.store (true);
    }

    void finishSubBlock()
    {
        const float eLR = subLR / (float) subLen;
        active = eLR > 1.0e-6f;                       // gate ~ -60 dBFS
        signalActive.store (active);

        ringE[(size_t) ringPos]  = eLR;
        ringPk[(size_t) ringPos] = subPeak;
        ringPos = (ringPos + 1) % shortTermBlocks;
        ringFilled = std::min (ringFilled + 1, shortTermBlocks);

        if (active)
        {
            float meanE = 0.0f, pk = 0.0f;
            for (int k = 0; k < ringFilled; ++k)
            { meanE += ringE[(size_t) k]; pk = std::max (pk, ringPk[(size_t) k]); }
            meanE /= (float) ringFilled;

            const float psr = 10.0f * std::log10 ((pk * pk + 1.0e-12f) / (meanE + 1.0e-12f));
            const float t = tau.load();
            psrAvg += alphaFor (subSeconds, t, psrCount) * (psr - psrAvg);
            avgPsr.store (psrAvg);

            const float a = alphaFor (subSeconds, t, widthCount);
            midAvg  += a * (subMid  / (float) subLen - midAvg);
            sideAvg += a * (subSide / (float) subLen - sideAvg);
            avgWidthDb.store (10.0f * std::log10 ((sideAvg + 1.0e-12f) / (midAvg + 1.0e-12f)));
        }

        subMid = subSide = subLR = subPeak = 0.0f;
        subCount = 0;
    }

    juce::dsp::FFT fft;
    juce::dsp::WindowingFunction<float> window;
    std::vector<float> fifo, fftData;

    double sampleRate = 44100.0, hopSeconds = 0.0, subSeconds = 0.1;
    int subLen = 4410;
    int fifoPos = 0, hopCount = 0, subCount = 0, ringPos = 0, ringFilled = 0;
    int avgCount = 0, fastCount = 0, psrCount = 0, widthCount = 0;
    float subMid = 0, subSide = 0, subLR = 0, subPeak = 0;
    float psrAvg = 11.0f, midAvg = 0, sideAvg = 0;
    bool  active = false;

    std::array<float, rp::numBands> avgPow {}, fastPow {};
    std::array<float, shortTermBlocks> ringE {}, ringPk {};

    std::atomic<bool>  resetFlag { false };
    std::atomic<float> tau { 10.0f };
};
