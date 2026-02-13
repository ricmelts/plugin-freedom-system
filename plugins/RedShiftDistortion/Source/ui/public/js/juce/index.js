// JUCE WebView parameter binding helpers
(function() {
  'use strict';

  // Only run if JUCE backend is available
  if (!window.__JUCE__) {
    return;
  }

  // Create backend API wrapper
  window.__JUCE__.backend = {
    // Get slider value from JUCE parameter
    getSliderValue: function(paramId) {
      if (window.__JUCE__.getSliderState) {
        const state = window.__JUCE__.getSliderState(paramId);
        return state ? state.value : 0;
      }
      return 0;
    },

    // Set slider value to JUCE parameter
    setSliderValue: function(paramId, value) {
      if (window.__JUCE__.setSliderValue) {
        window.__JUCE__.setSliderValue(paramId, value);
      }
    },

    // Listen for parameter changes from JUCE (automation, presets, etc.)
    addEventListener: function(paramId, callback) {
      if (window.__JUCE__.addEventListener) {
        window.__JUCE__.addEventListener(paramId, callback);
      }
    }
  };

  console.log('JUCE backend API initialized');
})();
