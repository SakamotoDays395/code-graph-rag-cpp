import sqlite3

db_path = r'C:\Users\N sathvik\.code-graph-rag\codegraph.db'
conn = sqlite3.connect(db_path)
conn.row_factory = sqlite3.Row
cursor = conn.cursor()

cursor.execute("SELECT DISTINCT file_path FROM entities LIMIT 10")
for row in cursor.fetchall():
    print(f"{row['file_path']}")
