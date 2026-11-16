from .client import BackendClient
from .plots import plot_data

__all__ = [
    "BackendClient",
    "create_mcp_server",
    "plot_data",
    "run_mcp_server",
]


def __getattr__(name: str):
    if name in ("create_mcp_server", "run_mcp_server"):
        from .mcp import create_mcp_server, run_mcp_server

        if name == "create_mcp_server":
            return create_mcp_server
        return run_mcp_server
    raise AttributeError(f"module {__name__!r} has no attribute {name!r}")

