include flags.mk

LOG_LEVEL := -1
CFLAGS := $(CFLAGS) -DLOG_LEVEL=$(LOG_LEVEL)
BUILD := build

.PHONY: all
all: ttyny

build/linenoise.o: CFLAGS = -Wall -W -Os
build/linenoise.o: vendor/linenoise/linenoise.c
	mkdir -p $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

build/yyjson.o: CFLAGS = -Wall -W -Os
build/yyjson.o: vendor/yyjson/src/yyjson.c
	mkdir -p $(BUILD)
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
		-DLLAMA_CURL=OFF \
		-DBUILD_SHARED_LIBS=OFF
	cmake --build $(LLAMA_BUILD) -j --config Release

src/world/world.o: CFLAGS := $(CFLAGS) -Ivendor/yyjson/src
src/world/world.o: build/yyjson.o

ttyny: CFLAGS := $(CFLAGS) -Ivendor/llama.cpp/include \
	-Ivendor/llama.cpp/ggml/include -Ivendor/linenoise -Ivendor/yyjson/src
ttyny: LDFLAGS := $(LDFLAGS) -lpthread -lstdc++ -framework Accelerate \
	-framework Foundation -framework Metal -framework MetalKit
ttyny: src/ai.o src/screen.o src/master.o src/parser.o src/world/world.o build/linenoise.o build/yyjson.o $(LLAMA_STATIC_LIBS)

tests/parser.test: CFLAGS := $(CFLAGS) -Ivendor/llama.cpp/include \
	-Ivendor/llama.cpp/ggml/include
tests/parser.test: LDFLAGS := $(LDFLAGS) -lpthread -lstdc++ -framework Accelerate \
	-framework Foundation -framework Metal -framework MetalKit
tests/parser.test: src/ai.o src/parser.o $(LLAMA_STATIC_LIBS)

tests/master.time: CFLAGS := $(CFLAGS) -Ivendor/llama.cpp/include \
	-Ivendor/llama.cpp/ggml/include
tests/master.time: LDFLAGS := $(LDFLAGS) -lpthread -lstdc++ -framework Accelerate \
	-framework Foundation -framework Metal -framework MetalKit
tests/master.time: src/ai.o src/master.o $(LLAMA_STATIC_LIBS)

tests/master.snap: CFLAGS := $(CFLAGS) -Ivendor/llama.cpp/include \
	-Ivendor/llama.cpp/ggml/include
tests/master.snap: LDFLAGS := $(LDFLAGS) -lpthread -lstdc++ -framework Accelerate \
	-framework Foundation -framework Metal -framework MetalKit
tests/master.snap: src/ai.o src/master.o $(LLAMA_STATIC_LIBS)

tests/master.test: CFLAGS := $(CFLAGS) -Ivendor/llama.cpp/include \
	-Ivendor/llama.cpp/ggml/include -Ivendor/yyjson/src
tests/master.test: LDFLAGS := $(LDFLAGS) -lpthread -lstdc++ -framework Accelerate \
	-framework Foundation -framework Metal -framework MetalKit
tests/master.test: src/ai.o src/master.o $(LLAMA_STATIC_LIBS)

tests/json.test: src/world/world.o build/yyjson.o
tests/world.test: src/world/world.o build/yyjson.o

.PHONY: snap
snap: tests/master.snap
	tests/master.snap

.PHONY: time
time: tests/master.time
	tests/master.time

.PHONY: test-slow
test-slow:  tests/parser.test
	 tests/parser.test

.PHONY: test
test: tests/buffers.test tests/map.test tests/world.test tests/json.test
	tests/buffers.test
	tests/map.test
	tests/world.test
	tests/json.test

.PHONY: clean
clean:
	rm -f ./ttyny
	rm -f tests/*.test
	rm -rf **/*.dSYM **/*.plist *.plist
	find . -type f -name '*.o' -not -path './build/*' -delete

.PHONY: deep-clean
deep-clean: clean
	rm -rf build/*

.PHONY: start
start: all
	./ttyny assets/psyche.json

.PHONY: start-profile
start-profile: all
	ASAN_OPTIONS=detect_leaks=1 LSAN_OPTIONS=suppressions=asan.supp ./ttyny assets/psyche.json
