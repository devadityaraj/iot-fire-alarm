import { useEffect, useRef, useState } from 'react'

export const useAlertSound = () => {
  const audioContextRef = useRef<AudioContext | null>(null)
  const oscillatorRef = useRef<OscillatorNode | null>(null)
  const gainNodeRef = useRef<GainNode | null>(null)
  const [isPlaying, setIsPlaying] = useState(false)
  const [volume, setVolume] = useState(0.5)

  const startSound = () => {
    try {
      if (!audioContextRef.current) {
        audioContextRef.current = new (window.AudioContext ||
          (window as any).webkitAudioContext)()
      }

      const audioContext = audioContextRef.current
      if (audioContext.state === 'suspended') {
        audioContext.resume()
      }

      // Stop previous oscillator if exists
      if (oscillatorRef.current) {
        oscillatorRef.current.stop()
      }

      // Create oscillator
      const oscillator = audioContext.createOscillator()
      const gainNode = audioContext.createGain()

      oscillator.type = 'sine'
      oscillator.frequency.value = 1000 // 1kHz tone

      gainNode.gain.value = volume * 0.3 // Keep volume reasonable

      oscillator.connect(gainNode)
      gainNode.connect(audioContext.destination)

      oscillator.start()

      oscillatorRef.current = oscillator
      gainNodeRef.current = gainNode
      setIsPlaying(true)
    } catch (error) {
      // Silently fail if audio context unavailable
    }
  }

  const stopSound = () => {
    try {
      if (oscillatorRef.current) {
        oscillatorRef.current.stop()
        oscillatorRef.current = null
        gainNodeRef.current = null
        setIsPlaying(false)
      }
    } catch (error) {
      // Silently fail if already stopped
    }
  }

  const toggleSound = () => {
    if (isPlaying) {
      stopSound()
    } else {
      startSound()
    }
  }

  const updateVolume = (newVolume: number) => {
    setVolume(newVolume)
    if (gainNodeRef.current) {
      gainNodeRef.current.gain.value = newVolume * 0.3
    }
  }

  const muteSound = () => {
    if (gainNodeRef.current) {
      gainNodeRef.current.gain.value = 0
    }
  }

  const unmuteSound = () => {
    if (gainNodeRef.current) {
      gainNodeRef.current.gain.value = volume * 0.3
    }
  }

  // Cleanup on unmount
  useEffect(() => {
    return () => {
      stopSound()
    }
  }, [])

  return {
    startSound,
    stopSound,
    toggleSound,
    isPlaying,
    volume,
    updateVolume,
    muteSound,
    unmuteSound,
  }
}
