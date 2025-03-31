class LZ78Encoder:
    def __init__(self):
        self.dictionary = {}
        self.current_index = 1  # Dictionary indices start from 1

    def encode(self, data):
        encoded_data = []
        phrase = ""
        
        for symbol in data:
            new_phrase = phrase + symbol
            if new_phrase in self.dictionary:
                phrase = new_phrase
            else:
                pointer = self.dictionary.get(phrase, 0)
                encoded_data.append((pointer, symbol))
                self.dictionary[new_phrase] = self.current_index
                self.current_index += 1
                phrase = ""
        
        if phrase:
            encoded_data.append((self.dictionary[phrase], ""))
        
        return encoded_data

class LZ78Decoder:
    def __init__(self):
        self.dictionary = {0: ""}  # Initialize dictionary with empty phrase
        self.current_index = 1

    def decode(self, encoded_data):
        decoded_string = ""
        
        for index, symbol in encoded_data:
            phrase = self.dictionary.get(index, "") + symbol
            decoded_string += phrase
            self.dictionary[self.current_index] = phrase
            self.current_index += 1
        
        return decoded_string

# Example usage
print("\nLZ78\n")
print("Simple String") # Simple string
data = "ABABABA"
encoder = LZ78Encoder()
compressed = encoder.encode(data)
print("Compressed:", data)

decoder = LZ78Decoder()
decompressed = decoder.decode(compressed)
print("Decompressed matches data:", (data == decompressed))

print("\nArbitrary Binary")# Arbitratry binary
test_str = "This is a binary" 
print("Compressed binary of: " + str(test_str))
binary = ''.join(format(ord(i), '08b') for i in test_str) # Converting String to binary

comp_binary = encoder.encode(binary)
decomp_binary = decoder.decode(comp_binary)
print("Decompressed matches binary:", (binary == decomp_binary))
