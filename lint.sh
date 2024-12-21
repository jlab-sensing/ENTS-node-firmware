#!/bin/sh

# lint c code
python -m cpplint --recursive --quiet \
  stm32/Src/examples \
  stm32/lib \
  stm32/test \
  esp32/include \
  esp32/src \
  esp32/test \
  esp32/lib \
  proto/c/include/transcoder.h \
  proto/c/src/transcoder.c

# lint python code
ruff check proto/python
