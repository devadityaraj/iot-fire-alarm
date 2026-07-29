import { useEffect, useRef, useState } from 'react'

export function useAlertAudio() {
  const audioRef = useRef<HTMLAudioElement | null>(null)
  const [isPlaying, setIsPlaying] = useState(false)

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
        if (audio.loop) {
          audio.currentTime = 0
          audio.play()
        }
      })
    }

    return () => {
      if (audioRef.current) {
        audioRef.current.pause()
      }
    }
  }, [])

  const startAudio = () => {
    if (audioRef.current && audioRef.current.paused) {
      audioRef.current.currentTime = 0
      audioRef.current.play().catch(() => {
        // Silently fail if autoplay is blocked
      })
    }
  }

  const stopAudio = () => {
    if (audioRef.current && !audioRef.current.paused) {
      audioRef.current.pause()
      audioRef.current.currentTime = 0
    }
  }

  const setVolume = (volume: number) => {
    if (audioRef.current) {
      audioRef.current.volume = Math.max(0, Math.min(1, volume))
    }
  }

  return {
    startAudio,
    stopAudio,
    isPlaying,
    setVolume,
  }
}
