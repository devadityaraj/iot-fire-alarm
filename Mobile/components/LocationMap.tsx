'use client'

import { DeviceData } from '@/hooks/useFirebaseListener'

interface LocationMapProps {
  data: DeviceData | null
}

export const LocationMap = ({ data }: LocationMapProps) => {
  const lat = data?.lat ? parseFloat(data.lat) : null
  const lon = data?.lon ? parseFloat(data.lon) : null
  const hasLocation = lat && lon && !isNaN(lat) && !isNaN(lon)

  const generateMapsLink = () => {
    if (!hasLocation) return '#'
    return `https://www.google.com/maps?q=${lat},${lon}`
  }

  const getMapsEmbedUrl = () => {
    if (!hasLocation) return ''
    return `https://maps.google.com/maps?q=${lat},${lon}&z=15&output=embed`
  }

  return (
    <div className="glass-lg overflow-hidden h-full flex flex-col group">
      {hasLocation ? (
        <>
          {/* Map Container */}
          <div className="flex-1 relative bg-gradient-to-br from-slate-900 to-slate-950 overflow-hidden">
            <div className="absolute inset-0 bg-gradient-to-br from-blue-600/10 to-cyan-600/10 pointer-events-none"></div>
            <iframe
              width="100%"
              height="100%"
              style={{ border: 0 }}
              loading="lazy"
              allowFullScreen
              src={getMapsEmbedUrl()}
              title="Alert Location Map"
            ></iframe>
          </div>

          {/* Location Info Footer */}
          <div className="border-t border-white/[0.1] bg-gradient-to-r from-white/[0.05] to-white/[0.02] p-4 backdrop-blur-sm">
            <p className="text-xs font-medium text-gray-400 mb-3">
              📍 {lat?.toFixed(4)}°, {lon?.toFixed(4)}°
            </p>
            <a
              href={generateMapsLink()}
              target="_blank"
              rel="noopener noreferrer"
              className="inline-flex items-center gap-2 text-xs font-semibold px-3 py-1.5 rounded-lg bg-red-600/20 text-red-400 hover:bg-red-600/40 transition-all duration-300 border border-red-500/30 hover:border-red-500/60"
            >
              <span>View in Maps</span>
              <span>→</span>
            </a>
          </div>
        </>
      ) : (
        <div className="flex items-center justify-center h-full relative z-10">
          <div className="text-center">
            <div className="text-5xl mb-3">📍</div>
            <p className="text-sm font-medium text-gray-300 mb-1">No Location Data</p>
            <p className="text-xs text-gray-500">Waiting for GPS coordinates...</p>
          </div>
        </div>
      )}
    </div>
  )
}
