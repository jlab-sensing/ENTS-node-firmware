# LZ78.py

class LZ78Compressor:
    def encode_varint(self, number):
        """Encode an integer into varint bytes."""
        result = bytearray()
        while True:
            byte = number & 0x7F
            number >>= 7
            if number:
                result.append(byte | 0x80)  # set continuation bit
            else:
                result.append(byte)
                break
        return bytes(result)

    def decode_varint(self, data, start):
        """Decode a varint from data starting at index 'start'.
        Returns a tuple (number, new_index)."""
        result = 0
        shift = 0
        i = start
        while True:
            byte = data[i]
            result |= (byte & 0x7F) << shift
            i += 1
            if not (byte & 0x80):
                break
            shift += 7
        return result, i

    def compress(self, input_file_path, output_file_path=None):
        """Compress the input file using LZ78 and write the result to output_file_path if provided.
        
        The output format is a sequence of (index, character) pairs:
          - 'index' is stored as a varint.
          - 'character' is stored as a single byte.
        """
        with open(input_file_path, 'rb') as input_file:
            data = input_file.read()

        dictionary = {}  # Maps phrases (bytes) to integer indices.
        current = b''
        code = 1  # Next available dictionary index.
        result = bytearray()

        for byte in data:
            char = bytes([byte])
            candidate = current + char
            if candidate in dictionary:
                current = candidate
            else:
                # If current is empty, we output index 0; otherwise, the dictionary index for current.
                index = dictionary.get(current, 0)
                result.extend(self.encode_varint(index))
                result.extend(char)
                dictionary[candidate] = code
                code += 1
                current = b''

        # Optionally, if there's a leftover current phrase without an accompanying char, you can handle it here.
        # Standard LZ78 doesn't require this because every new dictionary entry is output with a new char.

        if output_file_path:
            with open(output_file_path, 'wb') as output_file:
                output_file.write(result)
        return bytes(result)

    def decompress(self, input_file_path, output_file_path=None):
        """Decompress the file compressed with LZ78 and write the result to output_file_path if provided.
        
        It reads a sequence of (index, character) pairs where the index is encoded as a varint.
        """
        with open(input_file_path, 'rb') as input_file:
            data = input_file.read()

        dictionary = {0: b''}  # Maps indices to phrases.
        result = bytearray()
        i = 0
        code = 1

        while i < len(data):
            # Decode the varint-encoded index.
            index, i = self.decode_varint(data, i)
            if i < len(data):
                char = data[i:i+1]
                i += 1
            else:
                break  # No character follows; input might be corrupted.
            phrase = dictionary.get(index, b'') + char
            result.extend(phrase)
            dictionary[code] = phrase
            code += 1

        if output_file_path:
            with open(output_file_path, 'wb') as output_file:
                output_file.write(result)
        return bytes(result)

