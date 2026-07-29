'use client'

import { useState } from 'react'
import { useRouter } from 'next/navigation'
import { login, saveSession } from '@/lib/auth'

export default function LoginPage() {
  const [email, setEmail] = useState('')
  const [password, setPassword] = useState('')
  const [error, setError] = useState('')
  const [loading, setLoading] = useState(false)
  const [rememberMe, setRememberMe] = useState(false)
  const router = useRouter()

  const handleLogin = async (e: React.FormEvent) => {
    e.preventDefault()
    setError('')
    setLoading(true)

    try {
      const result = await login(email, password)
      if (rememberMe && result.user) {
        saveSession(email)
      }
      router.push('/')
    } catch (err: any) {
      setError(err.message || 'Login failed. Please try again.')
      setLoading(false)
    }
  }

  return (
    <div className="min-h-screen w-full py-8 px-4 bg-gradient-to-b from-[#0a0c10] via-[#050608] to-[#020304] flex items-center justify-center relative overflow-y-auto">
      {/* Animated Background Orbs */}
      <div className="absolute top-1/4 right-0 w-72 h-72 bg-red-600/10 rounded-full blur-3xl pointer-events-none" />
      <div className="absolute bottom-1/4 left-0 w-72 h-72 bg-blue-600/10 rounded-full blur-3xl pointer-events-none" />

      <div className="w-full max-w-sm relative z-10 my-auto">
        {/* Mobile App Header */}
        <div className="text-center mb-8">
          <div className="w-16 h-16 rounded-3xl bg-gradient-to-tr from-red-600 to-orange-500 flex items-center justify-center shadow-xl shadow-red-600/30 text-3xl mx-auto mb-4">
            🚒
          </div>
          <h1 className="text-2xl font-black bg-gradient-to-r from-red-400 via-red-500 to-orange-400 bg-clip-text text-transparent mb-1">
            Fire Safety Control
          </h1>
          <p className="text-gray-400 text-xs font-medium">IoT Mobile Safety Portal</p>
        </div>

        {/* Login Card */}
        <div className="bg-white/[0.04] border border-white/[0.08] backdrop-blur-xl p-6 sm:p-8 rounded-3xl shadow-2xl">
          <form onSubmit={handleLogin} className="space-y-5">
            {/* Email Input */}
            <div>
              <label className="block text-[11px] font-extrabold text-gray-300 mb-2 uppercase tracking-wider">
                Email Address
              </label>
              <input
                type="email"
                value={email}
                onChange={(e) => setEmail(e.target.value)}
                placeholder="your@email.com"
                className="w-full px-4 py-3.5 bg-white/[0.05] border border-white/[0.1] rounded-2xl text-white text-sm placeholder-gray-500 focus:outline-none focus:border-red-500/60 focus:ring-1 focus:ring-red-500/30 transition-all"
                required
              />
            </div>

            {/* Password Input */}
            <div>
              <label className="block text-[11px] font-extrabold text-gray-300 mb-2 uppercase tracking-wider">
                Password
              </label>
              <input
                type="password"
                value={password}
                onChange={(e) => setPassword(e.target.value)}
                placeholder="••••••••"
                className="w-full px-4 py-3.5 bg-white/[0.05] border border-white/[0.1] rounded-2xl text-white text-sm placeholder-gray-500 focus:outline-none focus:border-red-500/60 focus:ring-1 focus:ring-red-500/30 transition-all"
                required
              />
            </div>

            {/* Remember Me Checkbox */}
            <div className="flex items-center gap-3">
              <input
                type="checkbox"
                id="rememberMe"
                checked={rememberMe}
                onChange={(e) => setRememberMe(e.target.checked)}
                className="w-4 h-4 bg-white/[0.1] border border-white/[0.2] rounded cursor-pointer accent-red-600"
              />
              <label htmlFor="rememberMe" className="text-xs text-gray-400 cursor-pointer hover:text-gray-300 transition-colors">
                Keep me signed in
              </label>
            </div>

            {/* Error Message */}
            {error && (
              <div className="p-3.5 bg-red-950/40 border border-red-500/50 rounded-2xl text-red-300 text-xs font-medium backdrop-blur-sm">
                {error}
              </div>
            )}

            {/* Submit Button */}
            <button
              type="submit"
              disabled={loading}
              className="w-full py-4 px-4 bg-gradient-to-r from-red-600 to-orange-600 hover:from-red-500 hover:to-orange-500 text-white font-extrabold rounded-2xl transition-all duration-300 shadow-xl shadow-red-600/30 disabled:opacity-50 disabled:cursor-not-allowed uppercase tracking-wider text-xs min-h-[48px] active:scale-[0.98]"
            >
              {loading ? (
                <div className="flex items-center justify-center gap-2">
                  <div className="w-4 h-4 border-2 border-white/30 border-t-white rounded-full animate-spin" />
                  Signing in...
                </div>
              ) : (
                'Sign In to Dashboard'
              )}
            </button>
          </form>

          {/* Divider */}
          <div className="mt-5 pt-5 border-t border-white/[0.08]">
            <p className="text-center text-[11px] text-gray-400 font-medium">
              🔐 Protected by Firebase Authentication
            </p>
          </div>
        </div>
      </div>
    </div>
  )
}
