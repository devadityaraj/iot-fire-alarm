# Deployment Guide

## Local Development

The build error about Firebase credentials is expected and normal. During development:

```bash
# Firebase initialization happens at runtime, not build time
pnpm dev  # Development server works fine with invalid credentials
```

The dev server will show Firebase errors only if Firebase config is missing, but this doesn't prevent the app from loading.

## Build Output

To skip Firebase validation during build (recommended for development):

```bash
# Option 1: Add dummy Firebase credentials to .env.development.local
NEXT_PUBLIC_FIREBASE_API_KEY=dummy
NEXT_PUBLIC_FIREBASE_DATABASE_URL=https://dummy.firebaseio.com

# Option 2: Use environment-specific builds
pnpm build  # Production build - requires valid credentials
pnpm dev    # Development - works with any credentials
```

## Vercel Deployment

### 1. Push to GitHub

```bash
git init
git add .
git commit -m "Initial commit"
git remote add origin https://github.com/username/repo.git
git push -u origin main
```

### 2. Connect to Vercel

1. Go to https://vercel.com/new
2. Select your GitHub repository
3. Configure build settings:
   - Framework: Next.js
   - Build Command: `pnpm build`
   - Output Directory: `.next`

### 3. Add Environment Variables in Vercel

In Vercel Dashboard → Settings → Environment Variables, add:

```
NEXT_PUBLIC_FIREBASE_API_KEY=your_actual_key
NEXT_PUBLIC_FIREBASE_AUTH_DOMAIN=your_actual_domain
NEXT_PUBLIC_FIREBASE_PROJECT_ID=your_actual_project_id
NEXT_PUBLIC_FIREBASE_DATABASE_URL=your_actual_database_url
NEXT_PUBLIC_FIREBASE_STORAGE_BUCKET=your_actual_storage_bucket
NEXT_PUBLIC_FIREBASE_MESSAGING_SENDER_ID=your_actual_sender_id
NEXT_PUBLIC_FIREBASE_APP_ID=your_actual_app_id
```

### 4. Deploy

Click "Deploy" - Vercel will:
1. Build the app with valid Firebase credentials
2. Deploy to CDN
3. Provide you with a production URL

## Production Checklist

- [ ] Firebase credentials added to Vercel env vars
- [ ] Firebase Realtime Database has proper security rules
- [ ] Firebase Authentication is configured
- [ ] Test user account created
- [ ] SSL certificate enabled (automatic on Vercel)
- [ ] Custom domain configured (optional)
- [ ] Monitoring/logging configured
- [ ] Backup strategy in place

## Firebase Security Rules

Secure your Realtime Database with these rules:

```json
{
  "rules": {
    "device": {
      ".read": "auth != null",
      ".write": "root.child('admin').child(auth.uid).val() === true"
    }
  }
}
```

This allows authenticated users to read device data, but only admins can write.

## Troubleshooting Deployment

### Build Fails with "Invalid API Key"

**Solution:** Add valid Firebase credentials to `.env.development.local` before building, or disable static export:

```js
// next.config.js
const nextConfig = {
  skipMiddlewareValidation: true,
  // Remove export option if present
}
```

### Firebase Connection Fails in Production

**Check:**
1. Environment variables are set in Vercel
2. Database URL is correct (includes `.firebaseio.com`)
3. Firebase project is active in console
4. Security rules allow the operation

### Audio Not Working

**Ensure:**
1. HTTPS is enabled (required for Web Audio API)
2. Browser allows audio autoplay with user interaction
3. Audio context is initialized after user gesture

### Maps Not Displaying

**Verify:**
1. Coordinates are valid numbers in database
2. Google Maps embed URL is correct
3. CORS is not blocking iframe

## Performance Optimization

### Code Splitting

Components are automatically code-split by Next.js. No additional configuration needed.

### Image Optimization

All images are served by Next.js Image component (if used).

### Monitoring

Consider adding:
- Vercel Analytics
- Firebase Realtime Database usage monitoring
- Error tracking (Sentry)

## Scaling

For high-volume alerts:

1. **Database:** Firebase Realtime scales to thousands of concurrent connections
2. **Concurrent Users:** Vercel serverless functions scale automatically
3. **Bandwidth:** Optimize data payload in Firebase

Consider adding:
- Message queuing (Firestore, Cloud Pub/Sub)
- Rate limiting on client
- Batched updates

## Maintenance

### Regular Tasks

- [ ] Monitor Firebase usage and costs
- [ ] Update dependencies monthly: `pnpm update`
- [ ] Review Firebase security rules quarterly
- [ ] Check Vercel deployment logs for errors
- [ ] Test backup/restore procedures

### Update Dependencies

```bash
pnpm update              # Update all packages
pnpm update firebase     # Update specific package
pnpm audit              # Check for vulnerabilities
```

## Support

For issues:
- Next.js: https://nextjs.org/docs
- Firebase: https://firebase.google.com/docs
- Vercel: https://vercel.com/docs
- Web Audio: https://developer.mozilla.org/en-US/docs/Web/API/Web_Audio_API
