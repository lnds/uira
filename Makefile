# kaikai + raylib demos. The C shim and the raylib link flags are
# declared in kai.toml's [native] table, so `kai build` needs nothing
# from this Makefile beyond the list of demos.
#
# Defaults to the `kai` on $PATH (kaikai 0.165+). Override to point
# at a sibling kaikai checkout: `make KAI_BIN=../kaikai/bin/kai`.

KAI_BIN ?= kai

DEMOS := conway boids mandelbrot snake kaikai solar portfolio orderbook observatory audiopad sprites
FFI   := $(wildcard ffi/*.kai) $(wildcard ffi/*.c) kai.toml

.PHONY: all clean $(DEMOS) $(addprefix run-,$(DEMOS))

all: $(DEMOS)

$(DEMOS): %: build/%

build/%: %.kai $(FFI) | build
	$(KAI_BIN) build $< -o $@

$(addprefix run-,$(DEMOS)): run-%: build/%
	./build/$*

build:
	mkdir -p build

clean:
	rm -rf build
