'use client'

import { useEffect, useState } from 'react'
import { DeviceData } from '@/hooks/useFirebaseListener'

interface ResetPopupProps {
  isOpen: boolean
  data: DeviceData | null
  onClose: () => void
}

export function ResetPopup({ isOpen, data, onClose }: ResetPopupProps) {
  const [showMessage, setShowMessage] = useState(false)

  useEffect(() => {
    if (isOpen && data?.alerttype?.toLowerCase() === 'reset') {
      setShowMessage(true)
      const timer = setTimeout(() => {
        setShowMessage(false)
        onClose()
      }, 4000) // Auto-close after 4 seconds

      return () => clearTimeout(timer)
    }
  }, [isOpen, data, onClose])

  if (!showMessage) return null

  return (
    <div className="fixed inset-0 bg-black/60 backdrop-blur-md flex items-center justify-center z-[110]">
      {/* Reset Confirmation */}
      <div className="glass-lg max-w-md w-full mx-4 overflow-hidden border-2 border-blue-500/50 shadow-2xl shadow-blue-600/20">
        {/* Header */}
        <div className="bg-gradient-to-r from-blue-600/40 to-cyan-600/30 p-8 border-b border-blue-500/30">
          <div className="flex items-end gap-6">
            <div className="text-6xl animate-bounce">🔄</div>
            <div>
              <p className="text-sm font-bold uppercase tracking-wider mb-1 text-blue-400">
                System Status
              </p>
              <h2 className="text-2xl font-black text-blue-400">System Reset</h2>
            </div>
          </div>
        </div>

        {/* Content */}
        <div className="p-8 text-center">
          <p className="text-lg font-semibold text-gray-300 mb-4">
            Alarm has been reset by sensor
          </p>
          <p className="text-sm text-gray-400">
            The system returned to normal state. All sensors are functioning properly.
          </p>

          {/* Check Icon */}
          <div className="mt-6">
            <div className="inline-flex items-center justify-center w-16 h-16 bg-emerald-500/20 rounded-full border-2 border-emerald-500/50">
              <span className="text-3xl">✓</span>
            </div>
          </div>
        </div>

        {/* Footer */}
        <div className="p-4 border-t border-white/[0.1] bg-white/[0.05]">
          <p className="text-xs text-gray-500 text-center">Closing automatically...</p>
        </div>
      </div>
    </div>
  )
}
