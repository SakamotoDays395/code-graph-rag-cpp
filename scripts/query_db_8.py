import sqlite3

db_path = r'C:\Users\N sathvik\.code-graph-rag\codegraph.db'
conn = sqlite3.connect(db_path)
conn.row_factory = sqlite3.Row
cursor = conn.cursor()

cursor.execute("SELECT e.name, e.type, r.type FROM relationships r JOIN entities e ON r.to_id = e.id WHERE r.from_id IN (SELECT id FROM entities WHERE name = 'getSQLiteManagerOrThrow')")
for row in cursor.fetchall():
    print(f"-> {row['name']} ({row['type']})")
