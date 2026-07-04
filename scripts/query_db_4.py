import sqlite3

db_path = r'C:\Users\N sathvik\.code-graph-rag\codegraph.db'
conn = sqlite3.connect(db_path)
conn.row_factory = sqlite3.Row
cursor = conn.cursor()

cursor.execute("SELECT id FROM entities WHERE name = 'executeToolCall'")
start = cursor.fetchone()
cursor.execute("SELECT id FROM entities WHERE name = 'GraphStorageImpl'")
end = cursor.fetchone()

if start and end:
    start_id = start['id']
    end_id = end['id']
    
    visited = {start_id: None}
    queue = [start_id]
    
    while queue:
        curr = queue.pop(0)
        if curr == end_id:
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
        
        print("Path from executeToolCall to GraphStorageImpl:")
        for step in path:
            cursor.execute("SELECT name, type, file_path FROM entities WHERE id = ?", (step,))
            e = cursor.fetchone()
            print(f" -> {e['name']} ({e['type']}) in {e['file_path']}")
    else:
        print("No path found.")
else:
    print("Start or end not found.")
