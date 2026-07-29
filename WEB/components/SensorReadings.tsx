'use client'

import { DeviceData } from '@/hooks/useFirebaseListener'
import { formatSmokePercentage } from '@/lib/utils'

interface SensorReadingsProps {
  data: DeviceData | null
}

export const SensorReadings = ({ data }: SensorReadingsProps) => {
  const sensors = [
    {
      label: 'Temperature',
      value: data?.temperature ? `${data.temperature.toFixed(1)}` : '--',
      unit: '°C',
      icon: '🌡️',
      color: 'from-orange-600/40 to-red-600/20',
      accent: 'text-orange-400',
    },
    {
      label: 'Humidity',
      value: data?.humidity ? `${data.humidity}` : '--',
      unit: '%',
      icon: '💧',
      color: 'from-cyan-600/40 to-blue-600/20',
      accent: 'text-cyan-400',
    },
    {
      label: 'Smoke Level',
      value: data?.smoke !== undefined && data?.smoke !== null ? `${formatSmokePercentage(data.smoke)}` : '--',
      unit: '%',
      icon: '💨',
      color: 'from-gray-600/40 to-slate-600/20',
      accent: 'text-gray-400',
    },
    {
      label: 'Last Trigger',
      value: data?.lastTrigger || 'Never',
      unit: '',
      icon: '⏰',
      color: 'from-purple-600/40 to-pink-600/20',
      accent: 'text-purple-400',
    },
  ]

  return (
    <div className="space-y-3 h-full flex flex-col">
      {sensors.map((sensor, index) => (
        <div
          key={index}
          className={`glass-sm overflow-hidden group hover:border-white/[0.2] transition-all duration-300`}
        >
          {/* Background Gradient */}
          <div
            className={`absolute inset-0 bg-gradient-to-br ${sensor.color} opacity-0 group-hover:opacity-100 transition-opacity duration-500`}
          ></div>

          {/* Content */}
          <div className="relative z-10 p-4 flex items-center justify-between gap-3">
            <div className="flex items-center gap-3 flex-1 min-w-0">
              <div className="text-3xl flex-shrink-0">{sensor.icon}</div>
              <div className="min-w-0">
                <p className="text-xs font-semibold text-gray-400 uppercase tracking-wider">
                  {sensor.label}
                </p>
              </div>
            </div>

            <div className="text-right flex-shrink-0">
              <div className="flex items-baseline gap-1">
                <p className={`text-lg font-bold ${sensor.accent}`}>{sensor.value}</p>
                {sensor.unit && <span className="text-xs text-gray-400">{sensor.unit}</span>}
              </div>
            </div>
          </div>
        </div>
      ))}
    </div>
  )
}
