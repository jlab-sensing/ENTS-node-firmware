#!/bin/zsh

# format c code
echo "Formattting C code..."
clang-format --style=Google -i --verbose \
  stm32/Src/examples/*.c \
  stm32/lib/**/*.h \
  stm32/lib/**/*.c \
  stm32/test/**/*.h \
  stm32/test/**/*.c \
  esp32/**/*.cpp \
  esp32/**/*.hpp \
  proto/c/include/transcoder.h \
  proto/c/src/transcoder.c
echo "Done."

# format python code
echo "Formattting Python code..."
black proto/python
echo "Done."
