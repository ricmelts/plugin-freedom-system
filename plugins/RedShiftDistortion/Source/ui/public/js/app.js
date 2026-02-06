// RedShiftDistortion - Parameter Bindings
import { getSliderState, getToggleState } from './juce/index.js';

// Parameter configurations
const paramConfigs = {
    saturation: {
        type: 'slider',
        min: -12.0,
        max: 24.0,
        unit: 'dB',
        decimals: 1
    },
    dopplerShift: {
        type: 'slider',
        min: -50.0,
        max: 50.0,
        unit: '%',
        decimals: 1
    },
    delayTime: {
        type: 'slider',
        min: 0.0,
        max: 2500.0,
        unit: 'ms',
        decimals: 1
    },
    feedback: {
        type: 'slider',
        min: 0.0,
        max: 95.0,
        unit: '%',
        decimals: 1
    },
    distortionLevel: {
        type: 'slider',
        min: -60.0,
        max: 0.0,
        unit: 'dB',
        decimals: 1
    },
    masterOutput: {
        type: 'slider',
        min: -60.0,
        max: 12.0,
        unit: 'dB',
        decimals: 1
    },
    pitchEnable: {
        type: 'toggle'
    },
    tempoSync: {
        type: 'toggle'
    }
};

// Knob rotation range (degrees)
const KNOB_MIN_ANGLE = -135;
const KNOB_MAX_ANGLE = 135;
const KNOB_RANGE = KNOB_MAX_ANGLE - KNOB_MIN_ANGLE;

// Initialize when DOM loads
document.addEventListener('DOMContentLoaded', () => {
    console.log('RedShiftDistortion UI initializing...');

    // Bind all parameters
    for (const [paramId, config] of Object.entries(paramConfigs)) {
        if (config.type === 'slider') {
            bindSliderParameter(paramId, config);
        } else if (config.type === 'toggle') {
            bindToggleParameter(paramId, config);
        }
    }

    console.log('RedShiftDistortion UI initialized');
});

/**
 * Bind slider parameter (knob control)
 */
function bindSliderParameter(paramId, config) {
    const knobElement = document.getElementById(paramId);
    const valueDisplay = document.getElementById(`${paramId}-value`);
    const indicator = knobElement.querySelector('.knob-indicator');

    if (!knobElement || !valueDisplay || !indicator) {
        console.error(`Slider elements not found for parameter: ${paramId}`);
        return;
    }

    // Get parameter state from JUCE
    const paramState = getSliderState(paramId);

    if (!paramState) {
        console.error(`Failed to get slider state for: ${paramId}`);
        return;
    }

    // Helper: Convert normalized value (0-1) to parameter value
    const normalizedToValue = (normalized) => {
        return config.min + normalized * (config.max - config.min);
    };

    // Helper: Convert parameter value to normalized (0-1)
    const valueToNormalized = (value) => {
        return (value - config.min) / (config.max - config.min);
    };

    // Helper: Update knob visual
    const updateKnobVisual = (normalized) => {
        const angle = KNOB_MIN_ANGLE + normalized * KNOB_RANGE;
        indicator.style.transform = `translateX(-50%) rotate(${angle}deg)`;

        const value = normalizedToValue(normalized);
        valueDisplay.textContent = `${value.toFixed(config.decimals)} ${config.unit}`;
    };

    // Initialize knob position from current parameter value
    const initialValue = paramState.getNormalisedValue();
    updateKnobVisual(initialValue);

    // Listen for parameter changes from DAW (automation, preset load)
    paramState.valueChangedEvent.addListener(() => {
        const value = paramState.getNormalisedValue();
        updateKnobVisual(value);
    });

    // Mouse drag interaction (relative drag pattern)
    let isDragging = false;
    let lastY = 0;

    knobElement.addEventListener('mousedown', (e) => {
        isDragging = true;
        lastY = e.clientY;
        knobElement.style.cursor = 'grabbing';
        e.preventDefault();
    });

    document.addEventListener('mousemove', (e) => {
        if (!isDragging) return;

        // Calculate frame-to-frame delta (relative drag)
        const deltaY = lastY - e.clientY;
        lastY = e.clientY;

        // Get current normalized value
        let normalized = paramState.getNormalisedValue();

        // Apply delta (sensitivity: 0.003 per pixel)
        const sensitivity = 0.003;
        normalized += deltaY * sensitivity;

        // Clamp to 0-1 range
        normalized = Math.max(0, Math.min(1, normalized));

        // Update parameter state
        paramState.setNormalisedValue(normalized);

        // Update visual (listener will also fire, but direct update feels more responsive)
        updateKnobVisual(normalized);

        e.preventDefault();
    });

    document.addEventListener('mouseup', () => {
        if (isDragging) {
            isDragging = false;
            knobElement.style.cursor = 'pointer';
        }
    });
}

/**
 * Bind toggle parameter (switch control)
 */
function bindToggleParameter(paramId, config) {
    const toggleElement = document.getElementById(paramId);

    if (!toggleElement) {
        console.error(`Toggle element not found for parameter: ${paramId}`);
        return;
    }

    // Get parameter state from JUCE
    const paramState = getToggleState(paramId);

    if (!paramState) {
        console.error(`Failed to get toggle state for: ${paramId}`);
        return;
    }

    // Helper: Update toggle visual
    const updateToggleVisual = (isActive) => {
        if (isActive) {
            toggleElement.classList.add('active');
        } else {
            toggleElement.classList.remove('active');
        }
    };

    // Initialize toggle state from current parameter value
    const initialValue = paramState.getValue();
    updateToggleVisual(initialValue);

    // Listen for parameter changes from DAW (automation, preset load)
    paramState.valueChangedEvent.addListener(() => {
        const value = paramState.getValue();
        updateToggleVisual(value);
    });

    // Click interaction
    toggleElement.addEventListener('click', () => {
        const currentValue = paramState.getValue();
        const newValue = !currentValue;

        // Update parameter state
        paramState.setValue(newValue);

        // Update visual (listener will also fire, but direct update feels more responsive)
        updateToggleVisual(newValue);
    });
}
