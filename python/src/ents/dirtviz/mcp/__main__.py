"""CLI runner when invoked via `python -m ents.dirtviz.mcp`."""

import argparse
import sys

from ents.dirtviz.mcp.server import run_mcp_server


def main() -> None:
    parser = argparse.ArgumentParser(
        description="DirtViz MCP Server for AI tooling and direct API integration."
    )
    parser.add_argument(
        "--transport",
        choices=["stdio", "sse"],
        default="stdio",
        help="MCP transport type (default: stdio)",
    )
    parser.add_argument(
        "--host",
        default="0.0.0.0",
        help="Host address for SSE transport (default: 0.0.0.0)",
    )
    parser.add_argument(
        "--port",
        type=int,
        default=8001,
        help="Port number for SSE transport (default: 8001)",
    )
    parser.add_argument(
        "--url",
        default=None,
        help="Override DirtViz API base URL (default: https://dirtviz.jlab.ucsc.edu/api/)",
    )
    parser.add_argument(
        "--no-cache",
        action="store_true",
        help="Disable persistent caching of API responses",
    )

    args = parser.parse_args()

    run_mcp_server(
        transport=args.transport,
        host=args.host,
        port=args.port,
        url=args.url,
        no_cache=args.no_cache,
    )


if __name__ == "__main__":
    sys.exit(main())
