# ElizaOS C++ Leaderboard Setup

This directory contains the GitHub analytics/leaderboard system configured for the ZoneCog/elizaos-cpp repository.

## Initial Setup Complete ✅

The system has been configured and is ready to use:

- ✅ Pipeline configuration updated to track `ZoneCog/elizaos-cpp`
- ✅ Database schema initialized
- ✅ Build system working (Node.js compatible)
- ✅ Environment configuration created
- ✅ All dependencies installed

## Next Steps

1. **Add GitHub Token** (Required for data ingestion):
   ```bash
   # Edit .env file and add your GitHub token:
   GITHUB_TOKEN=your_github_personal_access_token_here
   ```

2. **Ingest GitHub Data**:
   ```bash
   npm run pipeline ingest
   ```

3. **Process and Generate Reports**:
   ```bash
   npm run pipeline process
   npm run pipeline export
   ```

4. **Enable Static Export** (for GitHub Pages):
   After data is available, uncomment the `output: "export"` line in `next.config.js`

## Development

Start the development server:
```bash
npm run dev
```

## Deployment

The system is configured for GitHub Pages deployment via the included GitHub Actions workflows.

## Configuration

- Repository tracked: `ZoneCog/elizaos-cpp`
- Database: SQLite with better-sqlite3 (Node.js compatible)
- UI: Next.js with Tailwind CSS
- Fonts: System fonts (no external dependencies)