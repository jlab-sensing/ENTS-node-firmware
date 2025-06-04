#!/bin/sh

cd stm32
pio check -e stm32
cd ..

cd esp32
pio check
cd ..

# lint python code
ruff check python
