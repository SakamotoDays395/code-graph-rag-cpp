import sqlite3
import json

db_path = r'C:\Users\N sathvik\.code-graph-rag\codegraph.db'
conn = sqlite3.connect(db_path)
conn.row_factory = sqlite3.Row
cursor = conn.cursor()

print("--- 1. God Class (Most incoming dependencies) ---")
cursor.execute('''
    SELECT to_id, COUNT(*) as incoming_count
    FROM relationships
    WHERE type IN ('calls', 'depends_on', 'uses', 'instantiates', 'references')
    GROUP BY to_id
    ORDER BY incoming_count DESC
    LIMIT 10
''')
for row in cursor.fetchall():
    cursor.execute('SELECT name, type, file_path FROM entities WHERE id = ?', (row['to_id'],))
    target = cursor.fetchone()
    if target:
        print(f"{target['name']} ({target['type']}) in {target['file_path']}: {row['incoming_count']} incoming dependencies")

print("\n--- 2. Trace relationship path from executeToolCall to SQLite ---")
# Let's find executeToolCall
cursor.execute("SELECT id FROM entities WHERE name = 'executeToolCall'")
row = cursor.fetchone()
if row:
    start_id = row['id']
    # A quick BFS in python
    visited = set([start_id])
    queue = [[start_id]]
    found = False
    
    cursor.execute("SELECT id FROM entities WHERE name LIKE '%sqlite%' OR file_path LIKE '%sqlite%'")
    target_ids = set([r['id'] for r in cursor.fetchall()])
    
    while queue and not found:
        path = queue.pop(0)
        curr = path[-1]
        
        cursor.execute("SELECT to_id FROM relationships WHERE from_id = ?", (curr,))
        for r in cursor.fetchall():
            nxt = r['to_id']
            if nxt in target_ids:
                path.append(nxt)
                found = True
                
                print("Path found:")
                for step in path:
                    cursor.execute("SELECT name, file_path FROM entities WHERE id = ?", (step,))
                    step_entity = cursor.fetchone()
                    if step_entity:
                        print(f" -> {step_entity['name']} ({step_entity['file_path']})")
                break
            if nxt not in visited:
                visited.add(nxt)
                new_path = list(path)
                new_path.append(nxt)
                queue.append(new_path)
        if len(visited) > 1000:
            print("BFS limit reached without finding path")
            break
else:
    print("executeToolCall not found")


print("\n--- 3. Language Analyzer with Most Internal Methods ---")
cursor.execute('''
    SELECT e.name, e.file_path, COUNT(r.id) as method_count
    FROM entities e
    JOIN relationships r ON e.id = r.from_id
    WHERE e.type = 'class' AND e.name LIKE '%Analyzer' AND r.type = 'contains'
    GROUP BY e.name
    ORDER BY method_count DESC
    LIMIT 5
''')
for row in cursor.fetchall():
    print(f"{row['name']} in {row['file_path']}: {row['method_count']} contained elements (methods/fields)")

