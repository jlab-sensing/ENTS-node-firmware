"""Unit tests for dirtviz interface."""

import unittest

from ents.dirtviz.client import BackendClient, Cell

class TestDirtviz(unittest.TestCase):
    """Unit tests for dirtviz interface."""

    def setUp(self):
        """Initialize the backend client."""
        self.client = BackendClient()

    def test_cells_from_tag_id(self):
        """Test that cells_from_tag_id returns a list of Cell objects."""

        # test tag id
        tag_id = 2

        cells = self.client.cells_from_tag_id(tag_id)

        self.assertIsInstance(cells, list)
        for cell in cells:
            self.assertIsInstance(cell, Cell)

    def test_cells_from_tag_name(self):
        """Test that cells_from_tag_name returns a list of Cell objects."""

        # test tag name
        tag_name = "Santa Cruz"

        cells = self.client.cells_from_tag_name(tag_name)

        self.assertIsInstance(cells, list)
        for cell in cells:
            self.assertIsInstance(cell, Cell)

if __name__ == "__main__":
    unittest.main()
