cmake_minimum_required(VERSION 3.22)

project(Omnaria VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.15
)
FetchContent_MakeAvailable(JUCE)

set(OMNARIA_SHARED_SOURCES
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
    Source/DiscoverEngine.h
    Source/Phase7Discover.h
    Source/InstructionEngine.h
    Source/FactoryPresets.h
    Source/dsp/BandlimitedOscillator.h
    Source/dsp/SupersawLaw.h
    Source/dsp/GlideLaw.h
    Source/dsp/LayerArchitecture.h
    Source/dsp/ExcitationResonator.h
    Source/dsp/TempoGateLaw.h
    Source/dsp/SoundQA.h
    Source/dsp/NastyCell.h
    Source/dsp/SamplePool.h
    Source/dsp/ProductionFX.cpp
    Source/dsp/ProductionFX.h
    Source/dsp/OmnariaState.h
    Source/dsp/OmnariaStateEngine.cpp
    Source/dsp/OmnariaStateEngine.h
    Source/dsp/OmnariaVoice.cpp
    Source/dsp/OmnariaVoice.h
    Source/ui/WorldGlobe.cpp
    Source/ui/WorldGlobe.h
)

juce_add_plugin(Omnaria
    COMPANY_NAME "LATHAMAUDIO"
    BUNDLE_ID com.lathamaudio.omnaria
    PLUGIN_MANUFACTURER_CODE Latm
    PLUGIN_CODE Omnr
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "OMNARIA"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
)

juce_generate_juce_header(Omnaria)
target_sources(Omnaria PRIVATE ${OMNARIA_SHARED_SOURCES})
target_compile_definitions(Omnaria PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(Omnaria PRIVATE juce::juce_audio_utils juce::juce_dsp PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags juce::juce_recommended_warning_flags)

# Phase 9 offline factory-program QA. This executable renders all 48 programs
# through deterministic mono-interval and triad probes and writes JSON metrics.
juce_add_console_app(OmnariaPresetQA PRODUCT_NAME "Omnaria Preset QA")
juce_generate_juce_header(OmnariaPresetQA)
target_sources(OmnariaPresetQA PRIVATE ${OMNARIA_SHARED_SOURCES} Source/qa/PresetRenderHarness.cpp Source/qa/PresetRenderHarness.h Source/qa/PresetQAMain.cpp)
target_compile_definitions(OmnariaPresetQA PRIVATE JucePlugin_Name="OMNARIA" JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(OmnariaPresetQA PRIVATE juce::juce_audio_utils juce::juce_dsp PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)
