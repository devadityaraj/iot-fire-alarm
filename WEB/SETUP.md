# Smart IoT Fire Safety Alarm - Setup Guide

## Overview

This is a real-time fire safety alarm monitoring dashboard built with Next.js, Firebase, and Web Audio API. It provides live sensor data, alert notifications with audio alarms, and location mapping.

## Prerequisites

- Firebase project with Realtime Database enabled
- Firebase Authentication (Email/Password) configured
- Node.js and pnpm installed

## Firebase Configuration

### 1. Get Your Firebase Credentials

1. Go to [Firebase Console](https://console.firebase.google.com)
2. Select your project
3. Click the settings icon → Project Settings
4. Under "Your apps", find your web app configuration
5. Copy these values:
   - API Key
   - Auth Domain
   - Project ID
   - Database URL
   - Storage Bucket
   - Messaging Sender ID
   - App ID

### 2. Add Credentials to .env.local

Edit `.env.development.local` and replace the placeholder values:

```env
NEXT_PUBLIC_FIREBASE_API_KEY=your_api_key_here
NEXT_PUBLIC_FIREBASE_AUTH_DOMAIN=your_auth_domain_here
NEXT_PUBLIC_FIREBASE_PROJECT_ID=your_project_id_here
NEXT_PUBLIC_FIREBASE_DATABASE_URL=your_database_url_here
NEXT_PUBLIC_FIREBASE_STORAGE_BUCKET=your_storage_bucket_here
NEXT_PUBLIC_FIREBASE_MESSAGING_SENDER_ID=your_messaging_sender_id_here
NEXT_PUBLIC_FIREBASE_APP_ID=your_app_id_here
```

### 3. Set Up Firebase Realtime Database

Create a `/device` path in your Realtime Database with this structure:

```json
{
  "device": {
    "alert": 0,
    "alerttype": "ok",
    "humidity": 85,
    "lastTrigger": "23:38-29072026",
    "lat": "40.7128",
    "lon": "-74.0060",
    "smoke": 256,
    "temperature": 24.5
  }
}
```

**Field Descriptions:**
- `alert`: 0 (safe) or 1 (active alert)
- `alerttype`: "ok", "fire", "smoke", "hightemp", or "reset"
- `humidity`: 0-100 (percentage)
- `lastTrigger`: Last alert timestamp "HH:MM-DDMMYYYY"
- `lat`: Latitude as string
- `lon`: Longitude as string
- `smoke`: Analog sensor reading (0-1024)
- `temperature`: Temperature in Celsius

### 4. Enable Firebase Authentication

1. In Firebase Console, go to Authentication
2. Enable Email/Password authentication method
3. Create a test user for development

## Running the App

```bash
# Install dependencies
pnpm install

# Start development server
pnpm dev

# Open browser to http://localhost:3000
```

## User Flow

1. **Login Page** (`/login`)
   - Email/password authentication
   - "Remember me" checkbox saves credentials locally
   - Auto-login on subsequent visits

2. **Dashboard** (`/`)
   - Protected route - redirects to login if not authenticated
   - **4-Panel Layout:**
     - **Top Left**: Alert Status Card - shows active alerts with status indicator
     - **Top Right**: Sensor Readings - temperature, humidity, smoke level, last trigger
     - **Bottom Left**: Location Map - embedded Google Maps showing device location
     - **Bottom Right**: Alert Controls - audio mute/unmute, volume control, logout

3. **Alert Features**
   - Real-time alert status from Firebase
   - Looping 1kHz audio alarm when `alert === 1`
   - Pulsing red animation during active alert
   - Mute/unmute controls with volume adjustment
   - Full-screen prompt on first dashboard load

## Features

- **Real-time Updates**: Live sync with Firebase Realtime Database via onValue listeners
- **Audio Alerts**: Web Audio API generates looping 1kHz tone when alert is active
- **Location Mapping**: Embedded Google Maps iframe showing device coordinates
- **Auto-login**: Credentials saved to localStorage for seamless return
- **Responsive Design**: Optimized for mobile, non-scrollable full-screen layout
- **Dark Theme**: Professional dark UI with red accent color for alerts

## Customization

### Theme Colors

Edit `app/globals.css` or use Tailwind classes:
- Background: `#0a0a0a`
- Dark Cards: `#1a1a1a`
- Borders: `#333333`
- Alert Red: `#dc2626`

### Alert Sounds

Modify `hooks/useAlertSound.ts`:
- Change frequency: `oscillator.frequency.value = 1000` (in Hz)
- Adjust volume: Modify `gainNode.gain.value = volume * 0.3`
- Volume multiplier: Change the `0.3` constant

### Sensor Display

Customize `components/SensorReadings.tsx` to add/remove sensors or change units.

## Environment Variables

All Firebase configuration uses `NEXT_PUBLIC_` prefix (safe for client-side):

```
NEXT_PUBLIC_FIREBASE_API_KEY
NEXT_PUBLIC_FIREBASE_AUTH_DOMAIN
NEXT_PUBLIC_FIREBASE_PROJECT_ID
NEXT_PUBLIC_FIREBASE_DATABASE_URL
NEXT_PUBLIC_FIREBASE_STORAGE_BUCKET
NEXT_PUBLIC_FIREBASE_MESSAGING_SENDER_ID
NEXT_PUBLIC_FIREBASE_APP_ID
```

## File Structure

```
app/
├── layout.tsx                  # Root layout with AuthProvider
├── page.tsx                    # Protected dashboard
└── login/
    └── page.tsx               # Login page

components/
├── ProtectedRoute.tsx         # Route protection wrapper
├── FullscreenPrompt.tsx       # Fullscreen entry prompt
├── AlertCard.tsx              # Alert status display
├── SensorReadings.tsx         # Sensor data cards
├── LocationMap.tsx            # Google Maps embed
└── AlertControls.tsx          # Audio controls & logout

context/
└── AuthContext.tsx            # Firebase auth context

hooks/
├── useAlertSound.ts           # Web Audio API hook
└── useFirebaseListener.ts     # Firebase realtime listener

lib/
├── firebase.ts                # Firebase initialization
└── auth.ts                    # Auth utilities

public/
└── [static assets]
```

## Browser Support

- Chrome/Edge 90+
- Firefox 88+
- Safari 14+
- Mobile browsers with Web Audio API support

## Troubleshooting

### Firebase Connection Error

1. Check `.env.local` has correct Firebase credentials
2. Verify Realtime Database URL format
3. Check Firebase project is active and has Realtime Database enabled
4. Ensure CORS is not blocking requests

### Audio Not Playing

1. Browser must allow audio (check permissions)
2. User interaction required (click after login)
3. Check browser console for Web Audio API errors
4. Verify `useAlertSound.ts` is properly initialized

### Maps Not Showing

1. Verify `lat` and `lon` are valid numbers in Firebase
2. Check embedded iframe is not blocked by CORS
3. Try opening the Google Maps link directly

### Auto-login Not Working

1. Check browser allows localStorage
2. Verify session hasn't expired in Firebase
3. Clear localStorage if credentials are stale: `localStorage.clear()`

## Security Notes

- Credentials stored in localStorage (non-production only)
- Firebase Rules should restrict `/device` read access
- For production: implement server-side session management
- Use HTTPS to protect auth tokens in transit

## Deployment

### To Vercel

```bash
# Install Vercel CLI
npm i -g vercel

# Deploy
vercel

# Add environment variables in Vercel Dashboard
# Settings → Environment Variables
```

### Production Considerations

1. Use production Firebase project
2. Implement proper RLS rules on Firebase
3. Move sensitive config to Vercel Environment Variables
4. Consider rate limiting for audio alerts
5. Implement proper error logging/monitoring

## License

MIT

## Support

For issues or questions, check:
- Firebase docs: https://firebase.google.com/docs
- Next.js docs: https://nextjs.org/docs
- Web Audio API: https://developer.mozilla.org/en-US/docs/Web/API/Web_Audio_API
