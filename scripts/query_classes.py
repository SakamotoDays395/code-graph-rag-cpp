import sqlite3

db_path = r'C:\Users\N sathvik\.code-graph-rag\codegraph.db'
conn = sqlite3.connect(db_path)
conn.row_factory = sqlite3.Row
cursor = conn.cursor()

cursor.execute("SELECT name, type, file_path FROM entities WHERE type = 'class' LIMIT 20")
for row in cursor.fetchall():
    print(f"{row['name']} ({row['type']}) in {row['file_path']}")

print("\n--- Parsers ---")
cursor.execute("SELECT name, type, file_path FROM entities WHERE file_path LIKE '%parser%' OR file_path LIKE '%analyzer%' LIMIT 20")
for row in cursor.fetchall():
    print(f"{row['name']} ({row['type']}) in {row['file_path']}")
