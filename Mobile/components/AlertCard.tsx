'use client'

import { DeviceData } from '@/hooks/useFirebaseListener'

interface AlertCardProps {
  data: DeviceData | null
}

export const AlertCard = ({ data }: AlertCardProps) => {
  const isActive = data?.alert === 1
  const alertType = data?.alerttype || 'ok'

  const getAlertInfo = () => {
    switch (alertType.toLowerCase()) {
      case 'fire':
        return {
          icon: '🔥',
          label: 'FIRE ALERT',
          color: 'text-red-400',
          bgGradient: 'from-red-600/40 to-orange-600/20',
          borderColor: 'border-red-500/50',
        }
      case 'smoke':
        return {
          icon: '💨',
          label: 'SMOKE DETECTED',
          color: 'text-orange-400',
          bgGradient: 'from-orange-500/40 to-yellow-600/20',
          borderColor: 'border-orange-500/50',
        }
      case 'hightemp':
        return {
          icon: '🌡️',
          label: 'HIGH TEMPERATURE',
          color: 'text-red-400',
          bgGradient: 'from-red-600/40 to-orange-600/20',
          borderColor: 'border-red-500/50',
        }
      case 'reset':
        return {
          icon: '🔄',
          label: 'SYSTEM RESET',
          color: 'text-blue-400',
          bgGradient: 'from-blue-600/40 to-cyan-600/20',
          borderColor: 'border-blue-500/50',
        }
      default:
        return {
          icon: '✓',
          label: 'ALL SYSTEMS OK',
          color: 'text-emerald-400',
          bgGradient: 'from-emerald-600/40 to-green-600/20',
          borderColor: 'border-emerald-500/50',
        }
    }
  }

  const alertInfo = getAlertInfo()

  return (
    <div
      className={`glass-lg h-full flex flex-col justify-between overflow-hidden group transition-all duration-500 ${
        isActive ? alertInfo.borderColor : 'border-white/[0.1]'
      } ${isActive ? 'alert-glow' : 'hover:border-white/[0.2]'}`}
    >
      {/* Background Gradient */}
      <div
        className={`absolute inset-0 bg-gradient-to-br ${alertInfo.bgGradient} opacity-0 ${isActive ? 'opacity-100' : 'group-hover:opacity-50'} transition-opacity duration-500`}
      ></div>

      {/* Content */}
      <div className="relative z-10 p-8 flex flex-col h-full justify-between">
        {/* Header */}
        <div>
          <p className="text-xs font-semibold text-gray-400 uppercase tracking-widest mb-6">
            System Alert
          </p>

          {/* Main Alert Display */}
          <div className="flex items-end gap-6">
            <div className="text-7xl leading-none">{alertInfo.icon}</div>
            <div className="flex-1 pb-1">
              <p
                className={`text-sm font-bold uppercase tracking-wider mb-2 ${isActive ? alertInfo.color : 'text-gray-400'}`}
              >
                {isActive ? 'ACTIVE' : 'INACTIVE'}
              </p>
              <h3 className={`text-3xl font-black ${alertInfo.color}`}>{alertInfo.label}</h3>
            </div>
          </div>
        </div>

        {/* Status Bar */}
        <div className="pt-6 border-t border-white/[0.1]">
          <div className="flex items-center gap-3">
            <div
              className={`w-2.5 h-2.5 rounded-full transition-all duration-300 ${
                isActive ? 'bg-red-500 alert-pulse' : 'bg-emerald-500'
              }`}
            ></div>
            <span className={`text-xs font-medium ${isActive ? 'text-red-400' : 'text-emerald-400'}`}>
              {isActive ? 'ALERT IN PROGRESS' : 'NO ALERTS'}
            </span>
          </div>
        </div>
      </div>
    </div>
  )
}
