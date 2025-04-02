from Algos.LZ77.LZ77 import LZ77Compressor

from pathlib import Path
import filecmp


def append_size_ratio(algorithim, size_orginal, size_compressed):
    """Appends the sizes and calculates the ratio between them"""
    ratio = size_orginal/size_compressed
    algorithim[0].append(size_orginal,)
    algorithim[1].append(size_compressed)
    algorithim[2].append(ratio)

compression_ratio_LZ77 = [[],[],[]] # Struct containing [[original size of file in bytes], [size of compressed in bytes], ratio between them]
compression_ratio_LZ78 = [[],[],[]]

LZ77_compressor = LZ77Compressor()

compressed_data = Path("compressed.txt") # Define the file that the compressed data will be written to, will be wiped after every use
sample_messages = Path('messages')

# Iterate through all the files in the directory
for filepath in sample_messages.iterdir():
    if filepath.is_file() and filepath.name != "__init__.py":
        print(f"File: {filepath.name}")
        # LZ77
        LZ77_compressor.compress(str(filepath), compressed_data)
        append_size_ratio(compression_ratio_LZ77, filepath.stat().st_size, compressed_data.stat().st_size)
        compressed_data.write_text('')  # Clears the contents of the file

print(compression_ratio_LZ77)