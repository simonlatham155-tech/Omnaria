#pragma once

#include <JuceHeader.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <string>
#include <vector>

namespace omnaria
{
struct SoundQAMetrics
{
    float peakDb = -100.0f;
    float rmsDb = -100.0f;
    float crestDb = 0.0f;
    float stereoCorrelation = 1.0f;
    float lowBandSideRatio = 0.0f;
    float transientRatio = 0.0f;
    float dcOffset = 0.0f;
    float highBandEnergyRatio = 0.0f;
    float spectralCentroidHz = 0.0f;
    float peakWaste = 0.0f;
    std::vector<std::string> warnings;
};

// Phase 9 engineering analyser. It deliberately reports physical measurements
// and warnings rather than inventing a single "quality score". Ears remain the
// final authority; these metrics tell us what to listen for.
class SoundQA
{
public:
    static SoundQAMetrics analyse(const juce::AudioBuffer<float>& buffer, double sampleRate)
    {
        SoundQAMetrics m;
        const auto channels = buffer.getNumChannels();
        const auto samples = buffer.getNumSamples();
        if (channels <= 0 || samples <= 0 || sampleRate <= 0.0)
            return m;

        double sumSq = 0.0, sum = 0.0, peak = 0.0;
        double lr = 0.0, ll = 0.0, rr = 0.0;
        double attackSq = 0.0, sustainSq = 0.0;
        const int attackN = std::max(1, std::min(samples, static_cast<int>(0.030 * sampleRate)));

        for (int i = 0; i < samples; ++i)
        {
            const float l = buffer.getSample(0, i);
            const float r = channels > 1 ? buffer.getSample(1, i) : l;
            const float mono = 0.5f * (l + r);
            peak = std::max(peak, static_cast<double>(std::max(std::abs(l), std::abs(r))));
            sumSq += static_cast<double>(mono) * mono;
            sum += mono;
            lr += static_cast<double>(l) * r;
            ll += static_cast<double>(l) * l;
            rr += static_cast<double>(r) * r;
            if (i < attackN) attackSq += static_cast<double>(mono) * mono;
            else sustainSq += static_cast<double>(mono) * mono;
        }

        const double rms = std::sqrt(sumSq / samples);
        m.peakDb = juce::Decibels::gainToDecibels(static_cast<float>(peak), -100.0f);
        m.rmsDb = juce::Decibels::gainToDecibels(static_cast<float>(rms), -100.0f);
        m.crestDb = m.peakDb - m.rmsDb;
        m.dcOffset = static_cast<float>(sum / samples);
        m.stereoCorrelation = static_cast<float>(lr / std::sqrt(std::max(1.0e-20, ll * rr)));
        const double attackRms = std::sqrt(attackSq / attackN);
        const int sustainN = std::max(1, samples - attackN);
        const double sustainRms = std::sqrt(sustainSq / sustainN);
        m.transientRatio = static_cast<float>(attackRms / std::max(1.0e-9, sustainRms));
        m.peakWaste = std::max(0.0f, m.crestDb - 14.0f);

        analyseSpectrum(buffer, sampleRate, m);
        addWarnings(m);
        return m;
    }

private:
    static void analyseSpectrum(const juce::AudioBuffer<float>& buffer, double sampleRate, SoundQAMetrics& m)
    {
        constexpr int order = 11;
        constexpr int fftSize = 1 << order;
        juce::dsp::FFT fft(order);
        std::array<float, fftSize * 2> data {};
        const int n = std::min(buffer.getNumSamples(), fftSize);
        for (int i = 0; i < n; ++i)
        {
            const float l = buffer.getSample(0, i);
            const float r = buffer.getNumChannels() > 1 ? buffer.getSample(1, i) : l;
            const float w = 0.5f - 0.5f * std::cos(juce::MathConstants<float>::twoPi * i / std::max(1, n - 1));
            data[static_cast<size_t>(i)] = 0.5f * (l + r) * w;
        }
        fft.performFrequencyOnlyForwardTransform(data.data());

        double weightedHz = 0.0, total = 0.0, high = 0.0;
        for (int k = 1; k < fftSize / 2; ++k)
        {
            const double hz = k * sampleRate / fftSize;
            const double e = static_cast<double>(data[static_cast<size_t>(k)]) * data[static_cast<size_t>(k)];
            weightedHz += hz * e;
            total += e;
            if (hz >= 8000.0) high += e;
        }
        m.spectralCentroidHz = total > 0.0 ? static_cast<float>(weightedHz / total) : 0.0f;
        m.highBandEnergyRatio = total > 0.0 ? static_cast<float>(high / total) : 0.0f;

        // Approximate low-band side-energy measurement with a one-pole LPF on
        // mid and side. It is intentionally conservative: excessive side below
        // ~140 Hz is a translation warning, not an automatic failure.
        const float a = std::exp(-juce::MathConstants<float>::twoPi * 140.0f / static_cast<float>(sampleRate));
        float midLP = 0.0f, sideLP = 0.0f;
        double midE = 0.0, sideE = 0.0;
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            const float l = buffer.getSample(0, i);
            const float r = buffer.getNumChannels() > 1 ? buffer.getSample(1, i) : l;
            const float mid = 0.5f * (l + r);
            const float side = 0.5f * (l - r);
            midLP = (1.0f - a) * mid + a * midLP;
            sideLP = (1.0f - a) * side + a * sideLP;
            midE += static_cast<double>(midLP) * midLP;
            sideE += static_cast<double>(sideLP) * sideLP;
        }
        m.lowBandSideRatio = static_cast<float>(sideE / std::max(1.0e-12, midE + sideE));
    }

    static void addWarnings(SoundQAMetrics& m)
    {
        if (m.peakDb > -0.3f) m.warnings.emplace_back("headroom: peak is too close to full scale");
        if (std::abs(m.dcOffset) > 0.01f) m.warnings.emplace_back("translation: measurable DC offset");
        if (m.lowBandSideRatio > 0.12f) m.warnings.emplace_back("bass: excessive stereo side energy below ~140 Hz");
        if (m.stereoCorrelation < -0.15f) m.warnings.emplace_back("stereo: negative correlation may collapse in mono");
        if (m.peakWaste > 3.0f) m.warnings.emplace_back("dynamics: high peak cost relative to average energy");
        if (m.highBandEnergyRatio > 0.28f) m.warnings.emplace_back("spectrum: unusually high energy above 8 kHz; check harshness/aliasing");
    }
};
}
