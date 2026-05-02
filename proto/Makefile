PROTOC = protoc
NANOPB = nanopb_generator

# Additional options:
# https://github.com/nanopb/nanopb/blob/master/generator/proto/nanopb.proto#L54
NANOPB_OPTIONS = -s enum_to_string:true

C_SRC_DIR = c/src
C_INC_DIR = c/include

PYTHON_DIR = ../python/src/ents/proto

BUILD_DIR = build

PROTO_FILES = $(wildcard *.proto)

all: c python

c:
	mkdir -p build
	$(NANOPB) $(NANOPB_OPTIONS) --output-dir=$(BUILD_DIR) $(PROTO_FILES)
	cp $(BUILD_DIR)/*.pb.c $(C_SRC_DIR)
	cp $(BUILD_DIR)/*.pb.h $(C_INC_DIR)

python:
	$(PROTOC) --python_out=$(PYTHON_DIR) $(PROTO_FILES)

clean:
	rm -r build

.PHONY: all clean c python
