#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

namespace
{
void applyFilterCharacter(juce::AudioBuffer<float>& buffer, int mode, float amount)
{
    amount = juce::jlimit(0.0f, 1.0f, amount);
    if (mode <= 0 || amount <= 0.0001f) return;

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* data = buffer.getWritePointer(ch);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            const auto x = data[i];
            float coloured = x;
            if (mode == 1) // Warm: gentle symmetric saturation.
            {
                const auto drive = 1.0f + 1.8f * amount;
                coloured = std::tanh(x * drive) / std::tanh(drive);
            }
            else if (mode == 2) // Acid: sharper resonance-friendly bite.
            {
                const auto drive = 1.0f + 3.2f * amount;
                const auto hard = std::tanh(x * drive) / std::tanh(drive);
                coloured = 0.72f * hard + 0.28f * std::sin(juce::MathConstants<float>::halfPi * juce::jlimit(-1.0f, 1.0f, x));
            }
            else // Driven: asymmetric console-like push.
            {
                const auto drive = 1.0f + 4.5f * amount;
                const auto biased = x * drive + 0.11f * amount;
                coloured = (2.0f / juce::MathConstants<float>::pi) * std::atan(biased * 1.8f) - 0.035f * amount;
            }
            data[i] = juce::jmap(amount, x, coloured);
        }
    }
}
}

