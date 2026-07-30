import { useEffect, useRef, useState, useCallback } from 'react'

export function useAlertAudio() {
  const audioRef = useRef<HTMLAudioElement | null>(null)
  const [isPlaying, setIsPlaying] = useState(false)
  const shouldPlayRef = useRef(false)

  // Initialize audio element
  useEffect(() => {
    if (!audioRef.current) {
      const audio = new Audio('/alert-sound.wav')
      audio.loop = true
      audio.volume = 0.7
      audioRef.current = audio

      audio.addEventListener('play', () => setIsPlaying(true))
      audio.addEventListener('pause', () => setIsPlaying(false))
      audio.addEventListener('ended', () => {
        if (shouldPlayRef.current && audioRef.current) {
          audioRef.current.currentTime = 0
          audioRef.current.play().catch(() => {})
        }
      })
    }

    // Auto-unlock audio on user interaction if autoplay was blocked
    const unlockAudio = () => {
      if (shouldPlayRef.current && audioRef.current && audioRef.current.paused) {
        audioRef.current.currentTime = 0
        audioRef.current.play().catch(() => {})
      }
    }

    window.addEventListener('click', unlockAudio)
    window.addEventListener('touchstart', unlockAudio)

    return () => {
      window.removeEventListener('click', unlockAudio)
      window.removeEventListener('touchstart', unlockAudio)
      if (audioRef.current) {
        audioRef.current.pause()
      }
    }
  }, [])

  const startAudio = useCallback(() => {
    shouldPlayRef.current = true
    if (audioRef.current) {
      audioRef.current.loop = true
      if (audioRef.current.paused) {
        audioRef.current.currentTime = 0
        audioRef.current.play().catch(() => {
          // Autoplay policy blocked initial playback; will resume on first user interaction
        })
      }
    }
  }, [])

  const stopAudio = useCallback(() => {
    shouldPlayRef.current = false
    if (audioRef.current) {
      audioRef.current.pause()
      audioRef.current.currentTime = 0
    }
    setIsPlaying(false)
  }, [])

  const setVolume = useCallback((volume: number) => {
    if (audioRef.current) {
      audioRef.current.volume = Math.max(0, Math.min(1, volume))
    }
  }, [])

  return {
    startAudio,
    stopAudio,
    isPlaying,
    setVolume,
  }
}

