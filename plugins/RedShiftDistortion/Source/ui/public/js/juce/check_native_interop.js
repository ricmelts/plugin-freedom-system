// Check if JUCE native backend is available
(function() {
  'use strict';

  // JUCE injects __JUCE__ global when running in native context
  if (typeof window.__JUCE__ === 'undefined') {
    console.warn('JUCE backend not detected - running in browser mode');
    window.__JUCE__ = null;
  } else {
    console.log('JUCE native backend detected');
  }
})();
