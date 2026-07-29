import { clsx, type ClassValue } from 'clsx'
import { twMerge } from 'tailwind-merge'

export function cn(...inputs: ClassValue[]) {
  return twMerge(clsx(inputs))
}

/**
 * Maps raw ESP32 analog smoke sensor value (0-4095 ADC) to a 0-100% range.
 */
export function formatSmokePercentage(rawSmoke: number | undefined | null): number {
  if (rawSmoke === undefined || rawSmoke === null || isNaN(rawSmoke)) return 0

  // ESP32 12-bit ADC full scale range is 0 to 4095 (0 - 4K)
  const percentage = (rawSmoke / 4095) * 100

  return Math.min(100, Math.max(0, Math.round(percentage)))
}
