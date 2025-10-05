include flags.mk

.PHONY: all
all: bin/zork

build/llama.cpp/src/libllama.a:
	mkdir -p build/llama.cpp
	cmake -S vendor/llama.cpp -B build/llama.cpp \
		-DLLAMA_BUILD_TESTS=OFF \
		-DLLAMA_BUILD_EXAMPLES=OFF \
		-DLLAMA_BUILD_TOOLS=OFF \
		-DLLAMA_BUILD_SERVER=OFF \
		-DLLAMA_TOOLS_INSTALL=OFF \
		-DBUILD_SHARED_LIBS=OFF
	cmake --build build/llama.cpp -j --config Release

bin/zork: CFLAGS := $(CFLAGS) -Ivendor/llama.cpp/include -Ivendor/llama.cpp/ggml/include
bin/zork: LDFLAGS := $(LDFLAGS) -lpthread -lstdc++
bin/zork: build/llama.cpp/ggml/src/libggml.a build/llama.cpp/src/libllama.a
	$(CC) $(CFLAGS) $(LDFLAGS) bin/zork.c \
		build/llama.cpp/src/libllama.a \
		build/llama.cpp/ggml/src/libggml.a \
		build/llama.cpp/ggml/src/libggml-cpu.a \
		build/llama.cpp/ggml/src/libggml-base.a \
		build/llama.cpp/ggml/src/ggml-metal/libggml-metal.a \
		build/llama.cpp/ggml/src/ggml-blas/libggml-blas.a \
		-framework Accelerate -framework Foundation -framework Metal -framework MetalKit \
		-o bin/zork

.PHONY: clean
clean:
	rm -f bin/zork

.PHONY: deep-clean
deep-clean: clean
	rm -rf build/*