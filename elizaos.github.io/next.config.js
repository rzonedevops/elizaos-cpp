/** @type {import('next').NextConfig} */
const nextConfig = {
  // Note: Static export is disabled until the database is populated with users
  // Enable this after running the pipeline to ingest GitHub data:
  // output: "export",
  images: {
    unoptimized: true, // Keep this for future static export
  },
  typescript: {
    // Using a custom tsconfig for the Next.js app
    tsconfigPath: "tsconfig.nextjs.json",
  },
};

export default nextConfig;
