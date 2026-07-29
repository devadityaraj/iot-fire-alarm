'use client'

import { useEffect, useState } from 'react'
import { ProtectedRoute } from '@/components/ProtectedRoute'
import { FullscreenPrompt } from '@/components/FullscreenPrompt'
import { AlertPopup } from '@/components/AlertPopup'
import { useFirebaseListener } from '@/hooks/useFirebaseListener'
import { useAlertAudio } from '@/hooks/useAlertAudio'
import { formatSmokePercentage } from '@/lib/utils'

function DashboardContent() {
  const { data, loading, error } = useFirebaseListener()
  const { startAudio, stopAudio } = useAlertAudio()
  const [showAlertPopup, setShowAlertPopup] = useState(false)
  const [prevAlertState, setPrevAlertState] = useState(0)

  const isAlertActive = data?.alert === 1
  const lat = data?.lat ? parseFloat(data.lat) : null
  const lon = data?.lon ? parseFloat(data.lon) : null

  // Handle alert state changes
  useEffect(() => {
    if (data) {
      if (data.alert === 1 && prevAlertState === 0) {
        setShowAlertPopup(true)
        startAudio()
      } else if (data.alert === 0 && prevAlertState === 1) {
        stopAudio()
        setShowAlertPopup(false)
      }
      setPrevAlertState(data.alert)
    }
  }, [data?.alert, prevAlertState, startAudio, stopAudio])

  const handleClosePopup = () => {
    setShowAlertPopup(false)
    stopAudio()
  }

  return (
    <main
      className={`fixed inset-0 overflow-hidden transition-colors duration-700 ${isAlertActive
          ? 'bg-gradient-to-br from-[#1a0404] via-[#0e0303] to-[#180303]'
          : 'bg-gradient-to-br from-[#050505] via-[#0a0a0a] to-[#050505]'
        }`}
    >
      {/* Active Alert Ambient Glow Overlay */}
      {isAlertActive && (
        <div className="absolute inset-0 bg-red-600/10 pointer-events-none animate-pulse z-0"></div>
      )}

      {/* Screen Edge Alert Border Indicator */}
      {isAlertActive && (
        <div className="absolute inset-0 border-4 border-red-600/60 shadow-[inset_0_0_80px_rgba(220,38,38,0.3)] pointer-events-none z-30 animate-pulse"></div>
      )}

      {/* Background Effects */}
      <div
        className={`absolute top-0 right-0 w-96 h-96 ${isAlertActive ? 'bg-red-600 opacity-30 animate-pulse' : 'bg-red-600 opacity-5'
          } rounded-full blur-3xl pointer-events-none`}
      ></div>
      <div
        className={`absolute bottom-0 left-0 w-96 h-96 ${isAlertActive ? 'bg-orange-600 opacity-25 animate-pulse' : 'bg-blue-600 opacity-5'
          } rounded-full blur-3xl pointer-events-none`}
      ></div>

      <FullscreenPrompt />
      <AlertPopup isOpen={showAlertPopup} data={data} onClose={handleClosePopup} onStopAudio={stopAudio} />

      {/* Loading State */}
      {loading && (
        <div className="absolute inset-0 flex items-center justify-center">
          <div className="text-center z-50">
            <div className="w-16 h-16 border-4 border-gray-700 border-t-red-600 rounded-full animate-spin mx-auto mb-6"></div>
            <p className="text-gray-400 text-lg font-medium">Loading Fire Safety Portal...</p>
          </div>
        </div>
      )}

      {/* Error State */}
      {error && (
        <div className="absolute inset-0 flex items-center justify-center">
          <div className="glass-lg p-8 text-center max-w-md border-red-500/50 z-50">
            <p className="text-red-400 font-bold text-lg mb-2">Connection Error</p>
            <p className="text-red-300 text-sm">{error}</p>
          </div>
        </div>
      )}

      {/* Professional Fire Department Dashboard */}
      {!loading && !error && (
        <div className="absolute inset-0 p-8 flex flex-col z-10">
          {/* Header Bar */}
          <div className="flex items-center justify-between mb-8">
            <div>
              <h1 className="text-4xl font-black text-white">🚒 Fire Safety Control Department</h1>
              <p className="text-gray-400 text-sm mt-1">Real-time IoT Monitoring System</p>
            </div>

            <div className="flex items-center gap-4">
              {isAlertActive && !showAlertPopup && (
                <button
                  onClick={() => setShowAlertPopup(true)}
                  className="px-4 py-2 bg-red-600 hover:bg-red-500 text-white font-bold text-xs uppercase tracking-wider rounded-xl shadow-lg hover:shadow-red-600/50 transition-all duration-300 animate-pulse"
                >
                  ⚠️ Re-open Alert Details
                </button>
              )}
              <div
                className={`flex items-center gap-3 px-6 py-3 rounded-xl border transition-all duration-500 ${isAlertActive
                    ? 'border-red-500/80 bg-red-600/25 shadow-lg shadow-red-600/30'
                    : 'border-green-500/50 bg-green-600/10'
                  }`}
              >
                <div
                  className={`w-3 h-3 rounded-full ${isAlertActive ? 'bg-red-500 animate-ping' : 'bg-green-500'
                    }`}
                ></div>
                <span
                  className={`font-bold text-sm uppercase tracking-wider ${isAlertActive ? 'text-red-400' : 'text-green-400'
                    }`}
                >
                  {isAlertActive ? 'ALERT ACTIVE' : 'ALL SYSTEMS NORMAL'}
                </span>
              </div>
            </div>
          </div>

          {/* Main Content Grid */}
          <div className="flex-1 grid grid-cols-3 gap-8 min-h-0">
            {/* Left Column - Sensor Cards */}
            <div className="flex flex-col gap-6 min-h-0">
              {/* Temperature Card */}
              <div
                className={`glass-lg p-6 rounded-2xl flex-1 border transition-all duration-500 flex flex-col justify-between ${isAlertActive ? 'border-red-500/40 bg-red-950/20' : 'border-white/[0.1]'
                  }`}
              >
                <div>
                  <p className="text-xs font-bold text-gray-400 uppercase tracking-widest mb-4">Temperature</p>
                  <p className="text-5xl font-black text-orange-400">{data?.temperature?.toFixed(1) || '--'}</p>
                  <p className="text-gray-500 text-sm mt-2">Celsius</p>
                </div>
                <div className="h-2 bg-white/[0.05] rounded-full mt-4 overflow-hidden">
                  <div
                    className="h-full bg-gradient-to-r from-orange-500 to-red-500"
                    style={{ width: `${Math.min(((data?.temperature || 0) / 50) * 100, 100)}%` }}
                  ></div>
                </div>
              </div>

              {/* Humidity Card */}
              <div
                className={`glass-lg p-6 rounded-2xl flex-1 border transition-all duration-500 flex flex-col justify-between ${isAlertActive ? 'border-red-500/40 bg-red-950/20' : 'border-white/[0.1]'
                  }`}
              >
                <div>
                  <p className="text-xs font-bold text-gray-400 uppercase tracking-widest mb-4">Humidity</p>
                  <p className="text-5xl font-black text-cyan-400">{data?.humidity || '--'}</p>
                  <p className="text-gray-500 text-sm mt-2">Percent</p>
                </div>
                <div className="h-2 bg-white/[0.05] rounded-full mt-4 overflow-hidden">
                  <div
                    className="h-full bg-gradient-to-r from-cyan-500 to-blue-500"
                    style={{ width: `${data?.humidity || 0}%` }}
                  ></div>
                </div>
              </div>

              {/* Smoke Card */}
              <div
                className={`glass-lg p-6 rounded-2xl flex-1 border transition-all duration-500 flex flex-col justify-between ${isAlertActive ? 'border-red-500/50 bg-red-950/30' : 'border-white/[0.1]'
                  }`}
              >
                <div>
                  <p className="text-xs font-bold text-gray-400 uppercase tracking-widest mb-4">Smoke Level</p>
                  <p className="text-5xl font-black text-gray-300">
                    {data?.smoke !== undefined && data?.smoke !== null ? `${formatSmokePercentage(data.smoke)}%` : '--'}
                  </p>
                  <p className="text-gray-500 text-sm mt-2">Percent</p>
                </div>
                <div className="h-2 bg-white/[0.05] rounded-full mt-4 overflow-hidden">
                  <div
                    className="h-full bg-gradient-to-r from-gray-500 to-slate-500"
                    style={{ width: `${formatSmokePercentage(data?.smoke)}%` }}
                  ></div>
                </div>
              </div>
            </div>

            {/* Middle Column - Google Maps */}
            <div
              className={`glass-lg rounded-2xl border transition-all duration-500 overflow-hidden flex flex-col ${isAlertActive ? 'border-red-500/40 bg-red-950/20' : 'border-white/[0.1]'
                }`}
            >
              <div className="p-6 border-b border-white/[0.1]">
                <p className="text-xs font-bold text-gray-400 uppercase tracking-widest">Device Location</p>
                {lat && lon && (
                  <p className="text-sm text-gray-300 mt-2 font-mono">
                    {lat.toFixed(6)}, {lon.toFixed(6)}
                  </p>
                )}
              </div>
              <div className="flex-1 overflow-hidden">
                {lat && lon ? (
                  <iframe
                    width="100%"
                    height="100%"
                    style={{ border: 'none' }}
                    loading="lazy"
                    allowFullScreen
                    src={`https://maps.google.com/maps?q=${lat},${lon}&z=15&output=embed`}
                    title="Device Location"
                  />
                ) : (
                  <div className="flex items-center justify-center h-full">
                    <div className="text-center">
                      <p className="text-5xl mb-3">📍</p>
                      <p className="text-gray-400">No location data</p>
                      <p className="text-gray-600 text-sm">Waiting for GPS coordinates...</p>
                    </div>
                  </div>
                )}
              </div>
            </div>

            {/* Right Column - Status & Actions */}
            <div className="flex flex-col gap-6 min-h-0">
              {/* Status Card */}
              <div
                className={`glass-lg p-6 rounded-2xl border transition-all duration-500 ${isAlertActive ? 'border-red-500/50 bg-red-950/30' : 'border-white/[0.1]'
                  }`}
              >
                <p className="text-xs font-bold text-gray-400 uppercase tracking-widest mb-6">System Status</p>
                <div className="space-y-4">
                  <div className="flex items-center justify-between">
                    <span className="text-gray-400 text-sm">Alert Status</span>
                    <span className={`font-bold text-sm ${isAlertActive ? 'text-red-400 animate-pulse' : 'text-green-400'}`}>
                      {isAlertActive ? 'ACTIVE EMERGENCY' : 'INACTIVE'}
                    </span>
                  </div>
                  <div className="flex items-center justify-between">
                    <span className="text-gray-400 text-sm">Alert Type</span>
                    <span className="text-gray-300 font-semibold text-sm uppercase">{data?.alerttype || 'ok'}</span>
                  </div>
                  <div className="flex items-center justify-between">
                    <span className="text-gray-400 text-sm">Last Trigger</span>
                    <span className="text-purple-400 font-semibold text-sm">{data?.lastTrigger || 'Never'}</span>
                  </div>
                </div>
              </div>

              {/* Action Buttons */}
              <div
                className={`glass-lg p-6 rounded-2xl border transition-all duration-500 flex-1 flex flex-col justify-between ${isAlertActive ? 'border-red-500/40 bg-red-950/20' : 'border-white/[0.1]'
                  }`}
              >
                <p className="text-xs font-bold text-gray-400 uppercase tracking-widest mb-6">Quick Actions</p>
                <div className="flex flex-col gap-3">
                  {/* Drive Button */}
                  <a
                    href={lat && lon ? `https://www.google.com/maps/search/${lat},${lon}` : '#'}
                    target="_blank"
                    rel="noopener noreferrer"
                    className={`py-4 px-4 rounded-xl font-bold text-center uppercase tracking-wider transition-all duration-300 ${lat && lon
                        ? 'bg-gradient-to-r from-blue-600 to-blue-700 hover:from-blue-500 hover:to-blue-600 text-white shadow-lg hover:shadow-blue-600/50'
                        : 'bg-gray-700/50 text-gray-500 cursor-not-allowed'
                      }`}
                  >
                    Drive Location
                  </a>

                  {/* Call Button */}
                  <a
                    href={data?.phone ? `tel:${data.phone}` : '#'}
                    className={`py-4 px-4 rounded-xl font-bold text-center uppercase tracking-wider transition-all duration-300 ${data?.phone
                        ? 'bg-gradient-to-r from-green-600 to-green-700 hover:from-green-500 hover:to-green-600 text-white shadow-lg hover:shadow-green-600/50'
                        : 'bg-gray-700/50 text-gray-500 cursor-not-allowed'
                      }`}
                  >
                    Call Responder
                  </a>
                </div>
              </div>
            </div>
          </div>
        </div>
      )}
    </main>
  )
}

export default function Page() {
  return (
    <ProtectedRoute>
      <DashboardContent />
    </ProtectedRoute>
  )
}
