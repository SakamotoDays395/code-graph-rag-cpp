#!/usr/bin/env python3

import argparse
import json
import os
import pathlib
import subprocess
import sys
import tempfile
from typing import Any


def encode_frame(payload: dict[str, Any]) -> str:
    body = json.dumps(payload, separators=(",", ":"), ensure_ascii=False)
    header = f"Content-Length: {len(body.encode('utf-8'))}\r\n\r\n"
    return header + body


def decode_frames(buffer: str) -> list[dict[str, Any]]:
    offset = 0
    frames: list[dict[str, Any]] = []

    while offset < len(buffer):
        header_separator = "\r\n\r\n"
        header_end = buffer.find(header_separator, offset)
        if header_end < 0:
            header_separator = "\n\n"
            header_end = buffer.find(header_separator, offset)
        if header_end < 0:
            raise RuntimeError("Incomplete JSON-RPC header block in stdout payload")

        headers: dict[str, str] = {}
        raw_headers = buffer[offset:header_end].replace("\r\n", "\n")
        for raw_line in raw_headers.split("\n"):
            name, _, value = raw_line.partition(":")
            headers[name.strip().lower()] = value.strip()

        try:
            content_length = int(headers["content-length"])
        except (KeyError, ValueError) as exc:
            raise RuntimeError(f"Invalid JSON-RPC headers: {headers}") from exc

        body_start = header_end + len(header_separator)
        body_end = body_start + content_length
        if body_end > len(buffer):
            raise RuntimeError("Incomplete JSON-RPC body in stdout payload")

        frames.append(json.loads(buffer[body_start:body_end]))
        offset = body_end

    return frames


def merge_index_args(args: argparse.Namespace, payload: dict[str, Any]) -> dict[str, Any]:
    if args.directory:
        payload["directory"] = args.directory
    if args.file:
        payload["files"] = args.file
    if args.reset:
        payload["reset"] = True
    if args.incremental:
        payload["incremental"] = True
    if args.full_scan:
        payload["fullScan"] = True
    return payload


def read_text_auto(path: pathlib.Path) -> str:
    if not path.exists():
        return ""

    raw = path.read_bytes()
    for encoding in ("utf-8", "utf-8-sig", "utf-16", "utf-16-le", "utf-16-be"):
        try:
            return raw.decode(encoding)
        except UnicodeDecodeError:
            continue
    return raw.decode("latin-1")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Minimal MCP stdio client for the local C++ server")
    parser.add_argument("tool", help="Tool name to call, for example: index, get_graph, resolve_entity")
    parser.add_argument("--exe", default=os.path.join("build", "Debug", "code_graph_rag_mcp.exe"), help="Path to the MCP server executable")
    parser.add_argument("--workspace", default=os.getcwd(), help="Workspace root to expose to the MCP server")
    parser.add_argument("--args-json", default="{}", help="Raw JSON object merged into the tool arguments")
    parser.add_argument("--directory", help="Convenience override for index.directory")
    parser.add_argument("--file", action="append", help="Convenience override for index.files; may be provided multiple times")
    parser.add_argument("--reset", action="store_true", help="Convenience override for index.reset")
    parser.add_argument("--incremental", action="store_true", help="Convenience override for index.incremental")
    parser.add_argument("--full-scan", action="store_true", help="Convenience override for index.fullScan")
    return parser.parse_args()


def main() -> int:
    args = parse_args()

    try:
        payload = json.loads(args.args_json)
    except json.JSONDecodeError as exc:
        raise SystemExit(f"Invalid --args-json payload: {exc}")

    if not isinstance(payload, dict):
        raise SystemExit("--args-json must decode to a JSON object")

    if args.tool == "index":
        payload = merge_index_args(args, payload)

    env = os.environ.copy()
    env["MCP_WORKSPACE_ROOT"] = args.workspace

    request_text = "".join(
        (
            encode_frame(
                {
                    "jsonrpc": "2.0",
                    "id": 1,
                    "method": "initialize",
                    "params": {
                        "protocolVersion": "2025-03-26",
                        "clientInfo": {"name": "mcp_debug.py", "version": "1.0.0"},
                        "capabilities": {},
                    },
                }
            ),
            encode_frame({"jsonrpc": "2.0", "method": "notifications/initialized", "params": {}}),
            encode_frame({"jsonrpc": "2.0", "id": 2, "method": "tools/call", "params": {"name": args.tool, "arguments": payload}}),
        )
    )

    with tempfile.TemporaryDirectory(prefix="code-graph-mcp-") as tmp_dir:
        tmp_path = pathlib.Path(tmp_dir)
        request_path = tmp_path / "request.txt"
        stdout_path = tmp_path / "stdout.txt"
        stderr_path = tmp_path / "stderr.txt"
        request_path_ps = str(request_path).replace("'", "''")
        stdout_path_ps = str(stdout_path).replace("'", "''")
        stderr_path_ps = str(stderr_path).replace("'", "''")
        exe_path_ps = args.exe.replace("'", "''")

        with request_path.open("w", encoding="utf-8", newline="") as handle:
            handle.write(request_text)

        with request_path.open("rb") as f_in, stdout_path.open("wb") as f_out, stderr_path.open("wb") as f_err:
            subprocess.run(
                [args.exe],
                stdin=f_in,
                stdout=f_out,
                stderr=f_err,
                env=env,
                check=False,
            )

        completed_stdout = read_text_auto(stdout_path)
        completed_stderr = read_text_auto(stderr_path)

    if completed_stderr:
        sys.stderr.write(completed_stderr)

    frames = decode_frames(completed_stdout)
    by_id = {frame.get("id"): frame for frame in frames if isinstance(frame, dict) and frame.get("id") is not None}

    initialize = by_id.get(1)
    if initialize is None:
        if completed_stdout:
            print(repr(completed_stdout[:500]), file=sys.stderr)
        raise SystemExit("Missing initialize response from MCP server")
    if "error" in initialize:
        print(json.dumps(initialize, indent=2), file=sys.stderr)
        return 1

    response = by_id.get(2)
    if response is None:
        raise SystemExit("Missing tool response from MCP server")

    print(json.dumps(response, indent=2))
    return 0 if "error" not in response else 1


if __name__ == "__main__":
    raise SystemExit(main())
