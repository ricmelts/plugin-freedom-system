cmake_minimum_required(VERSION 3.15)

# Plugin formats: VST3, AU, Standalone
juce_add_plugin(RedShiftDistortion
    COMPANY_NAME "FreedomAudio"
    PLUGIN_MANUFACTURER_CODE FrAu
    PLUGIN_CODE RsDt
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "RedShiftDistortion"
    NEEDS_WEB_BROWSER TRUE  # CRITICAL for VST3 WebView support
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

# Generate JuceHeader.h (JUCE 8 requirement - MUST come after target_link_libraries)
juce_generate_juce_header(RedShiftDistortion)

# WebView UI Resources (uncommented by gui-agent if WebView UI used)
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

# Compile definitions
target_compile_definitions(RedShiftDistortion
    PUBLIC
        JUCE_VST3_CAN_REPLACE_VST2=0
)

# WebView support (uncommented by gui-agent if WebView UI used)
# target_compile_definitions(RedShiftDistortion
#     PUBLIC
#         JUCE_WEB_BROWSER=1
#         JUCE_USE_CURL=0
# )
