from Algos.LZ77.LZ77 import LZ77Compressor
from Algos.LZ78.LZ78 import LZ78Compressor  # <-- Import LZ78
from Algos.LZW.LZW import LZWCompressor  # <-- Import LZW
from Algos.LZMA.LZMA import LZMACompressor  # <-- Import LZMA

from pathlib import Path
import time
import csv

def append_size_ratio(results, filename, algorithm, size_original, size_compressed, computation_time):
    ratio = size_original / size_compressed if size_compressed != 0 else float('inf')
    results.append({
        "file": filename,
        "algorithm": algorithm,
        "original_size": size_original,
        "compressed_size": size_compressed,
        "ratio": ratio,
        "computation_time": computation_time
    })

compression_results = []

LZ77_compressor = LZ77Compressor()
LZ78_compressor = LZ78Compressor()  # <-- Initialize LZ78
LZW_compressor = LZWCompressor()  # <-- Initialize LZW
LZMA_compressor = LZMACompressor()  # <-- Initialize LZMA

compressed_data = Path("compressed.txt")
sample_messages = Path('messages')

for filepath in sample_messages.iterdir():
    if filepath.is_file() and filepath.name != "__init__.py":
        print(f"Compressing: {filepath.name}")

        # ------------------- LZ77 -------------------
        start = time.perf_counter()
        LZ77_compressor.compress(str(filepath), compressed_data)
        end = time.perf_counter()

        append_size_ratio(
            compression_results,
            filepath.name,
            "LZ77",
            filepath.stat().st_size,
            compressed_data.stat().st_size,
            end - start
        )

        compressed_data.write_text('')  # Clear contents

        # ------------------- LZ78 -------------------
        start = time.perf_counter()
        LZ78_compressor.compress(str(filepath), compressed_data)
        end = time.perf_counter()

        append_size_ratio(
            compression_results,
            filepath.name,
            "LZ78",
            filepath.stat().st_size,
            compressed_data.stat().st_size,
            end - start
        )

        compressed_data.write_text('')  # Clear contents again

        # ------------------- LZW -------------------
        start = time.perf_counter()
        LZW_compressor.compress(str(filepath), compressed_data)
        end = time.perf_counter()

        append_size_ratio(
            compression_results,
            filepath.name,
            "LZW",
            filepath.stat().st_size,
            compressed_data.stat().st_size,
            end - start
        )

        compressed_data.write_text('')  # Clear contents again

        # ------------------- LZMA -------------------
        start = time.perf_counter()
        LZMA_compressor.compress(str(filepath), compressed_data)  # <-- Use LZMA compressor
        end = time.perf_counter()

        append_size_ratio(
            compression_results,
            filepath.name,
            "LZMA",
            filepath.stat().st_size,
            compressed_data.stat().st_size,
            end - start
        )

        compressed_data.write_text('')  # Clear contents again

# ------------------- Write to file -------------------
with open("compression_results.csv", "w", newline='') as csvfile:
    writer = csv.writer(csvfile)
    writer.writerow(["File", "Algorithm", "Original Size (B)", "Compressed Size (B)", "Compression Ratio", "Computation Time (s)"])

    for result in compression_results:
        writer.writerow([
            result['file'],
            result['algorithm'],
            result['original_size'],
            result['compressed_size'],
            f"{result['ratio']:.4f}",
            f"{result['computation_time']:.6f}"
        ])

print("Results saved to compression_results.csv")
