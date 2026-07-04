import json
import subprocess
import time
import sys

def encode_frame(payload):
    body = json.dumps(payload, separators=(",", ":")).encode("utf-8")
    header = f"Content-Length: {len(body)}\r\n\r\n".encode("ascii")
    return header + body

exe = r"D:\imp codes\code-graph-rag-cpp-ai\build\Debug\code_graph_rag_mcp.exe"
proc = subprocess.Popen(
    [exe],
    stdin=subprocess.PIPE,
    stdout=subprocess.PIPE,
    stderr=subprocess.PIPE,
)

init_msg = {
    "jsonrpc": "2.0",
    "id": 1,
    "method": "initialize",
    "params": {
        "protocolVersion": "2025-03-26",
        "capabilities": {},
        "clientInfo": {"name": "test", "version": "1.0"}
    }
}
proc.stdin.write(encode_frame(init_msg))
proc.stdin.flush()

index_msg = {
    "jsonrpc": "2.0",
    "id": 2,
    "method": "tools/call",
    "params": {
        "name": "index",
        "arguments": {
            "directory": r"C:\Users\N sathvik\Downloads\code-graph-rag-mcp-master\code-graph-rag-mcp-master",
            "files": [r"C:\Users\N sathvik\Downloads\code-graph-rag-mcp-master\code-graph-rag-mcp-master\src\index.ts"],
            "reset": True,
            "incremental": False,
            "fullScan": True
        }
    }
}
proc.stdin.write(encode_frame(index_msg))
proc.stdin.flush()

# Wait 5 seconds for the agent to parse the file before closing stdin!
print("Waiting for C++ agents to parse...")
time.sleep(5)
proc.stdin.close()
proc.wait()

print("STDERR:")
print(proc.stderr.read().decode('utf-8', errors='ignore'))
