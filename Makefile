include flags.mk

.PHONY: all
all: zork

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

zork: CFLAGS := $(CFLAGS) -Ivendor/llama.cpp/include \
	-Ivendor/llama.cpp/ggml/include
zork: LDFLAGS := $(LDFLAGS) -lpthread -lstdc++ -framework Accelerate \
	-framework Foundation -framework Metal -framework MetalKit
zork: src/ai.o $(LLAMA_STATIC_LIBS)

.PHONY: test
test: tests/buffers.test
	tests/buffers.test

.PHONY: clean
clean:
	rm -f ./zork
	rm -f tests/*.test
	rm -rf **/*.dSYM **/*.plist *.plist **/*.o

.PHONY: deep-clean
deep-clean: clean
	rm -rf build/*

.PHONY: start
start: all
	./zork

.PHONY: start-profile
start-profile: all
	ASAN_OPTIONS=detect_leaks=1 LSAN_OPTIONS=suppressions=asan.supp ./zork
