import {
  createUserWithEmailAndPassword,
  signInWithEmailAndPassword,
  signOut,
  onAuthStateChanged,
  User,
} from 'firebase/auth'
import { auth } from './firebase'

export interface AuthState {
  user: User | null
  loading: boolean
  error: string | null
}

export const signUp = async (email: string, password: string) => {
  return await createUserWithEmailAndPassword(auth, email, password)
}

export const login = async (email: string, password: string) => {
  return await signInWithEmailAndPassword(auth, email, password)
}

export const logout = async () => {
  return await signOut(auth)
}

export const onAuthChange = (callback: (user: User | null) => void) => {
  return onAuthStateChanged(auth, callback)
}

export const saveSession = (email: string) => {
  localStorage.setItem('auth_email', email)
  localStorage.setItem('auth_timestamp', Date.now().toString())
}

export const getSavedSession = () => {
  return localStorage.getItem('auth_email')
}

export const clearSession = () => {
  localStorage.removeItem('auth_email')
  localStorage.removeItem('auth_timestamp')
}

export const isSessionValid = () => {
  const email = getSavedSession()
  return email ? true : false
}
