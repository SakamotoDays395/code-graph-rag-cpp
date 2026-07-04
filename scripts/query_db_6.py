import sqlite3

db_path = r'C:\Users\N sathvik\.code-graph-rag\codegraph.db'
conn = sqlite3.connect(db_path)
conn.row_factory = sqlite3.Row
cursor = conn.cursor()

cursor.execute("SELECT id FROM entities WHERE name = 'executeToolCall'")
start = cursor.fetchone()
cursor.execute("SELECT id FROM entities WHERE file_path LIKE '%storage%'")
target_ids = set([r['id'] for r in cursor.fetchall()])

if start and target_ids:
    start_id = start['id']
    
    visited = {start_id: None}
    queue = [start_id]
    end_id = None
    
    while queue:
        curr = queue.pop(0)
        if curr in target_ids:
            end_id = curr
            break
            
        cursor.execute("SELECT to_id FROM relationships WHERE from_id = ?", (curr,))
        for r in cursor.fetchall():
            nxt = r['to_id']
            if nxt not in visited:
                visited[nxt] = curr
                queue.append(nxt)
                
    if end_id in visited:
        path = []
        curr = end_id
        while curr is not None:
            path.append(curr)
            curr = visited[curr]
        path.reverse()
        
        print("Path from executeToolCall to storage layer:")
        for step in path:
            cursor.execute("SELECT name, type, file_path FROM entities WHERE id = ?", (step,))
            e = cursor.fetchone()
            print(f" -> {e['name']} ({e['type']}) in {e['file_path']}")
    else:
        print("No path found to storage layer.")
else:
    print("Start or targets not found.")
