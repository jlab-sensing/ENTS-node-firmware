# LZMA.py

import lzma


class LZMACompressor:
    def compress(self, input_file_path, output_file_path=None):
        with open(input_file_path, "rb") as input_file:
            data = input_file.read()

        compressed_data = lzma.compress(data)

        if output_file_path:
            with open(output_file_path, "wb") as output_file:
                output_file.write(compressed_data)

        return compressed_data

    def decompress(self, input_file_path, output_file_path=None):
        with open(input_file_path, "rb") as input_file:
            compressed_data = input_file.read()

        decompressed_data = lzma.decompress(compressed_data)

        if output_file_path:
            with open(output_file_path, "wb") as output_file:
                output_file.write(decompressed_data)

        return decompressed_data
