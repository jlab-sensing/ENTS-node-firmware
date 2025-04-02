# ENTS Compression Algorithims 

Compression algorithims are being increasingly explored in [IoT settings](https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=10330191) to streamline data transmission to save power and bandwidth. 

Here we are exploring how the following compression algorithims impact the performance of the ENTS board when combined with ProtoBuf serilization.

1. LZ77
2. LZ78
3. LZQ
4. LZMA
5. Run length encoding
6. Asymmetric numerical systems
7. Arithmetic Coding

# Algos

Algos contains rough implementations and test harnesses for each of the algorithims. 

## LZ77

LZ77 is implemented using this {LZ77 library}[https://github.com/manassra/LZ77-Compressor]

# Compression Comparison

Compression comparison containts the harness for compressing and decompressing all the test messages stored in _messages_, and visualizing their compression ratio vs bytes.

# Messages

Contains the test messages as txt files