import sqlite3
import json
conn = sqlite3.connect(r'C:\Users\N sathvik\.code-graph-rag\codegraph.db')
cursor = conn.cursor()

print('\n### Q1: The God Class / Bottleneck (Highest Incoming Dependencies)')
cursor.execute('''
SELECT e.name, e.type, COUNT(*) as incoming 
FROM relationships r
JOIN entities e ON r.to_id = e.id
WHERE r.type IN ('depends_on', 'calls', 'references')
GROUP BY e.id
ORDER BY incoming DESC 
LIMIT 3
''')
for row in cursor.fetchall():
    print(f"- **{row[0]}** ({row[1]}): {row[2]} incoming connections")

print('\n### Q2: Architectural Trace for executeToolCall')
cursor.execute('''
SELECT e1.name, r.type, e2.name 
FROM relationships r
JOIN entities e1 ON r.from_id = e1.id
JOIN entities e2 ON r.to_id = e2.id
WHERE e1.name LIKE '%executeToolCall%' OR e2.name LIKE '%executeToolCall%'
LIMIT 5
''')
rows = cursor.fetchall()
if not rows:
    print("- No direct matches found for 'executeToolCall' (it might be named differently or the file wasn't fully indexed).")
else:
    for row in rows:
        print(f"- {row[0]} --[{row[1]}]--> {row[2]}")

print('\n### Q3: AST Parser Complexity (Analyzer Methods)')
cursor.execute('''
SELECT e1.name, COUNT(r.id) as method_count 
FROM entities e1
JOIN relationships r ON e1.id = r.from_id
JOIN entities e2 ON r.to_id = e2.id
WHERE e1.name LIKE '%Analyzer%' AND e2.type IN ('method', 'function') AND r.type = 'contains'
GROUP BY e1.id
ORDER BY method_count DESC 
LIMIT 3
''')
rows = cursor.fetchall()
if not rows:
    print("- No Analyzer methods found (the parsers might not have successfully indexed TS classes).")
else:
    for row in rows:
        print(f"- **{row[0]}**: {row[1]} methods")

