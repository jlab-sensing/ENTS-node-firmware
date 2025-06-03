from LZ77 import LZ77Compressor
import filecmp
import pdb

class LZ77Encoder:
    def __init__(self, window_size=20, lookahead_buffer_size=15):
        self.window_size = window_size
        self.lookahead_buffer_size = lookahead_buffer_size

    def encode(self, data):
        encoded_data = []
        i = 0  # Start at the beginning
        
        while i < len(data):
            match_distance = 0
            match_length = 0
            next_symbol = data[i] if i < len(data) else ""

            # Define the search window
            search_start = max(0, i - self.window_size)
            search_window = data[search_start:i]
            lookahead = min(self.lookahead_buffer_size, len(data) - i)
            
            # Find longest match in the search window
            for j in range(1, lookahead + 1):
                match_candidate = data[i:i + j]
                if match_candidate in search_window:
                    match_distance = i - search_window.rfind(match_candidate) - search_start
                    match_length = j
                else:
                    break  # Stop if match is broken
            
            # Next symbol after match
            next_symbol = data[i + match_length] if (i + match_length) < len(data) else ""

            # Append (distance, length, next symbol)
            encoded_data.append((match_distance, match_length, next_symbol))
            
            # Move forward
            i += match_length + 1 if match_length > 0 else 1
        
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
print("Decompressed matches data:", (data == decompressed))

print("\nArbitrary Binary")# Arbitratry binary
test_str = "This is a binary" 
print("Compressed binary of: " + str(test_str))
binary = ''.join(format(ord(i), '08b') for i in test_str) # Converting String to binary

comp_binary = encoder.encode(binary)
decomp_binary = decoder.decode(comp_binary)
print("Decompressed matches binary:", (binary == decomp_binary))

print("\npackage compression")
compressor = LZ77Compressor()



input_file = r'C:\Users\Steph\Documents\GitRepos\ENTS-node-firmware\compression\Algos\LZ77\input.txt'
compressed_data = r'C:\Users\Steph\Documents\GitRepos\ENTS-node-firmware\compression\Algos\LZ77\compressed.txt'
decompressed_data = r'C:\Users\Steph\Documents\GitRepos\ENTS-node-firmware\compression\Algos\LZ77\decompressed.txt'
# or assign compressed data into a variable 
compressor.compress(input_file, compressed_data, True)
compressor.decompress(compressed_data, decompressed_data)

print("Decompressed matches og file:", filecmp.cmp(decompressed_data, input_file))
