import Database from "better-sqlite3";
import { drizzle } from "drizzle-orm/better-sqlite3";
import path from "path";
import * as schema from "./schema";

// Initialize SQLite database with WAL mode for better performance
let sqlite: Database.Database;
try {
  sqlite = new Database(path.join(process.cwd(), "data/db.sqlite"), {});
  sqlite.pragma("journal_mode = WAL");
} catch (error) {
  console.error("Failed to initialize database:", error);
  process.exit(1);
}

// Create drizzle database instance
export const db = drizzle(sqlite, { schema });

// Ensure database is closed when the process exits
process.on("exit", () => {
  sqlite.close();
});
