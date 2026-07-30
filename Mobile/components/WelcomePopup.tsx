'use client'

import { useState, useEffect } from 'react'

export function WelcomePopup() {
  const [isOpen, setIsOpen] = useState(false)

  useEffect(() => {
    setIsOpen(true)
  }, [])

  if (!isOpen) return null

  return (
    <div className="fixed inset-0 bg-black/80 backdrop-blur-md flex items-center justify-center z-50 p-4 animate-fadeIn">
      {/* Background glow */}
      <div className="absolute w-72 h-72 bg-red-600/20 rounded-full blur-3xl pointer-events-none" />

      <div className="relative w-full max-w-sm bg-gradient-to-b from-[#161922] to-[#0c0e14] border border-red-500/30 rounded-3xl p-6 text-center shadow-2xl shadow-red-950/50 space-y-4 z-10">
        {/* Header Icon */}
        <div className="w-16 h-16 rounded-2xl bg-gradient-to-tr from-red-600 to-orange-500 flex items-center justify-center mx-auto shadow-lg shadow-red-600/30 text-3xl animate-bounce">
          🚒
        </div>

        {/* Title & Info */}
        <div className="space-y-1">
          <p className="text-[10px] font-black uppercase tracking-widest text-red-400">Welcome</p>
          <h2 className="text-xl font-black text-white tracking-tight leading-snug">
            Smart IoT based Fire Safety Alarm
          </h2>
          <p className="text-xs text-gray-400 pt-2 leading-relaxed">
            Real-time monitoring system is active. Sensor readings and emergency notifications will update live.
          </p>
        </div>

        {/* OK Button */}
        <button
          onClick={() => setIsOpen(false)}
          className="w-full py-3.5 px-6 rounded-2xl bg-gradient-to-r from-red-600 to-orange-600 hover:from-red-500 hover:to-orange-500 text-white font-black text-sm uppercase tracking-wider shadow-lg shadow-red-600/30 active:scale-[0.98] transition-all cursor-pointer mt-2"
        >
          OK
        </button>
      </div>
    </div>
  )
}