OmnariaAudioProcessor::OmnariaAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "OMNARIA_STATE", createParameterLayout()), stateEngine(engineState)
{
    formatManager.registerBasicFormats();
    for (int i = 0; i < 16; ++i)
        synthesiser.addVoice(new omnaria::OmnariaVoice(parameters, engineState, samplePool));
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
    layout.push_back(std::make_unique<juce::AudioParameterChoice>("filter_character", "Filter Character", juce::StringArray { "Clean", "Warm", "Acid", "Driven" }, 0));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("filter_character_amount", "Filter Character Amount", juce::NormalisableRange<float>(0.0f, 1.0f), 0.55f));
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

    for (int i = 1; i <= 4; ++i)
    {
        const auto s = juce::String(i);
        layout.push_back(std::make_unique<juce::AudioParameterFloat>("lfo" + s + "_rate", "LFO " + s + " Rate", juce::NormalisableRange<float>(0.01f, 30.0f, 0.0f, 0.25f), i == 1 ? 1.0f : 0.25f * i));
        layout.push_back(std::make_unique<juce::AudioParameterChoice>("lfo" + s + "_mode", "LFO " + s + " Mode", juce::StringArray { "Free", "Retrig", "One Shot" }, 1));
        layout.push_back(std::make_unique<juce::AudioParameterChoice>("lfo" + s + "_sync", "LFO " + s + " Sync", juce::StringArray { "Off", "4/1", "2/1", "1/1", "1/2", "1/4", "1/8", "1/16" }, 0));
        layout.push_back(std::make_unique<juce::AudioParameterFloat>("macro" + s, "Macro " + s, juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));
    }

    for (int i = 1; i <= 3; ++i)
    {
        const auto s = juce::String(i);
        layout.push_back(std::make_unique<juce::AudioParameterFloat>("env" + s + "_attack", "Env " + s + " Attack", juce::NormalisableRange<float>(0.001f, 10.0f, 0.0f, 0.25f), 0.01f));
        layout.push_back(std::make_unique<juce::AudioParameterFloat>("env" + s + "_decay", "Env " + s + " Decay", juce::NormalisableRange<float>(0.001f, 10.0f, 0.0f, 0.25f), 0.40f));
        layout.push_back(std::make_unique<juce::AudioParameterFloat>("env" + s + "_sustain", "Env " + s + " Sustain", juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));
        layout.push_back(std::make_unique<juce::AudioParameterFloat>("env" + s + "_release", "Env " + s + " Release", juce::NormalisableRange<float>(0.005f, 20.0f, 0.0f, 0.25f), 0.25f));
    }

    layout.push_back(std::make_unique<juce::AudioParameterChoice>("nasty_model", "NASTY Model", juce::StringArray { "Fold", "Feedback", "Coupled", "Duffing" }, 0));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("nasty_amount", "NASTY Amount", juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("nasty_deform", "NASTY Deform", juce::NormalisableRange<float>(0.0f, 1.0f), 0.35f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("nasty_feedback", "NASTY Feedback", juce::NormalisableRange<float>(0.0f, 0.985f), 0.20f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("nasty_coupling", "NASTY Coupling", juce::NormalisableRange<float>(0.0f, 1.0f), 0.25f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("nasty_energy", "NASTY Energy", juce::NormalisableRange<float>(0.0f, 1.0f), 0.35f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("nasty_damping", "NASTY Damping", juce::NormalisableRange<float>(0.02f, 1.0f), 0.55f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("nasty_moment", "NASTY Moment", juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));

    layout.push_back(std::make_unique<juce::AudioParameterChoice>("sample_mode", "Sample Mode", juce::StringArray { "One Shot", "Loop", "Texture" }, 0));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("sample_level", "Sample Level", juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));
    layout.push_back(std::make_unique<juce::AudioParameterInt>("sample_root", "Sample Root", 0, 127, 60));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("sample_tune", "Sample Tune", juce::NormalisableRange<float>(-24.0f, 24.0f, 0.01f), 0.0f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("sample_start", "Sample Start", juce::NormalisableRange<float>(0.0f, 0.995f, 0.001f), 0.0f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("sample_end", "Sample End", juce::NormalisableRange<float>(0.005f, 1.0f, 0.001f), 1.0f));
    layout.push_back(std::make_unique<juce::AudioParameterBool>("sample_reverse", "Sample Reverse", false));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("sample_position", "Sample Position", juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("sample_scan", "Sample Scan", juce::NormalisableRange<float>(0.0f, 1.0f), 0.35f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("sample_jitter", "Sample Jitter", juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));

    const juce::StringArray modSources { "None", "LFO 1", "LFO 2", "LFO 3", "LFO 4", "Env 1", "Env 2", "Env 3", "Velocity", "Key", "Mod Wheel", "Aftertouch", "Macro 1", "Macro 2", "Macro 3", "Macro 4", "Brown", "Stochastic" };
    const juce::StringArray modDestinations { "None", "Pitch", "Cutoff", "Resonance", "Osc Mix", "Detune", "Spread", "Drive", "Pulse Width", "NASTY Amount", "NASTY Deform", "NASTY Feedback", "NASTY Coupling", "NASTY Energy", "NASTY Damping", "NASTY Moment", "Sample Level", "Sample Position", "Sample Scan", "Sample Jitter", "Sample Tune" };
    for (int i = 1; i <= 4; ++i)
    {
        const auto s = juce::String(i);
        layout.push_back(std::make_unique<juce::AudioParameterChoice>("mod" + s + "_source", "Mod " + s + " Source", modSources, 0));
        layout.push_back(std::make_unique<juce::AudioParameterChoice>("mod" + s + "_dest", "Mod " + s + " Destination", modDestinations, 0));
        layout.push_back(std::make_unique<juce::AudioParameterFloat>("mod" + s + "_depth", "Mod " + s + " Depth", juce::NormalisableRange<float>(-1.0f, 1.0f, 0.001f), 0.0f));
    }

    layout.push_back(std::make_unique<juce::AudioParameterChoice>("fx_order", "FX Order", juce::StringArray { "Motion>Delay>Space", "Delay>Motion>Space" }, 0));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("fx_motion_mix", "FX Motion Mix", juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("fx_motion_rate", "FX Motion Rate", juce::NormalisableRange<float>(0.03f, 8.0f, 0.0f, 0.35f), 0.30f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("fx_motion_depth", "FX Motion Depth", juce::NormalisableRange<float>(0.0f, 1.0f), 0.35f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("fx_delay_mix", "FX Delay Mix", juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));
    layout.push_back(std::make_unique<juce::AudioParameterChoice>("fx_delay_div", "FX Delay Division", juce::StringArray { "4/1", "2/1", "1/1", "1/2", "1/4", "3/4", "3/8", "1/8" }, 4));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("fx_delay_feedback", "FX Delay Feedback", juce::NormalisableRange<float>(0.0f, 0.94f), 0.32f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("fx_space_mix", "FX Space Mix", juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("fx_space_size", "FX Space Size", juce::NormalisableRange<float>(0.0f, 1.0f), 0.58f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("fx_space_damping", "FX Space Damping", juce::NormalisableRange<float>(0.0f, 1.0f), 0.42f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("fx_width", "FX Width", juce::NormalisableRange<float>(0.0f, 2.0f), 1.0f));

    layout.push_back(std::make_unique<juce::AudioParameterFloat>("motion", "Motion", juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("history", "History", juce::NormalisableRange<float>(0.0f, 1.0f), 0.35f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("focus", "Focus", juce::NormalisableRange<float>(0.0f, 1.0f), 0.78f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("coupling", "Coupling", juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("output", "Output", juce::NormalisableRange<float>(-60.0f, 6.0f, 0.01f), -8.0f));
    return { layout.begin(), layout.end() };
}

void OmnariaAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = juce::jmax(1.0, sampleRate);
    captureHistory.setSize(2, juce::jmax(1, static_cast<int>(currentSampleRate * 4.0)), false, true, false);
    captureHistory.clear(); captureWritePosition = 0;
    stateEngine.prepare(currentSampleRate);
    synthesiser.setCurrentPlaybackSampleRate(currentSampleRate);
    for (int i = 0; i < synthesiser.getNumVoices(); ++i)
        if (auto* voice = dynamic_cast<omnaria::OmnariaVoice*>(synthesiser.getVoice(i))) voice->prepare(currentSampleRate, samplesPerBlock);
    productionFX.prepare(currentSampleRate, samplesPerBlock);
    outputGain.prepare({ currentSampleRate, static_cast<juce::uint32>(juce::jmax(1, samplesPerBlock)), 2 });
    outputGain.setRampDurationSeconds(0.02);
}

void OmnariaAudioProcessor::releaseResources() { stateEngine.reset(); productionFX.reset(); outputGain.reset(); }

bool OmnariaAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto output = layouts.getMainOutputChannelSet();
    return output == juce::AudioChannelSet::mono() || output == juce::AudioChannelSet::stereo();
}

void OmnariaAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();
    stateEngine.processBlock(midi, getPlayHead(), buffer.getNumSamples(), parameters.getRawParameterValue("motion")->load(),
                             parameters.getRawParameterValue("history")->load(), parameters.getRawParameterValue("focus")->load(),
                             parameters.getRawParameterValue("coupling")->load());
    synthesiser.renderNextBlock(buffer, midi, 0, buffer.getNumSamples());

    applyFilterCharacter(buffer, juce::roundToInt(parameters.getRawParameterValue("filter_character")->load()),
                         parameters.getRawParameterValue("filter_character_amount")->load());

    productionFX.process(buffer, engineState.bpm.load(),
                         parameters.getRawParameterValue("fx_motion_mix")->load(), parameters.getRawParameterValue("fx_motion_rate")->load(), parameters.getRawParameterValue("fx_motion_depth")->load(),
                         parameters.getRawParameterValue("fx_delay_mix")->load(), juce::roundToInt(parameters.getRawParameterValue("fx_delay_div")->load()), parameters.getRawParameterValue("fx_delay_feedback")->load(),
                         parameters.getRawParameterValue("fx_space_mix")->load(), parameters.getRawParameterValue("fx_space_size")->load(), parameters.getRawParameterValue("fx_space_damping")->load(),
                         parameters.getRawParameterValue("fx_width")->load(), static_cast<omnaria::ProductionFX::Order>(juce::jlimit(0, 1, juce::roundToInt(parameters.getRawParameterValue("fx_order")->load()))));

    outputGain.setGainDecibels(parameters.getRawParameterValue("output")->load());
    juce::dsp::AudioBlock<float> block(buffer);
    outputGain.process(juce::dsp::ProcessContextReplacing<float>(block));
    writeCaptureHistory(buffer);
}

void OmnariaAudioProcessor::writeCaptureHistory(const juce::AudioBuffer<float>& buffer)
{
    if (captureHistory.getNumSamples() == 0) return;
    const auto size = captureHistory.getNumSamples();
    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        for (int ch = 0; ch < 2; ++ch)
        {
            const auto sourceCh = juce::jmin(ch, buffer.getNumChannels() - 1);
            captureHistory.setSample(ch, captureWritePosition, sourceCh >= 0 ? buffer.getSample(sourceCh, i) : 0.0f);
        }
        captureWritePosition = (captureWritePosition + 1) % size;
    }
}

bool OmnariaAudioProcessor::loadSampleFile(const juce::File& file)
{
    if (! file.existsAsFile()) return false;
    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));
    if (! reader) return false;
    const auto maxSamples = static_cast<juce::int64>(reader->sampleRate * 60.0);
    const auto length = static_cast<int>(juce::jmin(reader->lengthInSamples, maxSamples));
    if (length < 2) return false;
    auto data = std::make_shared<omnaria::SampleData>();
    data->audio.setSize(2, length);
    reader->read(&data->audio, 0, length, 0, true, reader->numChannels > 1);
    if (reader->numChannels == 1) data->audio.copyFrom(1, 0, data->audio, 0, 0, length);
    data->sampleRate = reader->sampleRate;
    data->name = file.getFileNameWithoutExtension();
    samplePool.publish(data);
    { const juce::ScopedLock lock(sampleNameLock); sampleName = data->name; }
    return true;
}

bool OmnariaAudioProcessor::captureRecentOutput()
{
    const auto available = captureHistory.getNumSamples();
    if (available < 2) return false;
    const auto length = juce::jmin(available, static_cast<int>(currentSampleRate * 2.0));
    auto data = std::make_shared<omnaria::SampleData>();
    data->audio.setSize(2, length);
    const auto start = (captureWritePosition - length + available) % available;
    for (int i = 0; i < length; ++i)
    {
        const auto src = (start + i) % available;
        data->audio.setSample(0, i, captureHistory.getSample(0, src));
        data->audio.setSample(1, i, captureHistory.getSample(1, src));
    }
    data->sampleRate = currentSampleRate; data->name = "RESAMPLE"; samplePool.publish(data);
    { const juce::ScopedLock lock(sampleNameLock); sampleName = data->name; }
    return true;
}

juce::String OmnariaAudioProcessor::getSampleName() const { const juce::ScopedLock lock(sampleNameLock); return sampleName; }
juce::AudioProcessorEditor* OmnariaAudioProcessor::createEditor() { return new OmnariaAudioProcessorEditor(*this); }
void OmnariaAudioProcessor::getStateInformation(juce::MemoryBlock& destData) { if (auto xml = parameters.copyState().createXml()) copyXmlToBinary(*xml, destData); }
void OmnariaAudioProcessor::setStateInformation(const void* data, int sizeInBytes) { if (auto xml = getXmlFromBinary(data, sizeInBytes)) parameters.replaceState(juce::ValueTree::fromXml(*xml)); }

void OmnariaAudioProcessor::setParameterFromActualValue(const juce::String& id, float actualValue)
{
    if (auto* p = parameters.getParameter(id)) { p->beginChangeGesture(); p->setValueNotifyingHost(p->convertTo0to1(actualValue)); p->endChangeGesture(); }
}

void OmnariaAudioProcessor::randomiseDiscoverable()
{
    auto& random = juce::Random::getSystemRandom();
    const auto current = [this](const juce::String& id) { if (auto* p = parameters.getParameter(id)) return p->convertFrom0to1(p->getValue()); return 0.0f; };
    const auto mutate = [&] (const juce::String& id, float amount, float low, float high)
    { setParameterFromActualValue(id, juce::jlimit(low, high, current(id) + (random.nextFloat() * 2.0f - 1.0f) * amount)); };
    mutate("cutoff", 1800.0f, 40.0f, 19000.0f); mutate("resonance", 1.2f, 0.2f, 12.0f); mutate("detune", 5.0f, 0.0f, 50.0f);
    mutate("nasty_amount", 0.14f, 0.0f, 1.0f); mutate("sample_level", 0.12f, 0.0f, 1.0f);
    mutate("fx_motion_mix", 0.10f, 0.0f, 1.0f); mutate("fx_delay_mix", 0.10f, 0.0f, 1.0f); mutate("fx_space_mix", 0.10f, 0.0f, 1.0f);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new OmnariaAudioProcessor(); }
