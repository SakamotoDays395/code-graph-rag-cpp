import sqlite3
import json

db_path = r'C:\Users\N sathvik\.code-graph-rag\codegraph.db'
conn = sqlite3.connect(db_path)
conn.row_factory = sqlite3.Row
cursor = conn.cursor()

print("--- 1. God Class ---")
cursor.execute('''
    SELECT target_id, COUNT(*) as incoming_count
    FROM relationships
    WHERE relationship_type IN ('calls', 'depends_on', 'uses', 'instantiates')
    GROUP BY target_id
    ORDER BY incoming_count DESC
    LIMIT 5
''')
for row in cursor.fetchall():
    cursor.execute('SELECT name, type FROM entities WHERE id = ?', (row['target_id'],))
    target = cursor.fetchone()
    print(f"{target['name']} ({target['type']}): {row['incoming_count']} incoming dependencies")

print("\n--- 3. Language Analyzer with Most Internal Methods ---")
cursor.execute('''
    SELECT e.name, COUNT(r.id) as method_count
    FROM entities e
    JOIN relationships r ON e.id = r.source_id
    WHERE e.type = 'class' AND e.name LIKE '%Analyzer' AND r.relationship_type = 'contains'
    GROUP BY e.name
    ORDER BY method_count DESC
    LIMIT 5
''')
for row in cursor.fetchall():
    print(f"{row['name']}: {row['method_count']} contained elements (methods/fields)")

