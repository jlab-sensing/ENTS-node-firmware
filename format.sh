#!/bin/zsh

# format c code
echo "Formattting stm32 C code..."
clang-format --fallback-style=Google -i --verbose \
  stm32/Src/examples/*.c \
  stm32/lib/**/*.h \
  stm32/lib/**/*.c \
  stm32/test/**/*.h \
  stm32/test/**/*.c
echo "Done."

echo "Formattting esp32 C++ code..."
clang-format --fallback-style=Google -i --verbose \
  esp32/**/*.cpp \
  esp32/**/*.hpp
echo "Done."

echo "Formatting protobuf code..."
clang-format --style=Google -i --verbose \
  proto/c/include/transcoder.h \
  proto/c/src/transcoder.c \
  proto/**/*.proto
echo "Done."

# format python code
echo "Formattting Python code..."
black python
echo "Done."
