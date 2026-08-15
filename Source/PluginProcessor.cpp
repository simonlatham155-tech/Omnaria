#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

OmnariaAudioProcessor::OmnariaAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "OMNARIA_STATE", createParameterLayout()),
      stateEngine(engineState)
{
    for (int i = 0; i < 16; ++i)
        synthesiser.addVoice(new omnaria::OmnariaVoice(parameters, engineState));

    synthesiser.addSound(new omnaria::OmnariaSound());
    synthesiser.setNoteStealingEnabled(true);
}

juce::AudioProcessorValueTreeState::ParameterLayout OmnariaAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> layout;

    layout.push_back(std::make_unique<juce::AudioParameterChoice>("oscA_shape", "Osc A Shape", juce::StringArray { "Saw", "Pulse", "Sine" }, 0));
    layout.push_back(std::make_unique<juce::AudioParameterChoice>("oscB_shape", "Osc B Shape", juce::StringArray { "Saw", "Pulse", "Sine" }, 0));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("osc_mix", "Osc Mix", juce::NormalisableRange<float>(0.0f, 1.0f), 0.28f));
    layout.push_back(std::make_unique<juce::AudioParameterInt>("oscB_coarse", "Osc B Coarse", -24, 24, 0));
    layout.push_back(std::make_unique<juce::AudioParameterInt>("unison", "Unison", 1, 9, 5));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("detune", "Detune", juce::NormalisableRange<float>(0.0f, 50.0f, 0.01f), 12.0f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("spread", "Stereo Spread", juce::NormalisableRange<float>(0.0f, 1.0f), 0.82f));

    auto cutoffRange = juce::NormalisableRange<float>(20.0f, 20000.0f, 0.0f, 0.22f);
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("cutoff", "Cutoff", cutoffRange, 7200.0f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("resonance", "Resonance", juce::NormalisableRange<float>(0.2f, 12.0f, 0.001f, 0.5f), 0.72f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("drive", "Drive", juce::NormalisableRange<float>(0.0f, 24.0f, 0.01f), 1.5f));

    layout.push_back(std::make_unique<juce::AudioParameterFloat>("attack", "Attack", juce::NormalisableRange<float>(0.001f, 10.0f, 0.0f, 0.25f), 0.008f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("decay", "Decay", juce::NormalisableRange<float>(0.001f, 10.0f, 0.0f, 0.25f), 0.45f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("sustain", "Sustain", juce::NormalisableRange<float>(0.0f, 1.0f), 0.76f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("release", "Release", juce::NormalisableRange<float>(0.005f, 20.0f, 0.0f, 0.25f), 0.75f));

    // Experimental performance-state controls. These do not imply a new synthesis method.
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("motion", "Motion", juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("history", "History", juce::NormalisableRange<float>(0.0f, 1.0f), 0.35f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("focus", "Focus", juce::NormalisableRange<float>(0.0f, 1.0f), 0.78f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("coupling", "Coupling", juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("output", "Output", juce::NormalisableRange<float>(-60.0f, 6.0f, 0.01f), -8.0f));

    return { layout.begin(), layout.end() };
}

void OmnariaAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    stateEngine.prepare(sampleRate);
    synthesiser.setCurrentPlaybackSampleRate(sampleRate);

    for (int i = 0; i < synthesiser.getNumVoices(); ++i)
        if (auto* voice = dynamic_cast<omnaria::OmnariaVoice*>(synthesiser.getVoice(i)))
            voice->prepare(sampleRate, samplesPerBlock);

    outputGain.prepare({ sampleRate, static_cast<juce::uint32>(juce::jmax(1, samplesPerBlock)), 2 });
    outputGain.setRampDurationSeconds(0.02);
}

void OmnariaAudioProcessor::releaseResources()
{
    stateEngine.reset();
    outputGain.reset();
}

bool OmnariaAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto output = layouts.getMainOutputChannelSet();
    return output == juce::AudioChannelSet::mono() || output == juce::AudioChannelSet::stereo();
}

void OmnariaAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    stateEngine.processBlock(midi,
                             getPlayHead(),
                             buffer.getNumSamples(),
                             parameters.getRawParameterValue("motion")->load(),
                             parameters.getRawParameterValue("history")->load(),
                             parameters.getRawParameterValue("focus")->load(),
                             parameters.getRawParameterValue("coupling")->load());

    synthesiser.renderNextBlock(buffer, midi, 0, buffer.getNumSamples());

    outputGain.setGainDecibels(parameters.getRawParameterValue("output")->load());
    juce::dsp::AudioBlock<float> block(buffer);
    outputGain.process(juce::dsp::ProcessContextReplacing<float>(block));
}

juce::AudioProcessorEditor* OmnariaAudioProcessor::createEditor()
{
    return new OmnariaAudioProcessorEditor(*this);
}

void OmnariaAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    if (auto xml = parameters.copyState().createXml())
        copyXmlToBinary(*xml, destData);
}

void OmnariaAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        parameters.replaceState(juce::ValueTree::fromXml(*xml));
}

void OmnariaAudioProcessor::setParameterFromActualValue(const juce::String& id, float actualValue)
{
    if (auto* parameter = parameters.getParameter(id))
    {
        parameter->beginChangeGesture();
        parameter->setValueNotifyingHost(parameter->convertTo0to1(actualValue));
        parameter->endChangeGesture();
    }
}

void OmnariaAudioProcessor::randomiseDiscoverable()
{
    auto& random = juce::Random::getSystemRandom();
    const auto uniform = [&random](float low, float high)
    {
        return low + random.nextFloat() * (high - low);
    };

    setParameterFromActualValue("oscA_shape", static_cast<float>(random.nextInt(3)));
    setParameterFromActualValue("oscB_shape", static_cast<float>(random.nextInt(3)));
    setParameterFromActualValue("osc_mix", uniform(0.10f, 0.72f));
    setParameterFromActualValue("oscB_coarse", static_cast<float>(juce::StringArray { "0", "7", "12", "-12" }[random.nextInt(4)].getIntValue()));
    setParameterFromActualValue("unison", static_cast<float>(1 + 2 * random.nextInt(5)));
    setParameterFromActualValue("detune", uniform(4.0f, 24.0f));
    setParameterFromActualValue("spread", uniform(0.45f, 1.0f));

    const auto logMin = std::log(260.0f);
    const auto logMax = std::log(14000.0f);
    setParameterFromActualValue("cutoff", std::exp(uniform(logMin, logMax)));
    setParameterFromActualValue("resonance", uniform(0.45f, 3.5f));
    setParameterFromActualValue("drive", uniform(0.0f, 8.0f));

    setParameterFromActualValue("motion", uniform(0.05f, 0.70f));
    setParameterFromActualValue("history", uniform(0.10f, 0.75f));
    setParameterFromActualValue("focus", uniform(0.55f, 1.0f));
    setParameterFromActualValue("coupling", uniform(0.0f, 0.25f));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new OmnariaAudioProcessor();
}
