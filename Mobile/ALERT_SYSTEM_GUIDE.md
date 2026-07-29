# Smart IoT Fire Safety Alarm - Alert System Guide

## Complete Feature Implementation

### Alert Popup System
When `alert = 1` in Firebase:
1. **Alert Popup appears** with:
   - Large animated alert icon (🔥 for fire, 💨 for smoke, etc.)
   - Alert title and type
   - Current sensor readings (Temperature, Humidity, Smoke)
   - Location coordinates (if available)
   - Last trigger timestamp

2. **Action Buttons**:
   - **📍 Maps**: Opens Google Maps with device coordinates in new tab
   - **📞 Call**: Initiates phone call to number from `device/phone` Firebase path
   - **👍 Okay**: Acknowledges alert and closes popup (audio continues if alert still active)
   - **Close Alert**: Closes popup and stops audio

### Audio Alert System
- **File**: `public/alert-sound.wav` - 10-second looping alarm beep
- **Trigger**: Starts automatically when alert becomes active (alert = 1)
- **Behavior**: 
  - Plays in loop while popup is open
  - Stops when user clicks "Close Alert" button
  - **Visual-only mode**: Once popup is closed, alert remains visually active but audio stops
  - Resumes if alert reopens

### Reset Confirmation Popup
When `alerttype` changes to `"reset"`:
1. Auto-closing confirmation popup appears (4 seconds)
2. Shows "System Reset by Sensor" message
3. Displays checkmark and status info
4. Auto-closes after 4 seconds

### Dashboard Layout
**Premium Card-Based Design** (Fixed viewport, no scroll):
```
┌─────────────────────────────────────────────────────┐
│  Smart IoT Fire Safety | Real-time Monitoring      │
├──────────────┬──────────────┬──────────────┬────────┤
│              │              │              │        │
│ Alert Status │ Temperature  │  Humidity    │ Smoke  │
│   Card       │   Card       │   Card       │ Card   │
│              │              │              │        │
├──────────────────────────────────────────────────────┤
│                                                      │
│  Last Trigger | Status: NORMAL | Maps | Call       │
│                                                      │
└──────────────────────────────────────────────────────┘
```

### Sensor Cards (Apple Glassmorphism Design)
Each sensor displays:
- Icon (🌡️ 💧 💨)
- Current value with unit
- Color-coded (Orange=Temp, Cyan=Humidity, Gray=Smoke)
- Hover gradient effects
- Bottom accent bar

### Firebase Data Structure Required
```
/device
  ├── alert: 0 or 1
  ├── alerttype: "ok" | "fire" | "smoke" | "reset" | "hightemp"
  ├── temperature: 31.8
  ├── humidity: 87
  ├── smoke: 656
  ├── lat: "40.7128"
  ├── lon: "-74.0060"
  ├── lastTrigger: "23:38-29072026"
  └── phone: "+1234567890"
```

### Component Architecture

**useAlertAudio.ts** - Audio management hook
- `startAudio()` - Play alert sound
- `stopAudio()` - Stop alert sound
- `setVolume(0-1)` - Adjust volume
- `isPlaying` - Current audio state

**useFirebaseListener.ts** - Real-time data sync
- Fetches all device data from `/device` path
- Monitors phone number from `/device/phone`
- Auto-updates on Firebase changes

**AlertPopup.tsx** - Alert notification modal
- Shows when alert = 1
- Maps and Call button linking
- Sensor data display
- Dismissible but alert persists

**ResetPopup.tsx** - Reset confirmation
- Auto-closes after 4 seconds
- Triggered when alerttype = "reset"

**SensorCard.tsx** - Individual sensor display
- Reusable card component
- Customizable icon, label, unit, colors
- Glassmorphism with hover effects

**Page.tsx** - Main dashboard
- Orchestrates all components
- Manages alert state transitions
- Handles audio on/off logic
- Premium 4-column card grid layout

### Key Features
1. **Fixed Desktop Viewport** - No scrolling, entire dashboard on screen
2. **Fullscreen Popup** - Recommended fullscreen mode on load
3. **Premium Glassmorphism** - Frosted glass effect on all cards
4. **Real-time Updates** - Firebase listener updates all data instantly
5. **Multi-action Alerts** - Maps navigation, phone calling, alert acknowledgment
6. **Auto-reset Confirmation** - Special popup for system reset events
7. **Dark Theme** - Professional dark mode with red/orange accents
8. **Responsive Audio** - Loops during alert, stops on user action

### Testing the Alert System

1. **Trigger Alert**:
   ```
   Set in Firebase: alert = 1, alerttype = "fire"
   Result: Popup appears, audio plays, visual effects active
   ```

2. **Close Alert**:
   ```
   Click "Close Alert" button
   Result: Popup closes, audio stops, dashboard shows red pulsing
   ```

3. **Resolve Alert**:
   ```
   Set in Firebase: alert = 0, alerttype = "ok"
   Result: Popup auto-closes, audio stops, dashboard normalizes
   ```

4. **System Reset**:
   ```
   Set in Firebase: alerttype = "reset"
   Result: Reset confirmation popup appears for 4 seconds
   ```

5. **Phone Call**:
   ```
   Set phone number: device/phone = "+1234567890"
   Click 📞 Call button
   Result: Phone dialer opens with number pre-filled
   ```

6. **Location**:
   ```
   Set coordinates: lat = "40.7128", lon = "-74.0060"
   Click 📍 Maps button
   Result: Google Maps opens showing location
   ```

## Files Added/Modified

### New Files
- `/hooks/useAlertAudio.ts` - Alert sound management
- `/components/AlertPopup.tsx` - Alert notification modal
- `/components/ResetPopup.tsx` - Reset confirmation modal
- `/components/SensorCard.tsx` - Individual sensor card component
- `/public/alert-sound.wav` - Alert alarm sound file

### Modified Files
- `/app/page.tsx` - Complete dashboard redesign with alert logic
- `/hooks/useFirebaseListener.ts` - Added phone field support
- `/app/globals.css` - Premium glassmorphism utilities

### Configuration
- Firebase credentials in `.env.development.local`
- Fullscreen support in viewport settings
- Dark theme enabled by default

## Deployment Notes
- Audio file (`alert-sound.wav`) included in public folder
- No external API calls except Firebase
- Works offline after initial load (data cached in state)
- Fullscreen API requires user gesture (button click)
- Phone dialing uses native `tel:` protocol
- Google Maps opening uses public URL (no API key needed)
