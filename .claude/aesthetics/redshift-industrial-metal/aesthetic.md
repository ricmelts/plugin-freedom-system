# RedShift Industrial Metal

> **Vibe:** Bold industrial metal aesthetic with vibrant red accents and diamond plate textures
>
> **Source:** Created from RedShiftDistortion mockup v6
>
> **Best For:** Distortion effects, tape delays, vintage hardware emulations, aggressive processing plugins

---

## Visual Identity

Industrial metal aesthetic featuring diamond plate textured backgrounds with bright red (#F21A1D) accents against dark surfaces. Gray rotary knobs with subtle metallic sheen, red corner screws, and dual stereo VU meters create a rugged, hardware-inspired interface. Black text with red glow effects reinforces the "Red Shift" theme throughout. Creates a bold, professional atmosphere suitable for creative distortion, delay, and saturation effects where visual impact matches sonic aggression.

---

## Color System

### Primary Palette

**Background Colors:**
- Main background: Diamond plate texture image providing industrial metal surface
- Surface/panel background: Black (#000000) for contrast areas and LED backgrounds
- Elevated surfaces: Dark brown-red (#1a0a00) for VU meter backgrounds

**Accent Colors:**
- Primary accent: Bright red (#F21A1D) for active elements, LEDs, VU meters, and highlights
- Secondary accent: Dark red (#bd0000) for glows and shadows
- Hover/active state: Brighter red variations with increased opacity for interaction feedback

**Text Colors:**
- Primary text: Black (#000000) with red text-shadow glow for main labels
- Secondary/muted text: Same black with softer red glow for smaller labels
- Labels: Handjet font in black with red glow (0px -7px 50px #bd0000) creating distinctive halo effect

### Control Colors

**Knobs/Rotary Controls:**
- Base color: Light gray (#D9D9D9) knob images with subtle transparency (50% opacity)
- Pointer/indicator: Dark gray (#333) indicator line or integrated into knob image
- Active state: Red glow beneath main knob creates focal point (radial-gradient with rgba(189, 0, 0, 0.5-0.75))

**Sliders:**
- Track color: Semi-transparent black (rgba(0, 0, 0, 0.3)) for embedded appearance
- Thumb color: Light-to-dark gray gradient (#D9D9D9 to #888986)
- Fill color: Not used (simple track design)

**Buttons/Toggles:**
- Default state: Light-to-dark gray gradient (#D9D9D9 to #888986) with red border (rgba(242, 26, 29, 0.5))
- Active/on state: Bright red gradient (#F21A1D to #bd0000) with solid red border and LED glow
- Hover state: Border intensifies to rgba(242, 26, 29, 0.8)

### Philosophy

Bold monochromatic base (blacks and grays) with aggressive red accent creates high-impact visual language. Diamond plate texture grounds the aesthetic in industrial hardware while bright red provides immediate visual focus points. High contrast between dark backgrounds and red accents ensures instant readability. Red glow effects throughout create cohesive "Red Shift" branding theme. Overall mood: industrial, powerful, professional with creative edge.

---

## Typography

**Font Families:**
- Headings: Jacquard 12 (bold decorative sans-serif) for plugin title
- Body text: Handjet (technical sans-serif, 300-400 weight) for all labels and UI text
- Values/numbers: Courier New monospace for debug/technical displays

**Font Sizing:**
- Plugin title: Extra large (60px) for bold presence at top of interface
- Section labels: Large (30px) for subtitle and main knob label
- Parameter labels: Medium (26-28px) for sub-knob labels maintaining readability
- Parameter values: Small to medium (14-18px) for bypass labels and advanced settings

**Font Styling:**
- Title weight: 900 (extra bold) with uppercase transform and letter-spacing for impact
- Label weight: 300-400 (light to normal) with uppercase transform for technical consistency
- Letter spacing: Moderate (2px on title) for expanded, industrial feel
- Text transform: Uppercase for all labels maintaining technical/professional aesthetic

**Philosophy:**
Technical/industrial approach with distinctive decorative title font (Jacquard 12) contrasting with utilitarian Handjet labels. Strong hierarchy through size variation. Red text-shadow glow unifies all text elements with aesthetic theme. Optimized for at-a-glance parameter identification while maintaining bold visual character.

---

## Controls

### Knob Style

**Visual Design:**
- Shape: Circle for all rotary controls
- Size: Large (168px) for main doppler knob, medium (130px) for six surrounding parameter knobs
- Indicator style: Dark indicator bar integrated into knob image or separate indicator element
- Border treatment: No explicit border on knobs themselves (knob image provides edge definition)

**Surface Treatment:**
- Base appearance: Photorealistic gray knob image (f26475b6e81b4d3a1423f46d75b9ae5c2611416a.png) with metallic appearance
- Depth: Main knob has dramatic red glow (dual-layer radial gradients with blur) creating elevation
- Tick marks: Not present on knob perimeter (clean circular design)
- Center indicator: Subtle indicator bar/line showing rotation position

**Interaction Feel:**
Tactile, hardware-like interaction mimicking physical knobs. Main doppler knob features red glow halo creating visual focus and "hot" appearance reinforcing the effect name. Smooth continuous rotation with visual indicator feedback. Surrounding knobs maintain consistent metallic appearance creating unified control surface.

### Slider Style

**Layout:**
- Orientation preference: Horizontal for advanced settings (grain size)
- Dimensions: Thin track (23px height, 90px width) for compact integration
- Track design: Semi-transparent black fill with solid black border (embedded appearance)

**Thumb Design:**
- Shape: Small rectangle with subtle rounding (2px border-radius)
- Size: 15px wide × 19px tall relative to 23px track height
- Style: Light-to-dark gray gradient matching button aesthetic

**Visual Treatment:**
- Track appearance: Dark semi-transparent (rgba(0, 0, 0, 0.3)) with 1px solid black border
- Fill behavior: Not used (thumb position indicates value directly)
- Scale marks: None (simple slider design for secondary controls)

### Button Style

**Shape & Size:**
- Shape: Circle (border-radius: 50%) for bypass toggle buttons
- Padding: Minimal (29px diameter buttons are compact)
- Aspect ratio: 1:1 square aspect (circular)

**Visual Design:**
- Default state: Light-to-dark gray gradient (#D9D9D9 to #888986) with 3px red border (rgba(242, 26, 29, 0.5))
- Active state: Bright red gradient (#F21A1D to #bd0000) with solid red border and center LED illumination
- Border: 3px solid with red color, semi-transparent when off, opaque when active

**Typography:**
- Label style: Normal weight Handjet, uppercase, positioned beside button (not on button)
- Icon treatment: LED indicator (12px circle) centered within button, red glow when active

---

## Spacing & Layout Philosophy

**Overall Density:**
Comfortable spacing creating readable, uncluttered interface with distinctive radial layout. Main doppler knob centrally positioned with six surrounding knobs arranged in clock positions (10, 12, 2, 4, 6, 8 o'clock). Each control has adequate space for labels without overlap.

**Control Spacing:**
- Between controls: Radial arrangement maintains visual balance, controls positioned to avoid overlap with 130-168px knob sizes
- Vertical rhythm: Bypass buttons stack vertically with 55px gaps for clear separation
- Grouping strategy: Related controls share radial arc (top arc: width/feedback/master, bottom arc: filters/saturation)

**Padding & Margins:**
- Edge margins: Comfortable (20-25px) from window edges for corner screw placement
- Section padding: Advanced settings clustered in bottom-right with consistent label-control alignment
- Label-to-control gap: Tight to moderate (varies by control - labels positioned for optimal readability)

**Layout Flexibility:**
Radial layout is distinctive for 6-7 parameter plugins with prominent center control. For different parameter counts, adapt to grid or horizontal layout while maintaining red accent theme and spacing philosophy. Radial arrangement creates visual focus on center parameter.

---

## Surface Treatment

### Textures

**Background:**
- Texture type: Diamond plate metal texture (photorealistic asset: 00fa754f6e134fb61f5e452b7ba8415e529dd82a.png)
- Intensity: Prominent - texture provides primary background visual interest
- Implementation: Full-size background-image with cover sizing

**Control Surfaces:**
- Control texture: Photorealistic knob image provides metallic texture detail
- Consistency: Textured background with photorealistic controls creates unified industrial aesthetic

### Depth & Dimensionality

**Shadow Strategy:**
- Shadow presence: Dramatic on plugin frame (0 10px 40px rgba(0,0,0,0.5)) in test HTML, subtle elsewhere
- Shadow color: Black with medium opacity for drop shadows, red for glows
- Shadow blur: Dramatic (20px blur on outer glow, 10px on inner glow for main knob)
- Typical shadow values: 0 0 10px #F21A1D for red LED glow effects

**Elevation System:**
- Layers: Subtle elevation on main knob via red glow creates focal point
- How elevation is shown: Glowing halos and LED effects rather than traditional drop shadows

**Borders:**
- Border presence: Selective - bypass buttons have prominent borders, controls have minimal borders
- Border style: Solid lines with 1-3px width depending on element
- Border color approach: Red borders (semi-transparent when inactive) integrate with accent palette

---

## Details & Embellishments

**Special Features:**
- Red corner screws (38×38px image assets: 4287f6bace032d3fb347fc04a01722012ff802a1.png) at all four corners reinforcing industrial theme
- Dual stereo VU meters (L+R, 40×350px each) with animated red LED bars providing dynamic visual feedback
- Main knob red glow effect (dual-layer radial gradients) creating distinctive focal point
- Text shadow glows (0px -7px 50px #bd0000) unifying all labels with red accent theme

**Active State Feedback:**
- Bypass buttons transform completely: gradient shifts from gray to red, border becomes opaque, center LED illuminates with glow
- VU meters animate continuously with red fill bars indicating audio levels

**Hover States:**
- Bypass buttons: Border color intensifies from rgba(242, 26, 29, 0.5) to rgba(242, 26, 29, 0.8)
- Knobs: Cursor changes to pointer indicating interactivity

**Focus Indicators:**
- Not explicitly defined (WebView context limits keyboard navigation)

**Decorative Elements:**
- Diamond plate background texture creating industrial foundation
- Red metal corner screws as decorative fasteners
- "RED SHIFT" title with Jacquard 12 font and red glow
- "digital delay" subtitle reinforcing effect category

---

## Technical Patterns

**CSS Patterns:**
- Border radius: Prominent (50%) for circular controls, subtle (2-5px) for rectangular elements
- Transition speed: Fast (50-100ms) for knob rotation and button states maintaining responsive feel
- Easing: Not explicitly defined (browser defaults)

**Layout Techniques:**
- Preferred layout: Absolute positioning for radial knob arrangement and precise control placement
- Responsive strategy: Fixed 950×700px window size (non-resizable)
- Alignment: Center-focused radial layout with absolute pixel positioning

**Performance Considerations:**
- Background texture image (diamond plate) is static
- Photorealistic knob images rotate via CSS transform (efficient)
- VU meter animation via simple height property changes (lightweight)
- Red glow effects use CSS gradients and filters (no heavy computation)

---

## Interaction Feel

**Responsiveness:**
Controls react immediately with fast transitions (50ms for knobs). VU meters animate at 100ms intervals creating dynamic feedback without lag.

**Feedback:**
Visual feedback is bold and immediate. Bypass buttons transform dramatically on toggle. Main knob glows red creating "hot" appearance. VU meters provide continuous animated feedback. Red glow effects throughout create cohesive active-state language.

**Tactility:**
Feels like physical hardware - photorealistic knob images and diamond plate texture reinforce tangible industrial quality. Circular bypass buttons with LED indicators mimic real hardware toggles. Radial knob arrangement creates focused interaction surface.

**Overall UX Personality:**
Professional industrial with creative edge. Bold red accents prevent aesthetic from feeling purely utilitarian. Balance between rugged hardware inspiration and modern digital polish. Suitable for creative distortion/delay effects where visual impact matches sonic character.

---

## Best Suited For

**Plugin Types:**
Distortion effects, tape delays, saturation processors, vintage hardware emulations, aggressive creative effects, industrial-themed plugins. Works well for effects where visual boldness matches sonic impact.

**Design Contexts:**
Portfolio/showcase work where strong visual identity is valuable. Creative effect plugins targeting modern producers. Plugins where red/industrial branding aligns with sonic character. Projects where distinctive aesthetic differentiates from competitors.

**Not Recommended For:**
Subtle/transparent utility plugins (compressors, limiters, clean EQs) where red accents may feel too aggressive. Vintage/retro plugins requiring warm brown/orange palettes. Minimal/clinical plugins requiring neutral aesthetics. Plugins with many parameters (radial layout doesn't scale beyond 7-8 knobs).

---

## Application Guidelines

### When Applying to New Plugin

**Parameter Count Adaptation:**
- **1-3 parameters:** Single horizontal row with large controls. Consider vertical slider layout. Red accents on active parameter.
- **4-6 parameters:** Small 2×3 or 3×2 grid maintaining red accent theme. Prominent parameter (mix/output) as larger control on right.
- **7-9 parameters:** Consider radial layout (if one parameter is primary) or 3×3 grid. Maintain diamond plate background and red accents.
- **10+ parameters:** Switch to grid layout (radial doesn't scale well). Section parameters logically. Maintain red accent strategy but adapt layout to grid.

**Control Type Mapping:**
- Float parameters → Rotary knobs (default, using gray knob image asset) with potential red glow for primary parameter
- Boolean parameters → Circular toggle buttons with red LED indicators matching bypass button style
- Choice parameters → Dropdown (advanced settings style) or horizontal button group with red active state

**Prominent Parameter Handling:**
Primary parameter should receive red glow treatment (radial gradients beneath knob) creating visual focus. Consider larger size (168px vs 130px for secondary controls). Red glow effect signals "this is the main creative control."

### Customization Points

**Easy to Adjust:**
- Red accent hue (shift from #F21A1D to other reds/oranges while maintaining vibrance)
- Glow intensity (adjust opacity of radial gradients)
- Control sizes (within 20% maintains aesthetic character)
- Spacing values (while maintaining comfortable density)
- VU meter dimensions and placement

**Core Identity Elements:**
- Diamond plate background texture (defines industrial character)
- Bright red accent color (#F21A1D or similar vibrant red)
- Black text with red glow effects (distinctive labeling style)
- Photorealistic gray knob images (hardware feel)
- Red corner screws (decorative industrial detail)
- Jacquard 12 title font + Handjet label font combination

### Integration Notes

WebView implementation standard. Diamond plate texture image (00fa754f6e134fb61f5e452b7ba8415e529dd82a.png) and knob image (f26475b6e81b4d3a1423f46d75b9ae5c2611416a.png) must be included in binary resources. Red corner screw image (4287f6bace032d3fb347fc04a01722012ff802a1.png) required if using decorative corners. Fonts imported via Google Fonts (@import). No special browser requirements - standard CSS. Performance is good (static textures, efficient transforms).

---

## Example Color Codes

[Provide concrete color values as reference examples, even though prose descriptions above]

```css
/* Backgrounds */
--bg-main: Diamond plate texture image (00fa754f6e134fb61f5e452b7ba8415e529dd82a.png);
--bg-surface: #000000;
--bg-vu-meter: #1a0a00;

/* Accents */
--accent-primary: #F21A1D;
--accent-hover: #F21A1D at 80% opacity;
--accent-glow: #bd0000;

/* Text */
--text-primary: #000000;
--text-glow: #bd0000;

/* Controls */
--control-base: #D9D9D9;
--control-gradient-end: #888986;
--control-active: #F21A1D;
--control-border-inactive: rgba(242, 26, 29, 0.5);
--control-border-active: rgba(242, 26, 29, 1.0);

/* Glows */
--glow-outer: rgba(189, 0, 0, 0.5);
--glow-inner: rgba(189, 0, 0, 0.75);
--led-glow: rgba(242, 26, 29, 0.3) inactive, #F21A1D active;
```

---

## Implementation Checklist

When applying this aesthetic to a new plugin:

- [ ] Extract core color palette and define CSS variables
- [ ] Import Jacquard 12 and Handjet fonts via Google Fonts
- [ ] Apply typography hierarchy (60px title, 26-30px labels, uppercase transforms)
- [ ] Include diamond plate background texture image in binary resources
- [ ] Include gray knob image asset in binary resources (or generate CSS alternative)
- [ ] Style each control type: rotary knobs with red glow (primary), gray knobs (secondary), circular toggles with LED indicators
- [ ] Implement red text-shadow glow on all labels (0px -7px 50px #bd0000)
- [ ] Add corner screw decorative elements if using (optional but reinforces theme)
- [ ] Apply interaction states (hover intensifies borders, active toggles transform to red gradient)
- [ ] Consider radial layout for 6-8 parameters with prominent center control, grid for others
- [ ] Verify visual consistency: red accents, industrial texture, black/gray base with red highlights
- [ ] Validate WebView constraints (no viewport units, fixed 950×700px or appropriate size)
- [ ] Test in both Debug and Release builds
- [ ] Verify all binary resources (textures, knob images, corner screws) are correctly embedded
