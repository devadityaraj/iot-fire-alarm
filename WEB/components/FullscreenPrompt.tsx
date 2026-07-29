'use client'

import { useEffect, useState } from 'react'

export const FullscreenPrompt = () => {
  const [showPrompt, setShowPrompt] = useState(false)

  useEffect(() => {
    // Show prompt on website load if not already in fullscreen mode
    if (!document.fullscreenElement) {
      setShowPrompt(true)
    }

    const handleFullscreenChange = () => {
      if (document.fullscreenElement) {
        setShowPrompt(false)
      }
    }

    document.addEventListener('fullscreenchange', handleFullscreenChange)
    return () => document.removeEventListener('fullscreenchange', handleFullscreenChange)
  }, [])

  const handleEnterFullscreen = async () => {
    try {
      const elem = document.documentElement
      const requestFullscreen =
        elem.requestFullscreen ||
        (elem as any).webkitRequestFullscreen ||
        (elem as any).mozRequestFullScreen ||
        (elem as any).msRequestFullscreen

      if (requestFullscreen) {
        await requestFullscreen.call(elem)
      }
      setShowPrompt(false)
    } catch (error) {
      console.error('Fullscreen error:', error)
      setShowPrompt(false)
    }
  }

  const handleCancel = () => {
    setShowPrompt(false)
  }

  if (!showPrompt) return null

  return (
    <div className="fixed inset-0 bg-black/80 backdrop-blur-md flex items-center justify-center z-50 p-4">
      {/* Modal Card */}
      <div className="glass-lg max-w-md w-full text-center relative z-10 p-8 border border-red-500/40 rounded-2xl shadow-2xl">
        {/* Top Right Close 'X' Button */}
        <button
          onClick={handleCancel}
          className="absolute top-4 right-4 text-gray-400 hover:text-white transition-colors text-xl font-bold p-1"
          aria-label="Close popup"
        >
          ✕
        </button>

        {/* Icon */}
        <div className="text-5xl mb-4 animate-pulse"></div>

        {/* Title */}
        <h2 className="text-2xl font-black text-white mb-2">
          Switch to Full Screen
        </h2>

        {/* Description */}
        <p className="text-gray-300 text-sm leading-relaxed mb-6">
          For real-time monitoring and emergency fire safety alerts, switch to full screen mode.
        </p>

        {/* Action Buttons */}
        <div className="flex gap-4">
          <button
            onClick={handleCancel}
            className="flex-1 py-3 px-4 bg-gray-700/60 hover:bg-gray-700 text-gray-300 font-bold rounded-xl transition-all duration-300 uppercase tracking-wider text-sm border border-gray-600/50"
          >
            Cancel
          </button>

          <button
            onClick={handleEnterFullscreen}
            className="flex-1 py-3 px-4 bg-gradient-to-r from-red-600 to-orange-600 hover:from-red-500 hover:to-orange-500 text-white font-bold rounded-xl transition-all duration-300 shadow-lg hover:shadow-red-600/50 uppercase tracking-wider text-sm"
          >
            Full Screen
          </button>
        </div>
      </div>
    </div>
  )
}
