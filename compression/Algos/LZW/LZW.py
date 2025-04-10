from pathlib import Path

class LZWCompressor:
    def __init__(self):
        self.max_table_size = 4096  # Common max dictionary size for 12-bit LZW

    def compress(self, input_file_path, output_file_path=None, verbose=False):
        try:
            with open(input_file_path, 'rb') as f:
                data = f.read()
        except IOError:
            print("Could not open input file ...")
            raise

        # Initialize dictionary with single-byte entries
        dictionary = {bytes([i]): i for i in range(256)}
        string = b""
        code = 256
        compressed_data = []

        for symbol in data:
            symbol_byte = bytes([symbol])
            if string + symbol_byte in dictionary:
                string += symbol_byte
            else:
                compressed_data.append(dictionary[string])
                if len(dictionary) < self.max_table_size:
                    dictionary[string + symbol_byte] = code
                    code += 1
                string = symbol_byte

        if string:
            compressed_data.append(dictionary[string])

        # Write compressed data as 16-bit integers
        if output_file_path:
            try:
                with open(output_file_path, 'wb') as f:
                    for code in compressed_data:
                        f.write(code.to_bytes(2, byteorder='big'))
                if verbose:
                    print("File was compressed successfully and saved to output path ...")
                return None
            except IOError:
                print("Could not write to output file path. Please check if the path is correct ...")
                raise

        return compressed_data

    def decompress(self, input_file_path, output_file_path=None):
        try:
            with open(input_file_path, 'rb') as f:
                byte_data = f.read()
        except IOError:
            print("Could not open input file ...")
            raise

        # Convert byte stream to list of codes
        compressed_data = [int.from_bytes(byte_data[i:i+2], byteorder='big')
                           for i in range(0, len(byte_data), 2)]

        # Initialize dictionary
        dictionary = {i: bytes([i]) for i in range(256)}
        code = 256

        result = bytearray()
        string = dictionary[compressed_data[0]]
        result.extend(string)

        for k in compressed_data[1:]:
            if k in dictionary:
                entry = dictionary[k]
            elif k == code:
                entry = string + string[:1]
            else:
                raise ValueError("Bad compressed k: %s" % k)

            result.extend(entry)

            # Add new entry to the dictionary
            if len(dictionary) < self.max_table_size:
                dictionary[code] = string + entry[:1]
                code += 1

            string = entry

        if output_file_path:
            try:
                with open(output_file_path, 'wb') as f:
                    f.write(result)
                print("File was decompressed successfully and saved to output path ...")
                return None
            except IOError:
                print("Could not write to output file path. Please check if the path is correct ...")
                raise

        return result
