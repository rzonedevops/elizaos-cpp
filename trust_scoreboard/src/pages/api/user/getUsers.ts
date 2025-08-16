import type { NextApiRequest, NextApiResponse } from "next"

// Mock trust scoreboard data for demonstration
const mockUsers = [
  {
    id: "1",
    name: "QuantumTrader",
    rank: 1,
    index: 1,
    score: 95.5,
    avatarUrl: "",
    aum: 2500000,
    pnl: 450000,
    badge: "Diamond Partner"
  },
  {
    id: "2", 
    name: "AlphaWhale",
    rank: 2,
    index: 2,
    score: 87.2,
    avatarUrl: "",
    aum: 1800000,
    pnl: 320000,
    badge: "Gold Partner"
  },
  {
    id: "3",
    name: "DefiMaster",
    rank: 3,
    index: 3,
    score: 82.7,
    avatarUrl: "",
    aum: 1500000,
    pnl: 275000,
    badge: "Gold Partner"
  },
  {
    id: "4",
    name: "CryptoSage",
    rank: 4,
    index: 4,
    score: 79.3,
    avatarUrl: "",
    aum: 1200000,
    pnl: 210000,
    badge: "Silver Partner"
  },
  {
    id: "5",
    name: "BlockchainBear",
    rank: 5,
    index: 5,
    score: 75.8,
    avatarUrl: "",
    aum: 950000,
    pnl: 180000,
    badge: "Silver Partner"
  },
  {
    id: "6",
    name: "YieldFarmer",
    rank: 6,
    index: 6,
    score: 71.4,
    avatarUrl: "",
    aum: 800000,
    pnl: 145000,
    badge: "Silver Partner"
  },
  {
    id: "7",
    name: "SmartContractGuru",
    rank: 7,
    index: 7,
    score: 68.9,
    avatarUrl: "",
    aum: 650000,
    pnl: 125000,
    badge: "Bronze Partner"
  },
  {
    id: "8",
    name: "DeFiNinja",
    rank: 8,
    index: 8,
    score: 65.1,
    avatarUrl: "",
    aum: 550000,
    pnl: 95000,
    badge: "Bronze Partner"
  }
];

export default async function handler(
  req: NextApiRequest,
  res: NextApiResponse
) {
  try {
    // Get cursor and limit from query params, with defaults
    const cursor = parseInt(req.query.cursor as string) || 1
    const limit = parseInt(req.query.limit as string) || 100

    // Simulate pagination (though we're using mock data)
    const startIndex = (cursor - 1) * limit
    const endIndex = startIndex + limit
    const paginatedUsers = mockUsers.slice(startIndex, endIndex)

    console.log(`Returning ${paginatedUsers.length} mock users for trust scoreboard`)
    
    return res.status(200).json({
      users: paginatedUsers,
      total: mockUsers.length,
      cursor: cursor,
      hasMore: endIndex < mockUsers.length
    })
  } catch (error) {
    console.error("Error:", error)
    return res.status(500).json({ error: "Internal Server Error" })
  }
}
