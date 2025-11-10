cmake_minimum_required(VERSION 3.15)

# Plugin formats: VST3, AU, Standalone
juce_add_plugin(GainKnob
    COMPANY_NAME "YourCompany"
    PLUGIN_MANUFACTURER_CODE Manu
    PLUGIN_CODE Gain
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "GainKnob"
    NEEDS_WEB_BROWSER TRUE        # ← Enable WebView
)

# Embed UI files as binary data
juce_add_binary_data(GainKnobData
    SOURCES
        ui/public/index.html
        ui/public/js/juce.js
)

# Source files
target_sources(GainKnob
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
)

# Include paths
target_include_directories(GainKnob
    PRIVATE
        Source
)

# Required JUCE modules
target_link_libraries(GainKnob
    PRIVATE
        GainKnobData              # ← Link embedded UI files
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_plugin_client
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra      # ← Contains WebBrowserComponent
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# Compile definitions
target_compile_definitions(GainKnob
    PUBLIC
        JUCE_WEB_BROWSER=1        # ← Enable WebBrowserComponent
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)
