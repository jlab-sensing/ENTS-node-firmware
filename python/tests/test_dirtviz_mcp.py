"""Unit tests for DirtViz MCP server and tools."""

import tempfile
import unittest
from datetime import UTC, datetime, timedelta
from unittest.mock import MagicMock, patch

from ents.dirtviz.client import BackendClient
from ents.dirtviz.mcp.server import create_mcp_server
from ents.dirtviz.mcp.tools import (
    get_cell,
    get_cell_availability,
    get_cells_by_tag,
    get_cells_with_recent_data,
    list_cell_sensors,
    list_cells,
)


class TestDirtvizMCP(unittest.TestCase):
    """Test suite for DirtViz MCP tools and server."""

    def setUp(self):
        """Set up test fixtures."""
        self.temp_dir = tempfile.TemporaryDirectory()
        self.client = BackendClient(
            base_url="http://mock-api.local/api/",
            cache_dir=self.temp_dir.name,
        )

        self.mock_cells_data = [
            {
                "id": 1,
                "name": "Garden_Plot_1",
                "location": "Community Garden",
                "latitude": 37.0,
                "longitude": -122.0,
                "archive": False,
            },
            {
                "id": 2,
                "name": "Forest_Sensor_2",
                "location": "Redwood Grove",
                "latitude": 37.1,
                "longitude": -122.1,
                "archive": False,
            },
            {
                "id": 3,
                "name": "Old_Decommissioned_Cell",
                "location": "Greenhouse B",
                "latitude": 37.2,
                "longitude": -122.2,
                "archive": True,
            },
        ]

    def tearDown(self):
        """Clean up temporary directory."""
        self.temp_dir.cleanup()

    @patch("requests.get")
    def test_cell_resolution(self, mock_get):
        """Test resolve_cell with numeric IDs, exact names, and case-insensitive names."""
        mock_resp = MagicMock()
        mock_resp.json.return_value = self.mock_cells_data
        mock_get.return_value = mock_resp

        # By int ID
        c1 = self.client.resolve_cell(1)
        self.assertEqual(c1.id, 1)
        self.assertEqual(c1.name, "Garden_Plot_1")

        # By string digit ID
        c2 = self.client.resolve_cell("2")
        self.assertEqual(c2.id, 2)
        self.assertEqual(c2.name, "Forest_Sensor_2")

        # By exact name
        c_exact = self.client.resolve_cell("Garden_Plot_1")
        self.assertEqual(c_exact.id, 1)

        # By case-insensitive name
        c_lower = self.client.resolve_cell("forest_sensor_2")
        self.assertEqual(c_lower.id, 2)

        # Non-existent cell raises ValueError
        with self.assertRaises(ValueError) as ctx:
            self.client.resolve_cell("Non_Existent_Cell")
        self.assertIn("Non_Existent_Cell", str(ctx.exception))
        self.assertIn("Available cells", str(ctx.exception))

    @patch("requests.get")
    def test_list_cells_tool(self, mock_get):
        """Test list_cells tool with and without archived cells."""
        mock_resp = MagicMock()
        mock_resp.json.return_value = self.mock_cells_data
        mock_get.return_value = mock_resp

        # Without archive (default)
        active = list_cells(self.client, archive=False)
        self.assertEqual(len(active), 2)
        self.assertEqual(active[0]["name"], "Garden_Plot_1")
        self.assertEqual(active[1]["name"], "Forest_Sensor_2")

        # With archive
        all_cells = list_cells(self.client, archive=True)
        self.assertEqual(len(all_cells), 3)
        self.assertTrue(any(c["archive"] for c in all_cells))

    @patch("requests.get")
    def test_get_cell_tool(self, mock_get):
        """Test get_cell tool."""
        mock_resp = MagicMock()
        mock_resp.json.return_value = self.mock_cells_data
        mock_get.return_value = mock_resp

        cell_info = get_cell(self.client, "garden_plot_1")
        self.assertEqual(cell_info["id"], 1)
        self.assertEqual(cell_info["name"], "Garden_Plot_1")
        self.assertEqual(cell_info["location"], "Community Garden")

    @patch("requests.get")
    def test_list_cell_sensors_tool(self, mock_get):
        """Test list_cell_sensors tool."""
        mock_sensor_data = [
            {
                "id": 10,
                "name": "power",
                "measurement": "POWER_VOLTAGE",
                "data_type": "float",
                "unit": "mV",
            },
            {
                "id": 11,
                "name": "teros",
                "measurement": "TEROS12_VWC",
                "data_type": "float",
                "unit": "%",
            },
        ]

        def side_effect(url, params=None):
            resp = MagicMock()
            if "cell/id" in url:
                resp.json.return_value = self.mock_cells_data
            elif "sensors" in url:
                resp.json.return_value = mock_sensor_data
            else:
                resp.json.return_value = {}
            return resp

        mock_get.side_effect = side_effect

        sensors = list_cell_sensors(self.client, "Garden_Plot_1")
        self.assertEqual(len(sensors), 2)
        self.assertEqual(sensors[0]["measurement"], "POWER_VOLTAGE")
        self.assertEqual(sensors[1]["measurement"], "TEROS12_VWC")

    @patch("requests.get")
    def test_get_cell_availability_tool(self, mock_get):
        """Test get_cell_availability tool."""
        now_iso = datetime.now(UTC).isoformat()
        two_months_ago_iso = (
            datetime.now(UTC) - timedelta(days=60)
        ).isoformat()

        def side_effect(url, params=None):
            resp = MagicMock()
            if "cell/id" in url:
                resp.json.return_value = self.mock_cells_data
            elif "data-availability" in url:
                resp.json.return_value = {
                    "earliest_timestamp": two_months_ago_iso,
                    "latest_timestamp": now_iso,
                    "has_recent_data": True,
                }
            return resp

        mock_get.side_effect = side_effect

        avail = get_cell_availability(self.client, 1)
        self.assertEqual(avail["cell_id"], 1)
        self.assertEqual(avail["cell_name"], "Garden_Plot_1")
        self.assertEqual(avail["latest_timestamp"], now_iso)
        self.assertTrue(avail["has_recent_data"])

    @patch("requests.get")
    def test_get_cells_with_recent_data_tool(self, mock_get):
        """Test get_cells_with_recent_data filters cells active in the last 24h."""
        now = datetime.now(UTC)
        recent_ts = (now - timedelta(hours=2)).isoformat()
        old_ts = (now - timedelta(days=10)).isoformat()

        def side_effect(url, params=None):
            resp = MagicMock()
            if "cell/id" in url:
                resp.json.return_value = self.mock_cells_data
            elif "data-availability" in url:
                cell_id = params.get("cell_ids")
                if cell_id == "1":
                    resp.json.return_value = {
                        "earliest_timestamp": (now - timedelta(days=30)).isoformat(),
                        "latest_timestamp": recent_ts,
                        "has_recent_data": True,
                    }
                elif cell_id == "2":
                    resp.json.return_value = {
                        "earliest_timestamp": (now - timedelta(days=30)).isoformat(),
                        "latest_timestamp": old_ts,
                        "has_recent_data": False,
                    }
                else:
                    resp.json.return_value = {
                        "earliest_timestamp": None,
                        "latest_timestamp": None,
                        "has_recent_data": False,
                    }
            return resp

        mock_get.side_effect = side_effect

        recent = get_cells_with_recent_data(self.client, hours=24)
        self.assertEqual(len(recent), 1)
        self.assertEqual(recent[0]["cell_id"], 1)
        self.assertEqual(recent[0]["cell_name"], "Garden_Plot_1")
        self.assertAlmostEqual(recent[0]["hours_ago"], 2.0, delta=0.5)

    @patch("requests.get")
    def test_get_cells_by_tag_tool(self, mock_get):
        """Test get_cells_by_tag tool."""

        def side_effect(url, params=None):
            resp = MagicMock()
            if "tags/5/cells" in url:
                resp.json.return_value = {"cells": [self.mock_cells_data[0]]}
            elif "tag/" in url:
                resp.json.return_value = [
                    {"id": 5, "name": "Santa Cruz"},
                    {"id": 6, "name": "Davis"},
                ]
            return resp

        mock_get.side_effect = side_effect

        # By tag name
        cells_by_name = get_cells_by_tag(self.client, "Santa Cruz")
        self.assertEqual(len(cells_by_name), 1)
        self.assertEqual(cells_by_name[0]["id"], 1)

        # By tag ID
        cells_by_id = get_cells_by_tag(self.client, 5)
        self.assertEqual(len(cells_by_id), 1)
        self.assertEqual(cells_by_id[0]["id"], 1)

    def test_create_mcp_server(self):
        """Test server creation and tool registration."""
        server = create_mcp_server(client=self.client)
        self.assertIsNotNone(server)

        # Verify registered tools
        import asyncio

        tools = asyncio.run(server.list_tools())
        tool_names = [t.name for t in tools]

        expected_tools = [
            "list_cells",
            "get_cell",
            "list_cell_sensors",
            "get_cell_availability",
            "get_cells_with_recent_data",
            "get_cells_by_tag",
        ]
        for expected in expected_tools:
            self.assertIn(expected, tool_names)

    def test_server_instructions_and_resources(self):
        """Test server instructions and scripting guide resource registration."""
        server = create_mcp_server(client=self.client)
        self.assertIsNotNone(server.instructions)
        self.assertIn("AI Discovery & Analysis Script Synthesis Guide", server.instructions)
        self.assertIn("BackendClient", server.instructions)

        if hasattr(server, "list_resources"):
            import asyncio

            resources = asyncio.run(server.list_resources())
            resource_uris = [str(r.uri) for r in resources]
            self.assertIn("dirtviz://docs/scripting_guide", resource_uris)

    def test_server_prompts(self):
        """Test prompt template registration and generation."""
        server = create_mcp_server(client=self.client)
        if hasattr(server, "list_prompts") and hasattr(server, "get_prompt"):
            import asyncio

            prompts = asyncio.run(server.list_prompts())
            prompt_names = [p.name for p in prompts]
            for p_name in [
                "generate_analysis_script",
                "compare_cells_script",
                "pca_analysis_script",
            ]:
                self.assertIn(p_name, prompt_names)

            # Test prompt execution
            p_analysis = asyncio.run(
                server.get_prompt("generate_analysis_script", {"cell": "sc_synth_open_1"})
            )
            analysis_text = str(p_analysis.messages[0].content)
            self.assertIn("sc_synth_open_1", analysis_text)
            self.assertIn("BackendClient", analysis_text)

            p_compare = asyncio.run(
                server.get_prompt(
                    "compare_cells_script",
                    {"cells": "sc_synth_open_1,sc_synth_open_2", "tag": "Santa Cruz"},
                )
            )
            compare_text = str(p_compare.messages[0].content)
            self.assertIn("Santa Cruz", compare_text)
            self.assertIn("quantiles", compare_text)

            p_pca = asyncio.run(
                server.get_prompt("pca_analysis_script", {"cells": "cell1,cell2"})
            )
            pca_text = str(p_pca.messages[0].content)
            self.assertIn("Principal Component Analysis", pca_text)
            self.assertIn("StandardScaler", pca_text)


if __name__ == "__main__":
    unittest.main()
