import rle

class RunLengthEncodingCompressor:
    """
    Run-Length Encoding (RLE) Compressor and Decompressor using the python-rle package.
    """

    def compress(self, input_file_path, output_file_path=None):
        """
        Compresses the input file using Run-Length Encoding (RLE).

        Args:
            input_file_path (str): Path to the input file to compress.
            output_file_path (str, optional): Path to save the compressed file. Defaults to None.

        Returns:
            tuple: A tuple containing two lists: (values, counts).
        """
        with open(input_file_path, 'rb') as input_file:
            data = list(input_file.read())  # Read binary data as a list of integers

        # Use python-rle to encode the data
        values, counts = rle.encode(data)

        # Save the compressed data if an output file is specified
        if output_file_path:
            with open(output_file_path, 'wb') as output_file:
                # Write values and counts as binary data
                output_file.write(bytes(values) + b'\x00' + bytes(counts))

        return values, counts

    def decompress(self, input_file_path, output_file_path=None):
        """
        Decompresses the input file using Run-Length Encoding (RLE).

        Args:
            input_file_path (str): Path to the compressed file to decompress.
            output_file_path (str, optional): Path to save the decompressed file. Defaults to None.

        Returns:
            bytes: Decompressed data.
        """
        with open(input_file_path, 'rb') as input_file:
            # Read the compressed data
            compressed_data = input_file.read()

        # Split the data into values and counts
        split_index = compressed_data.index(0)  # Find the separator (0x00)
        values = list(compressed_data[:split_index])
        counts = list(compressed_data[split_index + 1:])

        # Use python-rle to decode the data
        decompressed_data = rle.decode(values, counts)

        # Save the decompressed data if an output file is specified
        if output_file_path:
            with open(output_file_path, 'wb') as output_file:
                output_file.write(bytes(decompressed_data))

        return bytes(decompressed_data)