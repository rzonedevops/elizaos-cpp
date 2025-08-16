// pages/api/partners.ts
import type { NextApiRequest, NextApiResponse } from 'next';

export interface Partner {
  owner: string;
  displayAddress: string;
  amount: number;
  trustScore: number;
}

// Mock data for trust scoreboard demonstration
const mockPartners: Partner[] = [
  {
    owner: "7xKXtg2CW87d97TXJSDpbD5jBkheTqA83TZRuJosgAsU",
    displayAddress: "7xKX...gAsU",
    amount: 2500000,
    trustScore: 95.5
  },
  {
    owner: "9WzDXwBbmkg8ZTbNMqUxvQRAyrZzDsGYdLVL9zYtAWWM",
    displayAddress: "9WzD...AWWM", 
    amount: 1800000,
    trustScore: 87.2
  },
  {
    owner: "CuieVDEDtLo7FypA9SbLM9saXFdb1dsshEkyErMqkRQq",
    displayAddress: "Cuie...kRQq",
    amount: 1500000,
    trustScore: 82.7
  },
  {
    owner: "8sLbNZoA1cfnvMJLPfp98ZLAnFSYCFApfJKMbiXNLwxj",
    displayAddress: "8sLb...Lwxj",
    amount: 1200000,
    trustScore: 79.3
  },
  {
    owner: "B1aLAAe4vW8nSQCetXnYqJfRxzTjnbooczwkUJAr7yMS",
    displayAddress: "B1aL...7yMS",
    amount: 950000,
    trustScore: 75.8
  },
  {
    owner: "DjVE6JNiYqPL2QXyCUEh73933aNBRXS4vqB2AF2H2H6x",
    displayAddress: "DjVE...H2H6x",
    amount: 800000,
    trustScore: 71.4
  },
  {
    owner: "F1Y2pYf4N2LvZgBGqVjYa3d4JcR4YqNtZx1mQzLk8RjZ",
    displayAddress: "F1Y2...8RjZ",
    amount: 650000,
    trustScore: 68.9
  },
  {
    owner: "H7ySQ9b3Rx4BfGvE6Qw2K8LpJ9sA5rR4WnZx7mK2VgLt",
    displayAddress: "H7yS...VgLt",
    amount: 550000,
    trustScore: 65.1
  }
];

const calculateTrustScore = (amount: number, minAmount: number = 100000): number => {
  const rawScore = amount === 0 ? 0 : Math.min(100, (amount / minAmount) * 10);
  return Number(rawScore.toFixed(1));
};

async function getAllPartners(): Promise<Partner[]> {
  // Return mock data sorted by amount in descending order
  return mockPartners.sort((a, b) => b.amount - a.amount);
}

export default async function handler(
  req: NextApiRequest,
  res: NextApiResponse
) {
  if (req.method !== 'GET') {
    return res.status(405).json({ error: 'Method not allowed' });
  }

  try {
    const allPartners = await getAllPartners();
    
    // Transform data to match expected PartnerData interface
    const formattedPartners = allPartners.map(partner => ({
      address: partner.owner,
      holdings: partner.amount,
      trustScore: partner.trustScore
    }));

    console.log(`Returning ${formattedPartners.length} mock partners for trust scoreboard`);

    // Return in expected ApiResponse format
    res.status(200).json({ 
      partners: formattedPartners 
    });

  } catch (error) {
    console.error('API Error:', error);
    res.status(500).json({ 
      error: 'Failed to fetch partner accounts',
      details: error instanceof Error ? error.message : 'Unknown error'
    });
  }
}

export { getAllPartners };