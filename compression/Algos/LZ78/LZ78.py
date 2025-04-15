class LZ78Compressor:
    def compress(self, input_file_path, output_file_path=None):
        with open(input_file_path, 'rb') as input_file:
            text = input_file.read()

        dictionary = {}
        current = b''  # Use bytes instead of strings
        code = 1
        result = bytearray()  # Use bytearray to store binary data

        for char in text:
            char = bytes([char])  # Convert the integer to a single-byte bytes object
            temp = current + char
            if temp in dictionary:
                current = temp
            else:
                if current == b'':
                    result.extend(b'0' + char)  # Add '0' + char to the result
                else:
                    result.extend(dictionary[current] + char)  # Add dictionary index + char
                dictionary[temp] = bytes([code])  # Store the code as bytes
                code += 1
                current = b''

        if output_file_path:
            with open(output_file_path, 'wb') as output_file:  # Write in binary mode
                output_file.write(result)
        return result

    def decompress(self, input_file_path, output_file_path=None):
        with open(input_file_path, 'rb') as input_file:  # Read in binary mode
            encoded_data = input_file.read()

        dictionary = {b'0': b''}
        result = bytearray()
        i = 0
        code = 1

        while i < len(encoded_data):
            index = b''
            while i < len(encoded_data) and encoded_data[i:i+1].isdigit():
                index += encoded_data[i:i+1]
                i += 1

            if i < len(encoded_data):
                char = encoded_data[i:i+1]
                i += 1
                entry = dictionary[index] + char
                result.extend(entry)
                dictionary[bytes([code])] = entry
                code += 1

        if output_file_path:
            with open(output_file_path, 'wb') as output_file:  # Write in binary mode
                output_file.write(result)
        return bytes(result)
