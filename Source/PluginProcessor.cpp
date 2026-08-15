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
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("pulse_width", "Pulse Width", juce::NormalisableRange<float>(0.05f, 0.95f, 0.001f), 0.50f));
    layout.push_back(std::make_unique<juce::AudioParameterChoice>("phase_mode", "Phase Mode", juce::StringArray { "Retrig", "Random" }, 0));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("phase", "Phase", juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));
    layout.push_back(std::make_unique<juce::AudioParameterInt>("unison", "Unison", 1, 9, 5));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("detune", "Detune", juce::NormalisableRange<float>(0.0f, 50.0f, 0.01f), 12.0f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("spread", "Stereo Spread", juce::NormalisableRange<float>(0.0f, 1.0f), 0.82f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("sub_level", "Sub Level", juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));
    layout.push_back(std::make_unique<juce::AudioParameterInt>("sub_octave", "Sub Octave", -2, 0, -1));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("noise_level", "Noise Level", juce::NormalisableRange<float>(0.0f, 0.5f), 0.0f));

    auto cutoffRange = juce::NormalisableRange<float>(20.0f, 20000.0f, 0.0f, 0.22f);
    layout.push_back(std::make_unique<juce::AudioParameterChoice>("filter_mode", "Filter Mode", juce::StringArray { "LP12", "LP24", "HP12", "BP12" }, 1));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("cutoff", "Cutoff", cutoffRange, 7200.0f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("resonance", "Resonance", juce::NormalisableRange<float>(0.2f, 12.0f, 0.001f, 0.5f), 0.72f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("keytrack", "Filter Keytrack", juce::NormalisableRange<float>(0.0f, 1.0f), 0.25f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("drive", "Drive", juce::NormalisableRange<float>(0.0f, 24.0f, 0.01f), 1.5f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("filter_env_amt", "Filter Env Amount", juce::NormalisableRange<float>(-6.0f, 6.0f, 0.01f), 0.0f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("velocity_timbre", "Velocity Timbre", juce::NormalisableRange<float>(0.0f, 1.0f), 0.35f));

    layout.push_back(std::make_unique<juce::AudioParameterFloat>("filter_attack", "Filter Attack", juce::NormalisableRange<float>(0.001f, 10.0f, 0.0f, 0.25f), 0.003f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("filter_decay", "Filter Decay", juce::NormalisableRange<float>(0.001f, 10.0f, 0.0f, 0.25f), 0.28f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("filter_sustain", "Filter Sustain", juce::NormalisableRange<float>(0.0f, 1.0f), 0.10f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("filter_release", "Filter Release", juce::NormalisableRange<float>(0.005f, 20.0f, 0.0f, 0.25f), 0.32f));

    layout.push_back(std::make_unique<juce::AudioParameterFloat>("attack", "Attack", juce::NormalisableRange<float>(0.001f, 10.0f, 0.0f, 0.25f), 0.008f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("decay", "Decay", juce::NormalisableRange<float>(0.001f, 10.0f, 0.0f, 0.25f), 0.45f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("sustain", "Sustain", juce::NormalisableRange<float>(0.0f, 1.0f), 0.76f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("release", "Release", juce::NormalisableRange<float>(0.005f, 20.0f, 0.0f, 0.25f), 0.75f));

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
    const auto uniform = [&random](float low, float high) { return low + random.nextFloat() * (high - low); };
    const auto current = [this](const juce::String& id)
    {
        if (auto* parameter = parameters.getParameter(id))
            return parameter->convertFrom0to1(parameter->getValue());
        return 0.0f;
    };
    const auto mutateLinear = [&] (const juce::String& id, float amount, float low, float high)
    {
        setParameterFromActualValue(id, juce::jlimit(low, high, current(id) + uniform(-amount, amount)));
    };
    const auto mutateRatio = [&] (const juce::String& id, float octaves, float low, float high)
    {
        setParameterFromActualValue(id, juce::jlimit(low, high, current(id) * std::pow(2.0f, uniform(-octaves, octaves))));
    };

    if (random.nextFloat() < 0.22f) setParameterFromActualValue("oscA_shape", static_cast<float>(random.nextInt(3)));
    if (random.nextFloat() < 0.22f) setParameterFromActualValue("oscB_shape", static_cast<float>(random.nextInt(3)));
    mutateLinear("osc_mix", 0.14f, 0.0f, 1.0f);
    mutateLinear("pulse_width", 0.08f, 0.05f, 0.95f);
    if (random.nextFloat() < 0.28f)
    {
        constexpr int musicalOffsets[] { -12, -7, 0, 7, 12 };
        setParameterFromActualValue("oscB_coarse", static_cast<float>(musicalOffsets[random.nextInt(5)]));
    }
    const auto currentUnison = juce::roundToInt(current("unison"));
    setParameterFromActualValue("unison", static_cast<float>(juce::jlimit(1, 9, currentUnison + (random.nextInt(3) - 1) * 2)));
    mutateLinear("detune", 5.0f, 0.0f, 50.0f);
    mutateLinear("spread", 0.12f, 0.0f, 1.0f);
    mutateLinear("sub_level", 0.10f, 0.0f, 1.0f);
    mutateLinear("noise_level", 0.05f, 0.0f, 0.5f);

    mutateRatio("cutoff", 0.65f, 40.0f, 19000.0f);
    mutateRatio("resonance", 0.45f, 0.2f, 12.0f);
    mutateLinear("keytrack", 0.12f, 0.0f, 1.0f);
    mutateLinear("drive", 3.5f, 0.0f, 24.0f);
    mutateLinear("filter_env_amt", 0.75f, -6.0f, 6.0f);
    mutateLinear("velocity_timbre", 0.12f, 0.0f, 1.0f);

    mutateRatio("filter_attack", 0.60f, 0.001f, 10.0f);
    mutateRatio("filter_decay", 0.60f, 0.001f, 10.0f);
    mutateLinear("filter_sustain", 0.12f, 0.0f, 1.0f);
    mutateRatio("filter_release", 0.55f, 0.005f, 20.0f);
    mutateRatio("attack", 0.65f, 0.001f, 10.0f);
    mutateRatio("decay", 0.55f, 0.001f, 10.0f);
    mutateLinear("sustain", 0.12f, 0.0f, 1.0f);
    mutateRatio("release", 0.55f, 0.005f, 20.0f);
    mutateLinear("motion", 0.14f, 0.0f, 1.0f);
    mutateLinear("history", 0.12f, 0.0f, 1.0f);
    mutateLinear("focus", 0.10f, 0.0f, 1.0f);
    mutateLinear("coupling", 0.12f, 0.0f, 1.0f);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new OmnariaAudioProcessor();
}
