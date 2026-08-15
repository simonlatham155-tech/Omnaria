#include "WorldGlobe.h"
#include <cmath>

namespace omnaria
{
WorldGlobe::WorldGlobe(const LatWorldState& worldState)
    : state(worldState)
{
    setOpaque(false);
    startTimerHz(30);
}

void WorldGlobe::paint(juce::Graphics& g)
{
    auto area = getLocalBounds().toFloat().reduced(12.0f);
    const auto diameter = juce::jmin(area.getWidth(), area.getHeight()) - 46.0f;
    const juce::Rectangle<float> sphere(area.getCentreX() - diameter * 0.5f,
                                        area.getCentreY() - diameter * 0.5f - 6.0f,
                                        diameter,
                                        diameter);
    const auto centre = sphere.getCentre();
    const auto radius = sphere.getWidth() * 0.5f;

    const auto accent = juce::Colour::fromRGB(150, 100, 255);
    const auto cyan = juce::Colour::fromRGB(70, 214, 255);
    const auto amber = juce::Colour::fromRGB(255, 174, 70);
    const auto energy = state.worldEnergy.load();
    const auto evolution = state.evolution.load();
    const auto memory = state.memoryState.load();
    const auto gravity = state.gravity.load();
    const auto phrase = state.phrasePosition.load();
    const auto low = state.spectralLow.load();
    const auto mid = state.spectralMid.load();
    const auto high = state.spectralHigh.load();

    juce::ColourGradient globeFill(accent.withAlpha(0.38f + 0.20f * energy),
                                   centre.x - radius * 0.35f, centre.y - radius * 0.45f,
                                   juce::Colour::fromRGB(14, 12, 26).withAlpha(0.96f),
                                   centre.x + radius * 0.72f, centre.y + radius * 0.72f,
                                   false);
    globeFill.addColour(0.48, cyan.withAlpha(0.10f + high * 0.12f));
    g.setGradientFill(globeFill);
    g.fillEllipse(sphere);

    g.setColour(accent.withAlpha(0.42f + energy * 0.30f));
    g.drawEllipse(sphere, 1.5f);

    for (int ring = -2; ring <= 2; ++ring)
    {
        const auto yOffset = static_cast<float>(ring) * radius * 0.24f;
        const auto squeeze = std::sqrt(juce::jmax(0.05f, 1.0f - std::pow(yOffset / radius, 2.0f)));
        auto ringBounds = juce::Rectangle<float>(centre.x - radius * squeeze,
                                                 centre.y + yOffset - radius * 0.13f,
                                                 radius * 2.0f * squeeze,
                                                 radius * (0.24f + 0.08f * evolution));
        g.setColour(cyan.withAlpha(0.055f + 0.045f * evolution));
        g.drawEllipse(ringBounds, 1.0f);
    }

    for (int meridian = 0; meridian < 5; ++meridian)
    {
        const auto widthScale = 0.18f + 0.18f * static_cast<float>(meridian);
        auto meridianBounds = sphere.withSizeKeepingCentre(sphere.getWidth() * widthScale, sphere.getHeight());
        g.setColour(accent.withAlpha(0.045f + 0.025f * gravity));
        g.drawEllipse(meridianBounds, 1.0f);
    }

    constexpr int particleCount = 84;
    constexpr float goldenAngle = 2.39996323f;
    for (int i = 0; i < particleCount; ++i)
    {
        const auto fraction = (static_cast<float>(i) + 0.5f) / static_cast<float>(particleCount);
        auto latitude = std::asin(2.0f * fraction - 1.0f);
        auto longitude = static_cast<float>(i) * goldenAngle + animationPhase + phrase * juce::MathConstants<float>::twoPi;

        const auto warp = evolution * (0.22f + memory * 0.28f)
                        * std::sin(longitude * (2.0f + 3.0f * high) + static_cast<float>(i) * 0.17f);
        latitude += warp * 0.32f;
        longitude += warp;

        const auto radialPulse = 0.78f + 0.13f * energy
                               + 0.10f * evolution * std::sin(animationPhase * 1.7f + static_cast<float>(i));
        const auto point = projectParticle(longitude, latitude, radius * radialPulse, centre);

        const auto bandPosition = static_cast<float>(i % 3);
        auto colour = bandPosition < 0.5f ? amber.interpolatedWith(accent, 1.0f - low)
                    : bandPosition < 1.5f ? accent.interpolatedWith(cyan, mid)
                                          : cyan.interpolatedWith(juce::Colours::white, high * 0.35f);
        const auto dotSize = 1.7f + energy * 2.2f + ((i % 11) == 0 ? 1.5f * memory : 0.0f);
        g.setColour(colour.withAlpha(0.30f + 0.52f * gravity));
        g.fillEllipse(point.x - dotSize * 0.5f, point.y - dotSize * 0.5f, dotSize, dotSize);
    }

    const auto noteCount = static_cast<float>(state.activeNotes.load());
    const auto coreRadius = 8.0f + 16.0f * gravity + juce::jmin(16.0f, noteCount * 2.4f);
    juce::ColourGradient coreGlow(juce::Colours::white.withAlpha(0.75f), centre.x, centre.y,
                                  accent.withAlpha(0.0f), centre.x + coreRadius * 2.2f, centre.y, true);
    g.setGradientFill(coreGlow);
    g.fillEllipse(centre.x - coreRadius, centre.y - coreRadius, coreRadius * 2.0f, coreRadius * 2.0f);

    static const juce::StringArray pitchClasses { "C", "C#", "D", "Eb", "E", "F", "F#", "G", "Ab", "A", "Bb", "B" };
    const auto root = juce::jlimit(0, 11, state.rootPitchClass.load());
    g.setColour(juce::Colours::white.withAlpha(0.86f));
    g.setFont(juce::FontOptions(18.0f, juce::Font::bold));
    g.drawText(pitchClasses[root], sphere.withSizeKeepingCentre(54.0f, 28.0f), juce::Justification::centred);

    g.setFont(juce::FontOptions(11.0f));
    g.setColour(juce::Colours::white.withAlpha(0.48f));
    const auto status = "PHRASE " + juce::String(static_cast<int>(phrase * 100.0f)).paddedLeft('0', 2)
                      + "%   ENERGY " + juce::String(static_cast<int>(energy * 100.0f)).paddedLeft('0', 2)
                      + "%   MEMORY " + juce::String(static_cast<int>(memory * 100.0f)).paddedLeft('0', 2) + "%";
    g.drawText(status, area.removeFromBottom(26.0f), juce::Justification::centred);
}

void WorldGlobe::timerCallback()
{
    const auto active = state.activeNotes.load() > 0;
    const auto playing = state.hostPlaying.load();
    const auto energy = state.worldEnergy.load();
    if (active || playing)
        animationPhase += 0.010f + 0.028f * energy;

    if (animationPhase > juce::MathConstants<float>::twoPi)
        animationPhase -= juce::MathConstants<float>::twoPi;

    repaint();
}

juce::Point<float> WorldGlobe::projectParticle(float longitude, float latitude, float radius, juce::Point<float> centre) const
{
    const auto cosLat = std::cos(latitude);
    const auto x = cosLat * std::cos(longitude);
    const auto y = std::sin(latitude);
    const auto z = cosLat * std::sin(longitude);
    const auto perspective = 0.78f + 0.22f * (z * 0.5f + 0.5f);
    return { centre.x + x * radius * perspective, centre.y + y * radius * perspective };
}
} // namespace omnaria
