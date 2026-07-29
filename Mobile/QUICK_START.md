# Quick Start Guide - Smart IoT Fire Safety Alarm

## What's New

Your dashboard now has a complete professional alert system with:

### Alert Popup Modal
- Appears automatically when `alert = 1` in Firebase
- Shows 3 action buttons: Maps, Call, Okay
- Displays real-time sensor data
- Audio alert plays in loop

### Premium Dashboard Design
- Fixed viewport (no scrolling)
- 4 sensor cards with glassmorphism effect:
  - Alert Status (top-left)
  - Temperature (top-center-left, orange)
  - Humidity (top-center-right, cyan)
  - Smoke Level (top-right, gray)
- Bottom info bar with Maps and Call buttons

### New Components
```
/components/
├── AlertPopup.tsx          (Alert notification modal)
├── ResetPopup.tsx          (Reset confirmation - auto-closes)
└── SensorCard.tsx          (Individual sensor card)

/hooks/
├── useAlertAudio.ts        (Audio management)
└── useFirebaseListener.ts  (Enhanced with phone support)

/public/
└── alert-sound.wav         (10-second looping alarm)
```

## Firebase Data Expected

```json
{
  "device": {
    "alert": 0,                      // 0=ok, 1=alert
    "alerttype": "ok",               // ok/fire/smoke/reset/hightemp
    "temperature": 31.8,
    "humidity": 87,
    "smoke": 656,
    "lat": "40.7128",
    "lon": "-74.0060",
    "lastTrigger": "23:38-29072026",
    "phone": "+1234567890"
  }
}
```

## Testing the Alert System

### 1. Trigger an Alert
Set in Firebase:
```
alert: 1
alerttype: "fire"
```
Result: Popup appears, alarm sounds

### 2. Close the Alert Popup
Click "Close Alert" button
Result: Popup closes, alarm stops (visual alert continues)

### 3. Resolve the Alert
Set in Firebase:
```
alert: 0
alerttype: "ok"
```
Result: Alert clears, dashboard normalizes

### 4. Test Reset
Set in Firebase:
```
alerttype: "reset"
```
Result: Reset confirmation popup for 4 seconds

### 5. Test Maps
Set Firebase values:
```
lat: "40.7128"
lon: "-74.0060"
```
Click "📍 Maps" button
Result: Google Maps opens in new tab

### 6. Test Call
Set Firebase value:
```
phone: "+1234567890"
```
Click "📞 Call" button
Result: Phone dialer opens with number

## Key Features Explained

### Alert Popup Buttons

| Button | Action | Result |
|--------|--------|--------|
| 📍 Maps | Opens location in Google Maps | New tab opens |
| 📞 Call | Opens phone dialer | Phone app launches |
| 👍 Okay | Acknowledge alert | Popup closes, audio stops |
| Close Alert | Stop audio | Audio mutes, visual alert remains |

### Audio System

- **Starts**: When alert = 1 (auto-loop)
- **Stops**: When user closes popup OR alert = 0
- **Volume**: Can be controlled (default 0.7)
- **File**: `public/alert-sound.wav` (10 seconds)

### Dashboard Cards

Each sensor card features:
- Live value from Firebase
- Color-coded icon
- Unit label
- Glassmorphism hover effect
- Bottom accent bar

### Fullscreen Feature

- Popup on page load
- Requests fullscreen mode
- Can skip and use normal view
- Better experience for full-screen monitoring

## Environment Setup

Add to `.env.development.local`:
```
NEXT_PUBLIC_FIREBASE_API_KEY=your_key
NEXT_PUBLIC_FIREBASE_AUTH_DOMAIN=your_domain.firebaseapp.com
NEXT_PUBLIC_FIREBASE_PROJECT_ID=your_project
NEXT_PUBLIC_FIREBASE_DATABASE_URL=https://your_project.firebaseio.com
NEXT_PUBLIC_FIREBASE_STORAGE_BUCKET=your_project.appspot.com
NEXT_PUBLIC_FIREBASE_MESSAGING_SENDER_ID=your_sender_id
NEXT_PUBLIC_FIREBASE_APP_ID=your_app_id
```

## Running the App

```bash
# Install dependencies
pnpm install

# Start dev server
pnpm dev

# Open browser
# Navigate to http://localhost:3000
# Login with Firebase credentials
```

## Mobile Support

- Responsive design works on all sizes
- Phone call button uses native dialer
- Fullscreen works on mobile
- Touch-friendly buttons

## Production Deployment

1. Deploy to Vercel
2. Add Firebase credentials to Vercel project settings
3. Ensure `.env.development.local` is in `.gitignore`
4. Deployment ready!

## Troubleshooting

### Alert popup doesn't appear
- Check Firebase path is `device` not `devices`
- Verify `alert: 1` is set
- Check browser console for errors

### Audio doesn't play
- Verify `/public/alert-sound.wav` exists
- Check browser allows audio playback
- Try clicking "Okay" to close and reopen

### Maps button doesn't work
- Ensure `lat` and `lon` are strings with values
- Check coordinates are valid (lat: -90 to 90, lon: -180 to 180)

### Call button doesn't work
- Verify phone number includes country code
- Use format: `+1234567890`
- Mobile: Opens native dialer
- Desktop: Opens computer dialer (if available)

## Next Steps

1. Connect your Firebase project
2. Test with sample data
3. Configure alert triggers in your IoT device
4. Deploy to production
5. Monitor in real-time

For detailed information, see `ALERT_SYSTEM_GUIDE.md` and `IMPLEMENTATION_COMPLETE.md`
