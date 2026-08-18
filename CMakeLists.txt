cmake_minimum_required(VERSION 3.22)
project(OmnariaEngine3Test VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  JUCE
  GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
  GIT_TAG 8.0.3
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(OmnariaEngine3Test
  COMPANY_NAME "LATHAMAUDIO"
  BUNDLE_ID "com.lathamaudio.omnariaengine3test"
  IS_SYNTH TRUE
  NEEDS_MIDI_INPUT TRUE
  NEEDS_MIDI_OUTPUT FALSE
  IS_MIDI_EFFECT FALSE
  EDITOR_WANTS_KEYBOARD_FOCUS FALSE
  COPY_PLUGIN_AFTER_BUILD FALSE
  PLUGIN_MANUFACTURER_CODE Lath
  PLUGIN_CODE Oe3T
  FORMATS VST3 Standalone
  PRODUCT_NAME "Omnaria Engine 3 Test"
)

juce_generate_juce_header(OmnariaEngine3Test)

target_sources(OmnariaEngine3Test PRIVATE
  Source/PluginProcessor.cpp
  Source/PluginProcessor.h
  Source/PluginEditor.cpp
  Source/PluginEditor.h
  Source/Engine3Voice.cpp
  Source/Engine3Voice.h
)

target_compile_definitions(OmnariaEngine3Test PUBLIC
  JUCE_WEB_BROWSER=0
  JUCE_USE_CURL=0
  JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(OmnariaEngine3Test PRIVATE
  juce::juce_audio_utils
  juce::juce_dsp
  PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags juce::juce_recommended_warning_flags
)
