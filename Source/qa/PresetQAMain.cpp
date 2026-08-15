#include <JuceHeader.h>
#include "PresetRenderHarness.h"

int main(int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI juceInit;
    const auto results = omnaria::PresetRenderHarness::runAll();
    const auto json = omnaria::PresetRenderHarness::toJson(results);

    juce::File output = juce::File::getCurrentWorkingDirectory().getChildFile("Omnaria_Preset_QA.json");
    if (argc > 1) output = juce::File(argv[1]);
    if (! output.replaceWithText(json))
        return 2;

    int warnings = 0;
    for (const auto& preset : results)
        for (const auto& probe : preset.probes)
            warnings += static_cast<int>(probe.metrics.warnings.size());

    juce::Logger::writeToLog("OMNARIA preset QA complete: " + juce::String(results.size()) +
                             " presets, " + juce::String(warnings) + " engineering warnings");
    juce::Logger::writeToLog("Report: " + output.getFullPathName());
    return 0;
}
