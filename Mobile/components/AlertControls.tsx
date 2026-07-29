'use client'

import { useAuth } from '@/context/AuthContext'
import { useRouter } from 'next/navigation'

interface AlertControlsProps {
  isAlertActive: boolean
  isSoundPlaying: boolean
  volume: number
  onToggleSound: () => void
  onVolumeChange: (volume: number) => void
}

export const AlertControls = ({
  isAlertActive,
  isSoundPlaying,
  volume,
  onToggleSound,
  onVolumeChange,
}: AlertControlsProps) => {
  const { logout } = useAuth()
  const router = useRouter()

  const handleLogout = async () => {
    await logout()
    router.push('/login')
  }

  return (
    <div className="glass-lg h-full flex flex-col justify-between p-6 group">
      {/* Sound Controls */}
      <div className="space-y-5">
        {/* Header */}
        <div>
          <p className="text-xs font-semibold text-gray-400 uppercase tracking-widest mb-4">
            Audio Control
          </p>

          {/* Status Badge */}
          <div
            className={`p-4 rounded-xl text-center backdrop-blur-sm transition-all duration-300 border ${
              isSoundPlaying && isAlertActive
                ? 'bg-red-600/30 border-red-500/50'
                : 'bg-white/[0.05] border-white/[0.1]'
            }`}
          >
            <p className={`text-sm font-bold ${isSoundPlaying && isAlertActive ? 'text-red-300' : 'text-gray-400'}`}>
              {isSoundPlaying && isAlertActive ? '🔊 ALERT AUDIO ON' : '🔇 AUDIO MUTED'}
            </p>
          </div>
        </div>

        {/* Volume Control */}
        <div>
          <div className="flex items-center justify-between mb-3">
            <label className="text-xs font-semibold text-gray-400 uppercase tracking-wider">
              Volume
            </label>
            <span className="text-sm font-bold text-red-400">{Math.round(volume * 100)}%</span>
          </div>
          <input
            type="range"
            min="0"
            max="100"
            value={Math.round(volume * 100)}
            onChange={(e) => onVolumeChange(parseInt(e.target.value) / 100)}
            className="w-full h-1.5 bg-gradient-to-r from-gray-700 to-gray-600 rounded-full appearance-none cursor-pointer accent-red-500"
          />
        </div>

        {/* Mute/Unmute Button */}
        <button
          onClick={onToggleSound}
          disabled={!isAlertActive}
          className={`w-full py-3 px-4 rounded-xl font-bold text-sm transition-all duration-300 uppercase tracking-wider ${
            isAlertActive
              ? isSoundPlaying
                ? 'bg-gradient-to-r from-red-600 to-red-700 hover:from-red-500 hover:to-red-600 text-white shadow-lg hover:shadow-red-600/50'
                : 'bg-gradient-to-r from-gray-700 to-gray-600 hover:from-gray-600 hover:to-gray-500 text-white'
              : 'bg-gradient-to-r from-gray-800 to-gray-700 text-gray-500 cursor-not-allowed opacity-50'
          }`}
        >
          {!isAlertActive
            ? 'No Alert'
            : isSoundPlaying
              ? '🔊 Mute Alert'
              : '🔇 Unmute Alert'}
        </button>
      </div>

      {/* Logout Button */}
      <button
        onClick={handleLogout}
        className="w-full py-3 px-4 rounded-xl bg-gradient-to-r from-white/[0.1] to-white/[0.05] hover:from-white/[0.15] hover:to-white/[0.1] text-gray-300 font-semibold text-sm transition-all duration-300 border border-white/[0.1] hover:border-white/[0.2] uppercase tracking-wider"
      >
        Logout
      </button>
    </div>
  )
}
