from arithmetic_compressor import AECompressor
from arithmetic_compressor.models import StaticModel


class ArithmeticEncodingCompressor:
    """
    Arithmetic Encoding Compressor and Decompressor using the arithmetic_compressor package.
    """

    def __init__(self):
        """
        Initialize the ArithmeticEncodingCompressor with a static model for byte values (0–255).
        """
        # Define a uniform probability distribution for all byte values (0–255)
        symbol_possibilities = {i: 1 / 256 for i in range(256)}
        self.model = StaticModel(symbol_possibilities)

    def compress(self, input_file_path, output_file_path=None):
        """
        Compresses the input file using Arithmetic Encoding.

        Args:
            input_file_path (str): Path to the input file to compress.
            output_file_path (str, optional): Path to save the compressed file. Defaults to None.

        Returns:
            list: Compressed data as a list of bits.
        """
        with open(input_file_path, "rb") as input_file:
            data = input_file.read()

        # Create an arithmetic encoder
        coder = AECompressor(self.model)

        # Compress the data
        compressed_data = coder.compress(data)

        # Save the compressed data if an output file is specified
        if output_file_path:
            with open(output_file_path, "w") as output_file:
                output_file.write(" ".join(map(str, compressed_data)))

        return compressed_data

    def decompress(self, input_file_path, output_file_path=None):
        """
        Decompresses the input file using Arithmetic Encoding.

        Args:
            input_file_path (str): Path to the compressed file to decompress.
            output_file_path (str, optional): Path to save the decompressed file. Defaults to None.

        Returns:
            bytes: Decompressed data as bytes.
        """
        with open(input_file_path, "r") as input_file:
            compressed_data = list(map(int, input_file.read().split()))

        # Create an arithmetic decoder
        decoder = AECompressor(self.model)

        # Decompress the data
        decompressed_data = decoder.decompress(compressed_data)

        # Save the decompressed data if an output file is specified
        if output_file_path:
            with open(output_file_path, "wb") as output_file:
                output_file.write(decompressed_data)

        return decompressed_data
