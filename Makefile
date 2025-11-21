include flags.mk

LOG_LEVEL := -1
CFLAGS := $(CFLAGS) -DLOG_LEVEL=$(LOG_LEVEL)

.PHONY: all
all: mystty

build/linenoise.o: CFLAGS = -Wall -W -Os
build/linenoise.o: vendor/linenoise/linenoise.c
	$(CC) $(CFLAGS) -c $< -o $@

LLAMA_BUILD := build/llama.cpp
LLAMA_STATIC_LIBS := $(LLAMA_BUILD)/src/libllama.a \
	$(LLAMA_BUILD)/ggml/src/libggml.a \
	$(LLAMA_BUILD)/ggml/src/libggml-cpu.a \
	$(LLAMA_BUILD)/ggml/src/libggml-base.a \
	$(LLAMA_BUILD)/ggml/src/ggml-metal/libggml-metal.a \
	$(LLAMA_BUILD)/ggml/src/ggml-blas/libggml-blas.a

build/llama.cpp/src/libllama.a:
	mkdir -p $(LLAMA_BUILD)
	cmake -S vendor/llama.cpp -B $(LLAMA_BUILD) \
		-DLLAMA_BUILD_TESTS=OFF \
		-DLLAMA_BUILD_EXAMPLES=OFF \
		-DLLAMA_BUILD_TOOLS=OFF \
		-DLLAMA_BUILD_SERVER=OFF \
		-DLLAMA_TOOLS_INSTALL=OFF \
		-DBUILD_SHARED_LIBS=OFF
	cmake --build $(LLAMA_BUILD) -j --config Release

mystty: CFLAGS := $(CFLAGS) -Ivendor/llama.cpp/include \
	-Ivendor/llama.cpp/ggml/include -Ivendor/linenoise
mystty: LDFLAGS := $(LDFLAGS) -lpthread -lstdc++ -framework Accelerate \
	-framework Foundation -framework Metal -framework MetalKit
mystty: src/ai.o src/screen.o src/master.o src/parser.o build/linenoise.o $(LLAMA_STATIC_LIBS)

tests/parser.test: CFLAGS := $(CFLAGS) -Ivendor/llama.cpp/include \
	-Ivendor/llama.cpp/ggml/include
tests/parser.test: LDFLAGS := $(LDFLAGS) -lpthread -lstdc++ -framework Accelerate \
	-framework Foundation -framework Metal -framework MetalKit
tests/parser.test: src/ai.o src/parser.o $(LLAMA_STATIC_LIBS)

tests/master.test: CFLAGS := $(CFLAGS) -Ivendor/llama.cpp/include \
	-Ivendor/llama.cpp/ggml/include
tests/master.test: LDFLAGS := $(LDFLAGS) -lpthread -lstdc++ -framework Accelerate \
	-framework Foundation -framework Metal -framework MetalKit
tests/master.test: src/ai.o src/master.o $(LLAMA_STATIC_LIBS)

.PHONY: test
test: tests/buffers.test tests/parser.test tests/map.test tests/master.test
	tests/buffers.test
	tests/map.test
	tests/parser.test
	tests/master.test

.PHONY: clean
clean:
	rm -f ./mystty
	rm -f tests/*.test
	rm -rf **/*.dSYM **/*.plist *.plist **/*.o

.PHONY: deep-clean
deep-clean: clean
	rm -rf build/*

.PHONY: start
start: all
	./mystty

.PHONY: start-profile
start-profile: all
	ASAN_OPTIONS=detect_leaks=1 LSAN_OPTIONS=suppressions=asan.supp ./mystty
