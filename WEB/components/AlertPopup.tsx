'use client'

import { useEffect } from 'react'
import { DeviceData } from '@/hooks/useFirebaseListener'
import { formatSmokePercentage } from '@/lib/utils'

interface AlertPopupProps {
  isOpen: boolean
  data: DeviceData | null
  onClose: () => void
  onStopAudio: () => void
}

export function AlertPopup({ isOpen, data, onClose, onStopAudio }: AlertPopupProps) {
  useEffect(() => {
    if (!isOpen) return

    const handleEsc = (e: KeyboardEvent) => {
      if (e.key === 'Escape') {
        onStopAudio()
        onClose()
      }
    }

    window.addEventListener('keydown', handleEsc)
    return () => window.removeEventListener('keydown', handleEsc)
  }, [isOpen, onClose, onStopAudio])

  if (!isOpen || !data || data.alert !== 1) return null

  const lat = data?.lat ? parseFloat(data.lat) : null
  const lon = data?.lon ? parseFloat(data.lon) : null
  const phone = data?.phone

  const handleClose = () => {
    onStopAudio()
    onClose()
  }

  const handleDrive = () => {
    if (lat && lon) {
      window.open(`https://www.google.com/maps/search/${lat},${lon}`, '_blank')
    }
    handleClose()
  }

  const handleCall = () => {
    if (phone) {
      window.location.href = `tel:${phone}`
    }
    handleClose()
  }

  const getAlertInfo = () => {
    const type = data?.alerttype?.toLowerCase() || 'fire'
    const infoMap: {
      [key: string]: { title: string; icon: string; color: string }
    } = {
      fire: { title: 'FIRE ALERT', icon: '🔥', color: 'text-red-600' },
      smoke: { title: 'SMOKE DETECTED', icon: '💨', color: 'text-orange-600' },
      hightemp: { title: 'HIGH TEMPERATURE', icon: '🌡️', color: 'text-red-600' },
      reset: { title: 'SYSTEM RESET', icon: '🔄', color: 'text-blue-600' },
      ok: { title: 'SYSTEM OK', icon: '✓', color: 'text-green-600' },
    }
    return infoMap[type] || infoMap.fire
  }

  const alertInfo = getAlertInfo()

  return (
    <div className="fixed inset-0 bg-black/90 backdrop-blur-lg flex items-center justify-center z-50 p-4">
      {/* Animated background glow */}
      <div className="absolute inset-0 bg-gradient-to-br from-red-600/10 via-transparent to-orange-600/10 pointer-events-none"></div>

      {/* Main Alert Modal */}
      <div className="glass-lg relative w-full max-w-2xl border-2 border-red-500/50 overflow-hidden">
        {/* Top Right Close 'X' Button */}
        <button
          onClick={handleClose}
          className="absolute top-4 right-4 z-20 text-gray-300 hover:text-white bg-black/40 hover:bg-black/70 rounded-full w-9 h-9 flex items-center justify-center transition-colors text-lg font-bold border border-white/20"
          aria-label="Close alert"
        >
          ✕
        </button>

        {/* Header */}
        <div className="bg-gradient-to-r from-red-600/40 to-orange-600/20 px-8 py-8 border-b border-red-500/30">
          <div className="flex items-center gap-8">
            <div className="text-8xl animate-pulse">{alertInfo.icon}</div>
            <div>
              <p className="text-sm font-bold text-red-400 uppercase tracking-widest mb-2">
                EMERGENCY ALERT
              </p>
              <h1 className={`text-5xl font-black ${alertInfo.color}`}>{alertInfo.title}</h1>
              <p className="text-gray-300 mt-2">Immediate action required</p>
            </div>
          </div>
        </div>

        {/* Content */}
        <div className="p-8 space-y-6">
          {/* Location & Sensor Data Grid */}
          <div className="grid grid-cols-2 gap-6">
            {/* Left - Location & Time */}
            <div className="space-y-4">
              {lat && lon && (
                <div className="bg-white/[0.05] rounded-xl p-4 border border-white/[0.1]">
                  <p className="text-xs font-bold text-gray-400 uppercase tracking-widest mb-3">
                    Location
                  </p>
                  <p className="text-sm font-mono text-gray-300 mb-1">
                    Lat: {lat.toFixed(6)}
                  </p>
                  <p className="text-sm font-mono text-gray-300">
                    Lon: {lon.toFixed(6)}
                  </p>
                </div>
              )}
              {data?.lastTrigger && (
                <div className="bg-white/[0.05] rounded-xl p-4 border border-white/[0.1]">
                  <p className="text-xs font-bold text-gray-400 uppercase tracking-widest mb-2">
                    Last Trigger
                  </p>
                  <p className="text-lg font-bold text-purple-400">{data.lastTrigger}</p>
                </div>
              )}
            </div>

            {/* Right - Sensor Data */}
            <div className="space-y-2">
              <div className="bg-gradient-to-br from-orange-600/20 to-red-600/10 rounded-xl p-4 border border-orange-500/30">
                <p className="text-xs text-gray-400 mb-2 uppercase tracking-widest">Temperature</p>
                <p className="text-3xl font-black text-orange-400">{data?.temperature?.toFixed(1)}°C</p>
              </div>
              <div className="bg-gradient-to-br from-cyan-600/20 to-blue-600/10 rounded-xl p-4 border border-cyan-500/30">
                <p className="text-xs text-gray-400 mb-2 uppercase tracking-widest">Humidity</p>
                <p className="text-3xl font-black text-cyan-400">{data?.humidity}%</p>
              </div>
              <div className="bg-gradient-to-br from-gray-600/20 to-slate-600/10 rounded-xl p-4 border border-gray-500/30">
                <p className="text-xs text-gray-400 mb-2 uppercase tracking-widest">Smoke Level</p>
                <p className="text-3xl font-black text-gray-300">
                  {data?.smoke !== undefined && data?.smoke !== null ? `${formatSmokePercentage(data.smoke)}%` : '--'}
                </p>
              </div>
            </div>
          </div>

          {/* Action Buttons - Drive, Call, and Close Alert */}
          <div className="grid grid-cols-1 md:grid-cols-3 gap-4 pt-4 border-t border-white/[0.1]">
            {/* Drive Button */}
            <button
              onClick={handleDrive}
              disabled={!lat || !lon}
              className={`py-4 px-4 font-bold text-base rounded-xl transition-all duration-300 uppercase tracking-wider ${lat && lon
                  ? 'bg-gradient-to-r from-blue-600 to-blue-700 hover:from-blue-500 hover:to-blue-600 text-white shadow-lg hover:shadow-blue-600/50'
                  : 'bg-gray-700/50 text-gray-500 cursor-not-allowed'
                }`}
            >
              Drive Location
            </button>

            {/* Call Button */}
            <button
              onClick={handleCall}
              disabled={!phone}
              className={`py-4 px-4 font-bold text-base rounded-xl transition-all duration-300 uppercase tracking-wider ${phone
                  ? 'bg-gradient-to-r from-green-600 to-green-700 hover:from-green-500 hover:to-green-600 text-white shadow-lg hover:shadow-green-600/50'
                  : 'bg-gray-700/50 text-gray-500 cursor-not-allowed'
                }`}
            >
              Call Responder
            </button>

            {/* Close Alert Button */}
            <button
              onClick={handleClose}
              className="py-4 px-4 font-bold text-base rounded-xl transition-all duration-300 uppercase tracking-wider bg-gradient-to-r from-red-600 to-red-700 hover:from-red-500 hover:to-red-600 text-white shadow-lg hover:shadow-red-600/50 border border-red-500/40"
            >
              Close Alert
            </button>
          </div>

          {/* Close Instructions */}
          <p className="text-center text-xs text-gray-500">
            Click &quot;Close Alert&quot;, press ESC, or use the ✕ button to dismiss popup (audio alarm will stop)
          </p>
        </div>
      </div>

      {/* Close with ESC key */}
      <style>{`
        @keyframes slideUp {
          from {
            transform: translateY(50px);
            opacity: 0;
          }
          to {
            transform: translateY(0);
            opacity: 1;
          }
        }
      `}</style>


    </div>
  )
}
