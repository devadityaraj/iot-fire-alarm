'use client'

import { useEffect, useState } from 'react'
import { ProtectedRoute } from '@/components/ProtectedRoute'
import { AlertPopup } from '@/components/AlertPopup'
import { useFirebaseListener } from '@/hooks/useFirebaseListener'
import { useAlertAudio } from '@/hooks/useAlertAudio'
import { formatSmokePercentage } from '@/lib/utils'
import { ref, update } from 'firebase/database'
import { database } from '@/lib/firebase'

function MobileDashboardContent() {
  const { data, loading, error } = useFirebaseListener()
  const { startAudio, stopAudio } = useAlertAudio()
  const [showAlertPopup, setShowAlertPopup] = useState(false)
  const [prevAlertState, setPrevAlertState] = useState(0)
  const [isStopping, setIsStopping] = useState(false)

  const isAlertActive = data?.alert === 1
  const lat = data?.lat ? parseFloat(data.lat) : null
  const lon = data?.lon ? parseFloat(data.lon) : null

  // Handle alert audio & popup triggers
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

  // STOP ALARM action: updates Firebase Realtime Database node 'device'
  // sets alert to 0 and alerttype to 'reset'
  const handleStopAlarm = async () => {
    if (!isAlertActive || isStopping) return
    setIsStopping(true)
    try {
      const deviceRef = ref(database, 'device')
      await update(deviceRef, {
        alert: 0,
        alerttype: 'reset',
      })
      stopAudio()
      setShowAlertPopup(false)
    } catch (err) {
      console.error('Failed to stop alarm in Firebase:', err)
    } finally {
      setIsStopping(false)
    }
  }

  return (
    <main
      className={`min-h-screen w-full select-none transition-colors duration-700 ${
        isAlertActive
          ? 'bg-gradient-to-b from-[#1a0505] via-[#0d0303] to-[#050505]'
          : 'bg-gradient-to-b from-[#0a0c10] via-[#050608] to-[#020304]'
      } text-white flex flex-col justify-between p-4 sm:p-6 pb-6 relative overflow-x-hidden`}
    >
      {/* Ambient emergency background glow */}
      {isAlertActive && (
        <div className="fixed inset-0 bg-red-600/10 pointer-events-none animate-pulse z-0" />
      )}

      {/* Screen edge alert glow indicator */}
      {isAlertActive && (
        <div className="fixed inset-0 border-2 border-red-500/40 pointer-events-none z-30 animate-pulse" />
      )}

      {/* Background Gradient Orbs */}
      <div
        className={`fixed -top-20 -right-20 w-72 h-72 ${
          isAlertActive ? 'bg-red-600/25 animate-pulse' : 'bg-red-500/5'
        } rounded-full blur-3xl pointer-events-none`}
      />
      <div
        className={`fixed -bottom-20 -left-20 w-72 h-72 ${
          isAlertActive ? 'bg-orange-600/20 animate-pulse' : 'bg-blue-600/5'
        } rounded-full blur-3xl pointer-events-none`}
      />

      <AlertPopup
        isOpen={showAlertPopup}
        data={data}
        onClose={handleClosePopup}
        onStopAudio={stopAudio}
        onStopAlarm={handleStopAlarm}
      />

      {/* Loading Screen */}
      {loading && (
        <div className="fixed inset-0 flex items-center justify-center bg-black/90 backdrop-blur-md z-50">
          <div className="text-center p-6">
            <div className="w-14 h-14 border-4 border-gray-800 border-t-red-500 rounded-full animate-spin mx-auto mb-4" />
            <p className="text-gray-300 font-semibold text-base">Connecting to IoT Device...</p>
            <p className="text-gray-500 text-xs mt-1">Smart Fire Safety Mobile Portal</p>
          </div>
        </div>
      )}

      {/* Error Screen */}
      {error && (
        <div className="fixed inset-0 flex items-center justify-center p-6 bg-black/90 backdrop-blur-md z-50">
          <div className="bg-red-950/40 border border-red-500/40 rounded-3xl p-6 text-center max-w-xs shadow-2xl">
            <div className="text-4xl mb-3">⚠️</div>
            <p className="text-red-400 font-bold text-lg mb-1">Connection Error</p>
            <p className="text-red-300/80 text-xs">{error}</p>
          </div>
        </div>
      )}

      {!loading && !error && (
        <div className="relative z-10 flex flex-col flex-1 max-w-lg mx-auto w-full gap-5">
          {/* Top Header Bar */}
          <header className="flex items-center justify-between pt-2">
            <div className="flex items-center gap-3">
              <div className="w-10 h-10 rounded-2xl bg-gradient-to-tr from-red-600 to-orange-500 flex items-center justify-center shadow-lg shadow-red-600/30 text-xl shrink-0">
                🚒
              </div>
              <div>
                <h1 className="text-lg font-black tracking-tight text-white leading-tight">
                  Fire Safety Monitor
                </h1>
                <p className="text-xs text-gray-400 font-medium">IoT Mobile Control</p>
              </div>
            </div>

            {/* Emergency Status Badge */}
            <div
              className={`flex items-center gap-2 px-3.5 py-1.5 rounded-full border text-xs font-bold uppercase tracking-wider backdrop-blur-md transition-all duration-500 ${
                isAlertActive
                  ? 'border-red-500/80 bg-red-600/30 text-red-300 shadow-lg shadow-red-600/40 animate-pulse'
                  : 'border-emerald-500/30 bg-emerald-500/10 text-emerald-400'
              }`}
            >
              <span
                className={`w-2 h-2 rounded-full ${
                  isAlertActive ? 'bg-red-500 animate-ping' : 'bg-emerald-400'
                }`}
              />
              <span>{isAlertActive ? 'ALERT' : 'NORMAL'}</span>
            </div>
          </header>

          {/* STOP ALARM Hero Emergency Override Control */}
          <section className="relative overflow-hidden rounded-3xl transition-all duration-500">
            <div
              className={`p-5 rounded-3xl border transition-all duration-500 backdrop-blur-xl ${
                isAlertActive
                  ? 'bg-gradient-to-br from-red-950/80 via-red-900/40 to-black/60 border-red-500/60 shadow-2xl shadow-red-600/30'
                  : 'bg-white/[0.03] border-white/[0.08]'
              }`}
            >
              <div className="flex items-center justify-between mb-3">
                <div className="flex items-center gap-2">
                  <span className="text-xl">{isAlertActive ? '🚨' : '🛡️'}</span>
                  <span className="text-xs font-bold uppercase tracking-widest text-gray-400">
                    System Control
                  </span>
                </div>
                {isAlertActive && (
                  <span className="text-[10px] font-extrabold uppercase px-2.5 py-0.5 rounded-full bg-red-500/20 text-red-400 border border-red-500/30">
                    Triggered: {data?.alerttype || 'Emergency'}
                  </span>
                )}
              </div>

              {/* STOP ALARM BUTTON */}
              <button
                onClick={handleStopAlarm}
                disabled={!isAlertActive || isStopping}
                className={`w-full py-4 px-5 rounded-2xl font-black text-sm uppercase tracking-wider transition-all duration-300 flex items-center justify-center gap-3 shadow-xl ${
                  isAlertActive
                    ? 'bg-gradient-to-r from-red-600 via-red-500 to-orange-600 hover:from-red-500 hover:to-orange-500 text-white shadow-red-600/50 active:scale-[0.98] animate-pulse cursor-pointer'
                    : 'bg-white/[0.04] text-gray-500 border border-white/[0.05] cursor-not-allowed'
                }`}
              >
                <span className="text-xl">{isStopping ? '⏳' : isAlertActive ? '🔕' : '🔒'}</span>
                <span>
                  {isStopping
                    ? 'RESETTING ALARM...'
                    : isAlertActive
                    ? 'STOP ALARM (RESET SYSTEM)'
                    : 'ALARM OFF - SYSTEM NORMAL'}
                </span>
              </button>

              {isAlertActive && (
                <p className="text-[11px] text-center text-red-300/70 mt-2 font-medium">
                  Tap button above to silence alarm audio & reset device alert state to normal.
                </p>
              )}
            </div>
          </section>

          {/* Sensor Readings Dashboard */}
          <section className="space-y-3.5">
            <h2 className="text-xs font-extrabold uppercase tracking-widest text-gray-400 px-1">
              Live Sensor Telemetry
            </h2>

            {/* Temperature Sensor Card */}
            <div
              className={`p-4 rounded-3xl border backdrop-blur-xl transition-all duration-500 ${
                isAlertActive
                  ? 'bg-red-950/20 border-red-500/30'
                  : 'bg-gradient-to-br from-white/[0.05] to-white/[0.02] border-white/[0.08]'
              }`}
            >
              <div className="flex items-center justify-between mb-2">
                <div className="flex items-center gap-2">
                  <div className="w-8 h-8 rounded-xl bg-orange-500/15 border border-orange-500/30 flex items-center justify-center text-orange-400">
                    🌡️
                  </div>
                  <div>
                    <p className="text-xs font-bold text-gray-300">Temperature</p>
                    <p className="text-[10px] text-gray-500">Thermal Monitor</p>
                  </div>
                </div>
                <div className="text-right">
                  <span className="text-2xl font-black text-orange-400 tracking-tight">
                    {data?.temperature?.toFixed(1) ?? '--'}
                  </span>
                  <span className="text-xs font-bold text-gray-400 ml-1">°C</span>
                </div>
              </div>
              {/* Progress Bar */}
              <div className="w-full h-2 bg-white/[0.06] rounded-full overflow-hidden mt-3">
                <div
                  className="h-full bg-gradient-to-r from-amber-500 to-red-500 rounded-full transition-all duration-500"
                  style={{ width: `${Math.min(((data?.temperature || 0) / 50) * 100, 100)}%` }}
                />
              </div>
            </div>

            {/* Humidity Sensor Card */}
            <div
              className={`p-4 rounded-3xl border backdrop-blur-xl transition-all duration-500 ${
                isAlertActive
                  ? 'bg-red-950/20 border-red-500/30'
                  : 'bg-gradient-to-br from-white/[0.05] to-white/[0.02] border-white/[0.08]'
              }`}
            >
              <div className="flex items-center justify-between mb-2">
                <div className="flex items-center gap-2">
                  <div className="w-8 h-8 rounded-xl bg-cyan-500/15 border border-cyan-500/30 flex items-center justify-center text-cyan-400">
                    💧
                  </div>
                  <div>
                    <p className="text-xs font-bold text-gray-300">Humidity</p>
                    <p className="text-[10px] text-gray-500">Air Moisture</p>
                  </div>
                </div>
                <div className="text-right">
                  <span className="text-2xl font-black text-cyan-400 tracking-tight">
                    {data?.humidity ?? '--'}
                  </span>
                  <span className="text-xs font-bold text-gray-400 ml-1">%</span>
                </div>
              </div>
              {/* Progress Bar */}
              <div className="w-full h-2 bg-white/[0.06] rounded-full overflow-hidden mt-3">
                <div
                  className="h-full bg-gradient-to-r from-cyan-500 to-blue-500 rounded-full transition-all duration-500"
                  style={{ width: `${Math.min(data?.humidity || 0, 100)}%` }}
                />
              </div>
            </div>

            {/* Smoke Sensor Card */}
            <div
              className={`p-4 rounded-3xl border backdrop-blur-xl transition-all duration-500 ${
                isAlertActive
                  ? 'bg-red-950/30 border-red-500/40'
                  : 'bg-gradient-to-br from-white/[0.05] to-white/[0.02] border-white/[0.08]'
              }`}
            >
              <div className="flex items-center justify-between mb-2">
                <div className="flex items-center gap-2">
                  <div className="w-8 h-8 rounded-xl bg-gray-500/20 border border-gray-400/30 flex items-center justify-center text-gray-300">
                    💨
                  </div>
                  <div>
                    <p className="text-xs font-bold text-gray-300">Smoke Density</p>
                    <p className="text-[10px] text-gray-500">Gas & Particle Detection</p>
                  </div>
                </div>
                <div className="text-right">
                  <span className="text-2xl font-black text-gray-200 tracking-tight">
                    {data?.smoke !== undefined && data?.smoke !== null
                      ? `${formatSmokePercentage(data.smoke)}%`
                      : '--'}
                  </span>
                </div>
              </div>
              {/* Progress Bar */}
              <div className="w-full h-2 bg-white/[0.06] rounded-full overflow-hidden mt-3">
                <div
                  className="h-full bg-gradient-to-r from-gray-400 to-red-500 rounded-full transition-all duration-500"
                  style={{ width: `${Math.min(formatSmokePercentage(data?.smoke), 100)}%` }}
                />
              </div>
            </div>
          </section>

          {/* Location Coordinates & Last Trigger Details */}
          <section className="p-4 rounded-3xl bg-white/[0.03] border border-white/[0.08] backdrop-blur-xl flex justify-between items-center text-xs">
            <div>
              <p className="text-[10px] font-extrabold uppercase tracking-widest text-gray-500">GPS Location</p>
              {lat && lon ? (
                <p className="font-mono text-gray-300 mt-0.5 text-[11px]">
                  📍 {lat.toFixed(5)}, {lon.toFixed(5)}
                </p>
              ) : (
                <p className="text-gray-500 mt-0.5">Waiting for GPS...</p>
              )}
            </div>

            <div className="text-right">
              <p className="text-[10px] font-extrabold uppercase tracking-widest text-gray-500">Last Trigger</p>
              <p className="font-bold text-purple-400 mt-0.5 text-[11px]">
                {data?.lastTrigger || 'None'}
              </p>
            </div>
          </section>

          {/* Quick Action Navigation Bar */}
          <footer className="pt-2">
            <div className="grid grid-cols-2 gap-3">
              {/* Drive Location Link */}
              <a
                href={lat && lon ? `https://www.google.com/maps/search/${lat},${lon}` : '#'}
                target="_blank"
                rel="noopener noreferrer"
                className={`py-3.5 px-4 rounded-2xl font-bold text-xs uppercase tracking-wider text-center flex items-center justify-center gap-2 transition-all duration-300 min-h-[48px] shadow-lg ${
                  lat && lon
                    ? 'bg-gradient-to-r from-blue-600 to-blue-700 hover:from-blue-500 hover:to-blue-600 text-white shadow-blue-600/30 active:scale-[0.98]'
                    : 'bg-white/[0.04] text-gray-600 border border-white/[0.05] cursor-not-allowed'
                }`}
              >
                <span>🚗</span>
                <span>Drive Location</span>
              </a>

              {/* Call Responder Link */}
              <a
                href={data?.phone ? `tel:${data.phone}` : '#'}
                className={`py-3.5 px-4 rounded-2xl font-bold text-xs uppercase tracking-wider text-center flex items-center justify-center gap-2 transition-all duration-300 min-h-[48px] shadow-lg ${
                  data?.phone
                    ? 'bg-gradient-to-r from-emerald-600 to-emerald-700 hover:from-emerald-500 hover:to-emerald-600 text-white shadow-emerald-600/30 active:scale-[0.98]'
                    : 'bg-white/[0.04] text-gray-600 border border-white/[0.05] cursor-not-allowed'
                }`}
              >
                <span>📞</span>
                <span>Call Responder</span>
              </a>
            </div>
          </footer>
        </div>
      )}
    </main>
  )
}

export default function Page() {
  return (
    <ProtectedRoute>
      <MobileDashboardContent />
    </ProtectedRoute>
  )
}
