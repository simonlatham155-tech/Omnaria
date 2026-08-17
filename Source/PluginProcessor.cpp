#include "PluginProcessor.h"
#include "PluginEditor.h"

OmnariaEngine3TestAudioProcessor::OmnariaEngine3TestAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "PARAMETERS", createParameterLayout())
{
    Engine3Params p;
    p.partialDensity = apvts.getRawParameterValue("partialDensity");
    p.spectralTilt = apvts.getRawParameterValue("spectralTilt");
    p.modulationDepth = apvts.getRawParameterValue("modDepth");
    p.resonance = apvts.getRawParameterValue("resonance");
    p.excitation = apvts.getRawParameterValue("excitation");
    p.nonlinearity = apvts.getRawParameterValue("nonlinearity");
    p.asymmetry = apvts.getRawParameterValue("asymmetry");
    p.attack = apvts.getRawParameterValue("attack");
    p.decay = apvts.getRawParameterValue("decay");
    p.sustain = apvts.getRawParameterValue("sustain");
    p.release = apvts.getRawParameterValue("release");

    for (int i = 0; i < 8; ++i)
        synth.addVoice(new Engine3Voice(p));
    synth.addSound(new Engine3Sound());
}

juce::AudioProcessorValueTreeState::ParameterLayout OmnariaEngine3TestAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    auto add01 = [&params](const char* id, const char* name, float def)
    {
        params.push_back(std::make_unique<juce::AudioParameterFloat>(id, name, juce::NormalisableRange<float>(0.0f, 1.0f), def));
    };

    add01("partialDensity", "Partial Density", 0.45f);
    add01("spectralTilt", "Spectral Tilt", 0.55f);
    add01("modDepth", "Modulation Depth", 0.25f);
    add01("resonance", "Resonance", 0.35f);
    add01("excitation", "Excitation", 0.20f);
    add01("nonlinearity", "Nonlinearity", 0.25f);
    add01("asymmetry", "Nonlinear Asymmetry", 0.10f);

    params.push_back(std::make_unique<juce::AudioParameterFloat>("attack", "Attack", juce::NormalisableRange<float>(0.001f, 4.0f, 0.0f, 0.35f), 0.01f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("decay", "Decay", juce::NormalisableRange<float>(0.01f, 4.0f, 0.0f, 0.35f), 0.35f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("sustain", "Sustain", juce::NormalisableRange<float>(0.0f, 1.0f), 0.75f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("release", "Release", juce::NormalisableRange<float>(0.01f, 8.0f, 0.0f, 0.35f), 0.8f));
    return { params.begin(), params.end() };
}

void OmnariaEngine3TestAudioProcessor::prepareToPlay(double sampleRate, int)
{
    synth.setCurrentPlaybackSampleRate(sampleRate);
    for (int i = 0; i < synth.getNumVoices(); ++i)
        if (auto* v = dynamic_cast<Engine3Voice*>(synth.getVoice(i)))
            v->prepare(sampleRate);
}

bool OmnariaEngine3TestAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::mono()
        || layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void OmnariaEngine3TestAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();
    synth.renderNextBlock(buffer, midi, 0, buffer.getNumSamples());
}

void OmnariaEngine3TestAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    if (auto xml = state.createXml())
        copyXmlToBinary(*xml, destData);
}

void OmnariaEngine3TestAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        if (xml->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessorEditor* OmnariaEngine3TestAudioProcessor::createEditor()
{
    return new OmnariaEngine3TestAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new OmnariaEngine3TestAudioProcessor();
}
