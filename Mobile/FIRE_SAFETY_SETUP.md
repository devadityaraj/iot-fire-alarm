# Fire Safety Department Portal - Complete Setup Guide

## System Overview

This is a professional Fire Safety Department control portal designed for real-time IoT monitoring of fire alarm systems. The dashboard displays live sensor data and provides emergency response capabilities.

## What's Been Built

### 1. Professional Dashboard Layout
- **3-Column Layout**: Optimized for fire department stations
- **Left Column**: Temperature, Humidity, Smoke Level sensors with progress bars
- **Middle Column**: Embedded Google Maps showing device location
- **Right Column**: System status, alert type, and quick action buttons
- **Full Desktop**: Fixed layout, no scrolling, optimized for 1920x1080+

### 2. Alert System
- **Popup Alert**: Displays immediately when alert status = 1
- **Alert Data**: Shows all sensor readings at moment of alert
- **Drive Button**: Opens Google Maps with device coordinates
- **Call Button**: Opens phone dialer with responder phone number
- **Audio Alert**: Looping 1000Hz alarm sound that plays during alert
- **Audio Stop**: Audio stops immediately when popup is closed

### 3. Features

#### Real-time Data Display
- Temperature (°C) with progress bar
- Humidity (%) with progress bar
- Smoke Level (PPM) with progress bar
- Alert status and type
- Last trigger timestamp
- Location coordinates

#### Emergency Actions
- Drive Location: Opens Google Maps at device coordinates
- Call Responder: Dials phone number from Firebase

#### User Interface
- Professional dark theme with red accents
- Glassmorphism effect on all cards
- Animated alerts with pulsing indicators
- Progress bars for sensor values
- Responsive status indicator

## Firebase Database Structure Required

Your Firebase Realtime Database must have this structure under `/device`:

```
device/
  ├── alert: 0 (0=inactive, 1=active)
  ├── alerttype: "ok" ("ok", "fire", "smoke", "hightemp", "reset")
  ├── temperature: 25.5 (Celsius)
  ├── humidity: 65 (Percent)
  ├── smoke: 450 (PPM)
  ├── lastTrigger: "14:30-30072026" (HH:MM-DDMMYYYY)
  ├── lat: "40.712776" (latitude as string)
  ├── lon: "-74.005974" (longitude as string)
  └── phone: "+1234567890" (responder phone number)
```

## Setup Instructions

### Step 1: Configure Firebase Credentials

Edit `.env.development.local` and add your Firebase credentials:

```
NEXT_PUBLIC_FIREBASE_API_KEY=your_api_key_here
NEXT_PUBLIC_FIREBASE_AUTH_DOMAIN=your_project.firebaseapp.com
NEXT_PUBLIC_FIREBASE_PROJECT_ID=your_project_id
NEXT_PUBLIC_FIREBASE_DATABASE_URL=https://your_project.firebaseio.com
NEXT_PUBLIC_FIREBASE_STORAGE_BUCKET=your_project.appspot.com
NEXT_PUBLIC_FIREBASE_MESSAGING_SENDER_ID=your_sender_id
NEXT_PUBLIC_FIREBASE_APP_ID=your_app_id
```

### Step 2: Set Up Firebase Authentication

1. Create a test user in Firebase Authentication
2. Use email/password authentication
3. Login with those credentials to access the dashboard

### Step 3: Configure Database Rules (Supabase/Realtime)

Make sure your database rules allow read access for authenticated users:

```json
{
  "rules": {
    "device": {
      ".read": "auth != null",
      ".write": false
    }
  }
}
```

### Step 4: Test the System

1. Start the dev server: `pnpm dev`
2. Navigate to http://localhost:3000
3. You'll see a fullscreen popup - click "Enter Fullscreen"
4. Login with your test credentials
5. Dashboard will show live data from Firebase

### Step 5: Test Alert Popup

1. In Firebase Console, change `/device/alert` from 0 to 1
2. The alert popup will appear automatically
3. Audio alarm will play (if browser allows autoplay)
4. Click "Drive Location" or "Call Responder" buttons
5. Click outside popup or ESC to close - audio stops

## Component Files

- `app/page.tsx` - Main dashboard (3-column layout)
- `components/AlertPopup.tsx` - Emergency alert modal
- `components/FullscreenPrompt.tsx` - Fullscreen request on load
- `hooks/useAlertAudio.ts` - Audio playback management
- `hooks/useFirebaseListener.ts` - Real-time Firebase listener

## Features Checklist

- [x] Professional fire department dashboard
- [x] 3-column layout with sensor cards
- [x] Embedded Google Maps
- [x] Alert popup with Drive and Call buttons
- [x] Looping audio alarm
- [x] Audio stops on popup close
- [x] Real-time Firebase data
- [x] Full-screen mode request
- [x] Login authentication
- [x] Mobile phone number support
- [x] No scrolling on desktop
- [x] Glassmorphism UI design

## Troubleshooting

### Firebase Error on Load
**Problem**: "Firebase: Error (auth/invalid-api-key)"
**Solution**: Add valid Firebase credentials to `.env.development.local`

### No Data Showing
**Problem**: Dashboard shows "Loading..." or "Connection Error"
**Solution**: 
1. Check Firebase credentials are correct
2. Verify database structure matches the required format
3. Check browser console for errors

### Alert Popup Not Appearing
**Problem**: Alert doesn't show when alert status changes
**Solution**:
1. Make sure `/device/alert` is set to 1 in Firebase
2. Check browser console for JavaScript errors
3. Refresh the page to reconnect

### Audio Not Playing
**Problem**: Alert sound doesn't play
**Solution**:
1. Check browser autoplay policy (some require user interaction)
2. Verify `/alert-sound.wav` exists in `/public` folder
3. Check browser volume is not muted

## Deployment

To deploy to Vercel:

```bash
git add .
git commit -m "Fire Safety Department Portal"
git push origin main
```

Then in Vercel Dashboard:
1. Add environment variables with Firebase credentials
2. Deploy will automatically build and start

## Phone Support

To enable call functionality:
1. Add a `phone` field to `/device` in Firebase
2. Use format: "+1234567890" or "1234567890"
3. Call button will appear when data is available

## Location Support

To enable Drive Location functionality:
1. Make sure `lat` and `lon` are set in `/device`
2. Use decimal format: "40.712776" and "-74.005974"
3. Drive button will be enabled when both are present
4. Clicking opens Google Maps at those coordinates

## Support

For issues or questions, refer to:
- Firebase Documentation: https://firebase.google.com/docs
- Next.js Documentation: https://nextjs.org/docs
- Tailwind CSS: https://tailwindcss.com/docs
