import { useState, useEffect, useCallback } from "react"

export const useGetUsers = ({ cursor = 1, limit = 100 }) => {
  const [isLoading, setIsLoading] = useState(true)
  const [users, setUsers] = useState([])

  const getUsers = useCallback(async () => {
    const response = await fetch(
      `/api/user/getUsers?cursor=${cursor}&limit=${limit}`
    )
    setIsLoading(false)
    const data = await response.json()
    setUsers(data.users)
  }, [cursor, limit])

  useEffect(() => {
    getUsers()
  }, [getUsers])

  return { users, isLoading }
}
