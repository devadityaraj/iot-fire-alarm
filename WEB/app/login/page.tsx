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
    <div className="fixed inset-0 overflow-hidden bg-gradient-to-br from-[#050505] via-[#0a0a0a] to-[#050505] flex items-center justify-center px-4">
      {/* Animated Background Orbs */}
      <div className="absolute top-1/4 right-0 w-96 h-96 bg-red-600 opacity-5 rounded-full blur-3xl pointer-events-none"></div>
      <div className="absolute bottom-1/4 left-0 w-96 h-96 bg-blue-600 opacity-5 rounded-full blur-3xl pointer-events-none"></div>

      <div className="w-full max-w-md relative z-10">
        {/* Header */}
        <div className="text-center mb-10">
          <div className="text-5xl mb-4">🔒</div>
          <h1 className="text-4xl font-black bg-gradient-to-r from-red-400 via-red-500 to-orange-500 bg-clip-text text-transparent mb-2">
            Smart IoT Fire Safety
          </h1>
          <p className="text-gray-400 text-sm font-medium">Secure Alarm Management System</p>
        </div>

        {/* Login Form */}
        <div className="glass-lg p-8 md:p-10">
          <form onSubmit={handleLogin} className="space-y-6">
            {/* Email Input */}
            <div>
              <label className="block text-xs font-semibold text-gray-300 mb-3 uppercase tracking-wider">
                Email Address
              </label>
              <input
                type="email"
                value={email}
                onChange={(e) => setEmail(e.target.value)}
                placeholder="your@email.com"
                className="w-full px-4 py-3 bg-white/[0.05] border border-white/[0.1] rounded-xl text-white placeholder-gray-500 focus:outline-none focus:border-red-500/50 focus:ring-1 focus:ring-red-500/20 transition-all"
                required
              />
            </div>

            {/* Password Input */}
            <div>
              <label className="block text-xs font-semibold text-gray-300 mb-3 uppercase tracking-wider">
                Password
              </label>
              <input
                type="password"
                value={password}
                onChange={(e) => setPassword(e.target.value)}
                placeholder="••••••••"
                className="w-full px-4 py-3 bg-white/[0.05] border border-white/[0.1] rounded-xl text-white placeholder-gray-500 focus:outline-none focus:border-red-500/50 focus:ring-1 focus:ring-red-500/20 transition-all"
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
              <label htmlFor="rememberMe" className="text-sm text-gray-400 cursor-pointer hover:text-gray-300 transition-colors">
                Keep me signed in
              </label>
            </div>

            {/* Error Message */}
            {error && (
              <div className="p-4 bg-red-600/20 border border-red-500/50 rounded-xl text-red-300 text-sm font-medium backdrop-blur-sm">
                {error}
              </div>
            )}

            {/* Submit Button */}
            <button
              type="submit"
              disabled={loading}
              className="w-full py-3 px-4 bg-gradient-to-r from-red-600 to-orange-600 hover:from-red-500 hover:to-orange-500 text-white font-bold rounded-xl transition-all duration-300 shadow-lg hover:shadow-red-600/50 disabled:opacity-50 disabled:cursor-not-allowed uppercase tracking-wider text-sm"
            >
              {loading ? (
                <div className="flex items-center justify-center gap-2">
                  <div className="w-4 h-4 border-2 border-white/30 border-t-white rounded-full animate-spin"></div>
                  Signing in...
                </div>
              ) : (
                'Sign In'
              )}
            </button>
          </form>

          {/* Divider */}
          <div className="mt-6 pt-6 border-t border-white/[0.1]">
            <p className="text-center text-xs text-gray-400 font-medium">
              🔐 Secure connection with Firebase Authentication
            </p>
          </div>
        </div>

        {/* Footer Info */}
        <p className="text-center text-gray-500 text-xs mt-6">
          Enter your registered email and password to access the real-time monitoring dashboard
        </p>
      </div>
    </div>
  )
}
