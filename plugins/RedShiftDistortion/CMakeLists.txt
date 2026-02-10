cmake_minimum_required(VERSION 3.15)

# Plugin formats: VST3, AU, Standalone
juce_add_plugin(RedShiftDistortion
    COMPANY_NAME "RicMelts"
    PLUGIN_MANUFACTURER_CODE Pfre
    PLUGIN_CODE RsDs
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "RedShiftDistortion"
    NEEDS_WEB_BROWSER TRUE  # REQUIRED for VST3 WebView support (Pattern #9)
)

# Source files
target_sources(RedShiftDistortion
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
)

# Include paths
target_include_directories(RedShiftDistortion
    PRIVATE
        Source
)

# Required JUCE modules
target_link_libraries(RedShiftDistortion
    PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_plugin_client
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_data_structures
        juce::juce_dsp
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra  # Required for WebBrowserComponent
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# CRITICAL: Generate JuceHeader.h (JUCE 8 requirement, Pattern #1)
# Must come AFTER target_link_libraries
juce_generate_juce_header(RedShiftDistortion)

# Compile definitions
target_compile_definitions(RedShiftDistortion
    PUBLIC
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_WEB_BROWSER=1
        JUCE_USE_CURL=0
)

# WebView UI Resources (Pattern #13: binary data for embedded HTML/JS/assets)
juce_add_binary_data(RedShiftDistortion_UIResources
    SOURCES
        Source/ui/public/index.html
        Source/ui/public/js/juce/index.js
        Source/ui/public/js/juce/check_native_interop.js
        Source/ui/public/assets/00fa754f6e134fb61f5e452b7ba8415e529dd82a.png
        Source/ui/public/assets/4287f6bace032d3fb347fc04a01722012ff802a1.png
        Source/ui/public/assets/f26475b6e81b4d3a1423f46d75b9ae5c2611416a.png
        Source/ui/public/assets/hise_knob_big.png
        Source/ui/public/assets/hise_knob_small.png
)

# Link UI resources to plugin
target_link_libraries(RedShiftDistortion
    PRIVATE
        RedShiftDistortion_UIResources
)
