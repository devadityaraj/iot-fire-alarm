import { useEffect, useState } from 'react'
import { onValue, ref } from 'firebase/database'
import { database } from '@/lib/firebase'

export interface DeviceData {
  alert: number
  alerttype: string
  humidity: number
  lastTrigger: string
  lat: string
  lon: string
  smoke: number
  temperature: number
  phone?: string
}

export const useFirebaseListener = () => {
  const [data, setData] = useState<DeviceData | null>(null)
  const [loading, setLoading] = useState(true)
  const [error, setError] = useState<string | null>(null)

  useEffect(() => {
    try {
      const deviceRef = ref(database, 'device')
      let phoneNumber = ''

      // First, try to fetch phone number
      const phoneRef = ref(database, 'device/phone')
      onValue(phoneRef, (phoneSnapshot) => {
        if (phoneSnapshot.exists()) {
          phoneNumber = phoneSnapshot.val()
        }
      })

      const unsubscribe = onValue(
        deviceRef,
        (snapshot) => {
          if (snapshot.exists()) {
            const deviceData = snapshot.val() as DeviceData
            setData({ ...deviceData, phone: phoneNumber })
            setError(null)
          }
          setLoading(false)
        },
        (error) => {
          setError(error.message)
          setLoading(false)
        }
      )

      return () => unsubscribe()
    } catch (err) {
      setError(String(err))
      setLoading(false)
    }
  }, [])

  return { data, loading, error }
}
