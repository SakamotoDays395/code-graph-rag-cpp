import json
import subprocess
import time
import sys
import threading

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

def stream_reader(pipe, name):
    for line in iter(pipe.readline, b''):
        sys.stderr.buffer.write(f"[{name}] ".encode() + line)
        sys.stderr.buffer.flush()

threading.Thread(target=stream_reader, args=(proc.stderr, "ERR"), daemon=True).start()
threading.Thread(target=stream_reader, args=(proc.stdout, "OUT"), daemon=True).start()

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
            "reset": True,
            "incremental": False,
            "fullScan": True
        }
    }
}
proc.stdin.write(encode_frame(index_msg))
proc.stdin.flush()

# Give it up to 30 seconds to parse and send response!
time.sleep(25)
proc.stdin.close()
proc.wait()
