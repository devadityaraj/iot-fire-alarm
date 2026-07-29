'use client'

import { useEffect } from 'react'
import { DeviceData } from '@/hooks/useFirebaseListener'
import { formatSmokePercentage } from '@/lib/utils'

interface AlertPopupProps {
  isOpen: boolean
  data: DeviceData | null
  onClose: () => void
  onStopAudio: () => void
  onStopAlarm?: () => void
}

export function AlertPopup({ isOpen, data, onClose, onStopAudio, onStopAlarm }: AlertPopupProps) {
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

  const handleStopAlarmClick = () => {
    if (onStopAlarm) {
      onStopAlarm()
    } else {
      handleClose()
    }
  }

  const getAlertInfo = () => {
    const type = data?.alerttype?.toLowerCase() || 'fire'
    const infoMap: {
      [key: string]: { title: string; icon: string; color: string }
    } = {
      fire: { title: 'FIRE ALERT', icon: '🔥', color: 'text-red-500' },
      smoke: { title: 'SMOKE DETECTED', icon: '💨', color: 'text-orange-500' },
      hightemp: { title: 'HIGH TEMPERATURE', icon: '🌡️', color: 'text-red-500' },
      reset: { title: 'SYSTEM RESET', icon: '🔄', color: 'text-blue-500' },
      ok: { title: 'SYSTEM OK', icon: '✓', color: 'text-emerald-500' },
    }
    return infoMap[type] || infoMap.fire
  }

  const alertInfo = getAlertInfo()

  return (
    <div className="fixed inset-0 bg-black/90 backdrop-blur-xl flex items-center justify-center z-50 p-4 overflow-y-auto">
      {/* Background glow */}
      <div className="absolute inset-0 bg-gradient-to-b from-red-600/20 via-transparent to-orange-600/10 pointer-events-none" />

      {/* Main Mobile Popup Card */}
      <div className="relative w-full max-w-sm bg-[#0d0404] border border-red-500/50 rounded-3xl overflow-hidden shadow-2xl shadow-red-600/30 my-auto flex flex-col">
        {/* Top Close Button */}
        <button
          onClick={handleClose}
          className="absolute top-4 right-4 z-20 text-gray-400 hover:text-white bg-white/[0.1] hover:bg-white/[0.2] rounded-full w-8 h-8 flex items-center justify-center transition-colors text-base font-bold"
          aria-label="Close alert popup"
        >
          ✕
        </button>

        {/* Header */}
        <div className="bg-gradient-to-b from-red-600/30 to-transparent p-6 text-center border-b border-red-500/20">
          <div className="text-6xl mb-2 animate-bounce">{alertInfo.icon}</div>
          <p className="text-[10px] font-black text-red-400 uppercase tracking-widest mb-1">
            EMERGENCY NOTIFICATION
          </p>
          <h2 className={`text-2xl font-black ${alertInfo.color}`}>{alertInfo.title}</h2>
        </div>

        {/* Content */}
        <div className="p-5 space-y-4">
          {/* STOP ALARM BUTTON IN POPUP */}
          <button
            onClick={handleStopAlarmClick}
            className="w-full py-4 px-4 rounded-2xl bg-gradient-to-r from-red-600 to-orange-600 hover:from-red-500 hover:to-orange-500 text-white font-black text-sm uppercase tracking-wider shadow-lg shadow-red-600/40 animate-pulse flex items-center justify-center gap-2 active:scale-[0.98]"
          >
            <span>🔕</span>
            <span>STOP ALARM & RESET</span>
          </button>

          {/* Sensor Snapshot */}
          <div className="grid grid-cols-3 gap-2 py-2">
            <div className="bg-white/[0.04] p-2.5 rounded-2xl text-center border border-white/[0.06]">
              <p className="text-[9px] font-bold text-gray-400 uppercase">Temp</p>
              <p className="text-base font-black text-orange-400 mt-0.5">{data?.temperature?.toFixed(1)}°C</p>
            </div>
            <div className="bg-white/[0.04] p-2.5 rounded-2xl text-center border border-white/[0.06]">
              <p className="text-[9px] font-bold text-gray-400 uppercase">Humidity</p>
              <p className="text-base font-black text-cyan-400 mt-0.5">{data?.humidity}%</p>
            </div>
            <div className="bg-white/[0.04] p-2.5 rounded-2xl text-center border border-white/[0.06]">
              <p className="text-[9px] font-bold text-gray-400 uppercase">Smoke</p>
              <p className="text-base font-black text-gray-300 mt-0.5">
                {data?.smoke !== undefined && data?.smoke !== null ? `${formatSmokePercentage(data.smoke)}%` : '--'}
              </p>
            </div>
          </div>

          {/* Actions */}
          <div className="grid grid-cols-2 gap-2.5 pt-2">
            <button
              onClick={handleDrive}
              disabled={!lat || !lon}
              className={`py-3 px-3 rounded-2xl font-bold text-xs uppercase tracking-wider flex items-center justify-center gap-1.5 transition-all ${
                lat && lon
                  ? 'bg-blue-600 hover:bg-blue-500 text-white'
                  : 'bg-white/[0.05] text-gray-600 cursor-not-allowed'
              }`}
            >
              <span>🚗</span> Drive
            </button>
            <button
              onClick={handleCall}
              disabled={!phone}
              className={`py-3 px-3 rounded-2xl font-bold text-xs uppercase tracking-wider flex items-center justify-center gap-1.5 transition-all ${
                phone
                  ? 'bg-emerald-600 hover:bg-emerald-500 text-white'
                  : 'bg-white/[0.05] text-gray-600 cursor-not-allowed'
              }`}
            >
              <span>📞</span> Call
            </button>
          </div>
        </div>
      </div>
    </div>
  )
}
