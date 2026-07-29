'use client'

interface SensorCardProps {
  icon: string
  label: string
  value: string | number
  unit: string
  color: string
  gradientFrom: string
  gradientTo: string
}

export function SensorCard({
  icon,
  label,
  value,
  unit,
  color,
  gradientFrom,
  gradientTo,
}: SensorCardProps) {
  return (
    <div
      className={`glass-lg p-6 h-full flex flex-col justify-between group hover:border-white/[0.2] transition-all duration-300 overflow-hidden relative`}
    >
      {/* Background Gradient */}
      <div
        className={`absolute inset-0 bg-gradient-to-br ${gradientFrom} ${gradientTo} opacity-0 group-hover:opacity-100 transition-opacity duration-500`}
      ></div>

      {/* Content */}
      <div className="relative z-10">
        <div className="flex items-center justify-between mb-4">
          <h3 className="text-xs font-semibold text-gray-400 uppercase tracking-widest">
            {label}
          </h3>
          <div className="text-2xl opacity-75">{icon}</div>
        </div>

        <div className="flex items-end gap-2">
          <span className={`text-4xl font-black ${color}`}>{value}</span>
          <span className="text-sm text-gray-400 font-medium mb-1">{unit}</span>
        </div>
      </div>

      {/* Bottom accent bar */}
      <div className="relative z-10 mt-6 pt-4 border-t border-white/[0.1]">
        <div className={`h-1 w-full rounded-full bg-gradient-to-r ${gradientFrom} ${gradientTo} opacity-50`}></div>
      </div>
    </div>
  )
}
