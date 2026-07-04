import sqlite3

db_path = r'C:\Users\N sathvik\.code-graph-rag\codegraph.db'
conn = sqlite3.connect(db_path)
conn.row_factory = sqlite3.Row
cursor = conn.cursor()

cursor.execute("SELECT id FROM entities WHERE name = 'executeToolCall'")
start = cursor.fetchone()

if start:
    start_id = start['id']
    cursor.execute("SELECT e.name, e.type, r.type as rel_type FROM relationships r JOIN entities e ON r.to_id = e.id WHERE r.from_id = ?", (start_id,))
    for row in cursor.fetchall():
        print(f"-> {row['name']} ({row['type']}) [{row['rel_type']}]")
