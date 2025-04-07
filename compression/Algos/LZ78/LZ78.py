class LZ78Compressor:
    def compress(self, input_file_path, output_file_path=None):
        with open(input_file_path, 'r') as input_file:
            text = input_file.read()

        dictionary = {}
        current = ''
        code = 1
        result = []

        for char in text:
            temp = current + char
            if temp in dictionary:
                current = temp
            else:
                if current == '':
                    result.append(f'0{char}')
                else:
                    result.append(f'{dictionary[current]}{char}')
                dictionary[temp] = str(code)
                code += 1
                current = ''

        if output_file_path:
            with open(output_file_path, 'w') as output_file:
                output_file.write(''.join(result))
        return ''.join(result)

    def decompress(self, input_file_path, output_file_path=None):
        with open(input_file_path, 'r') as input_file:
            encoded_text = input_file.read()

        dictionary = {'0': ''}
        result = []
        i = 0
        code = 1

        while i < len(encoded_text):
            index = ''
            while i < len(encoded_text) and encoded_text[i].isdigit():
                index += encoded_text[i]
                i += 1

            if i < len(encoded_text):
                char = encoded_text[i]
                i += 1
                entry = dictionary[index] + char
                result.append(entry)
                dictionary[str(code)] = entry
                code += 1

        decoded_text = ''.join(result)
        if output_file_path:
            with open(output_file_path, 'w') as output_file:
                output_file.write(decoded_text)
        return decoded_text
