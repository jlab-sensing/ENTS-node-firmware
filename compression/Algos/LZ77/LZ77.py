class LZ77Encoder:
    def __init__(self, window_size=20, lookahead_buffer_size=15):
        self.window_size = window_size
        self.lookahead_buffer_size = lookahead_buffer_size

    def encode(self, data):
        encoded_data = []
        i = 0
        
        while i < len(data):
            match_distance = 0
            match_length = 0
            lookahead = min(self.lookahead_buffer_size, len(data) - i)
            
            for j in range(1, min(self.window_size, i) + 1):
                substring = data[i:i + lookahead]
                start_index = max(0, i - j)
                window = data[start_index:i]
                
                for k in range(lookahead, 0, -1):
                    if substring[:k] in window:
                        match_distance = j
                        match_length = k
                        break
                
            next_symbol = data[i + match_length] if i + match_length < len(data) else ""
            encoded_data.append((match_distance, match_length, next_symbol))
            i += match_length + 1
        
        return encoded_data
    
class LZ77Decoder:
    def __init__(self):
        pass

    def decode(self, encoded_data):
        decoded_string = []
        
        for distance, length, symbol in encoded_data:
            if distance > 0:
                start_index = len(decoded_string) - distance
                for _ in range(length):
                    decoded_string.append(decoded_string[start_index])
                    start_index += 1
            
            if symbol:  # Only add symbol if it's not an empty string
                decoded_string.append(symbol)
        
        return "".join(decoded_string)  # Convert list back to string

# Example usage
print("\nLZ77\n")
print("Simple String") # Simple string
data = "ABABABA"
encoder = LZ77Encoder()
compressed = encoder.encode(data)
print("Compressed:", data)

decoder = LZ77Decoder()
decompressed = decoder.decode(compressed)
print("Decompressed matches data:", decompressed)

print("\nArbitrary Binary")# Arbitratry binary
test_str = "This is a binary" 
print("Compressed binary of: " + str(test_str))
binary = ''.join(format(ord(i), '08b') for i in test_str) # Converting String to binary

comp_binary = encoder.encode(binary)
decomp_binary = decoder.decode(comp_binary)
print("Decompressed matches binary:", (binary == decomp_binary))
