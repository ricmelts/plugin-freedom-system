cmake_minimum_required(VERSION 3.15)

# Plugin formats: VST3, AU, Standalone
juce_add_plugin(RedShiftDistortion
    COMPANY_NAME "Plugin Freedom"
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

# WebView UI Resources (will be uncommented by gui-agent in Stage 3)
# juce_add_binary_data(RedShiftDistortion_UIResources
#     SOURCES
#         Source/ui/public/index.html
#         Source/ui/public/js/juce/index.js
#         Source/ui/public/js/juce/check_native_interop.js
# )
#
# target_link_libraries(RedShiftDistortion
#     PRIVATE
#         RedShiftDistortion_UIResources
# )
