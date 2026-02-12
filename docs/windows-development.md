# Build environment setup for Windows

## Nanopb and Protobuf
1. Download the latest nanopb release from https://jpa.kapsi.fi/nanopb/download/
    - At the time of writing, this is `nanopb-0.4.9.1-windows-x86.zip`
2. Unzip the archive somewhere and add the `generator-bin/` subdirectory to your PATH
    - Start Menu > search "environment" > Edit the system environment variables > Environment Variables... > Under the "System variables" selection, choose "Path" and press the "Edit" button > Press the "New" button and enter in the full path to `generator-bin/`
3. Download the latest protobuf release from: https://github.com/protocolbuffers/protobuf/releases
    - At the time of writing, this is `protoc-33.2-win64.zip`
4. Unzip the archive somewhere and copy the `bin/protoc.exe` into the nanopb `generator-bin/` directory, overwriting the existing one.
5. Note: You may need to modify the makefiles to use the executable names (the `*.exe` files from the extracted archives).

## clang-format
1. Download MSYS2 https://www.msys2.org/
2. In a UCRT shell, run: `pacman -S mingw-w64-ucrt-x86_64-gcc make gettext base-devel mingw-w64-ucrt-x86_64-clang`
3. Add `C:\msys64\usr\bin` and `C:\msys64\ucrt64\bin` to your PATH
