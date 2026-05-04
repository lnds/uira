# kaikai + raylib demos. The kai compiler at the sibling kaikai/
# repo emits C and links via cc; we pipe in the FFI shim and the
# raylib pkg-config flags through CFLAGS.

KAI_BIN ?= ../kaikai/bin/kai

RAYLIB_CFLAGS := $(shell pkg-config --cflags raylib)
RAYLIB_LIBS   := $(shell pkg-config --libs raylib)

SHIM_C := ffi/raylib_shim.c
SHIM_H := ffi/raylib_shim.h

# kaikai's stdlib has type-checking regressions in 0.37.0 that
# spuriously fail unrelated builds, so we opt out and rely only on
# compiler builtins (Array, print, int_to_string, real_to_string,
# int_to_real). FFI primitives (extern_c axioms) work either way.
KAI_ENV := KAI_NO_STDLIB=1
KAI_CFLAGS := -std=c99 -O2 -Wno-unused-function -Wno-unused-variable \
              -include $(SHIM_H) $(RAYLIB_CFLAGS) $(SHIM_C) $(RAYLIB_LIBS)

.PHONY: all conway boids mandelbrot snake run-conway run-boids run-mandelbrot run-snake clean

all: conway boids mandelbrot snake

conway: build/conway

boids: build/boids

mandelbrot: build/mandelbrot

snake: build/snake

build/conway: conway.kai ffi/raylib.kai $(SHIM_C) $(SHIM_H) | build
	$(KAI_ENV) CFLAGS="$(KAI_CFLAGS)" $(KAI_BIN) build conway.kai -o $@

build/boids: boids.kai ffi/raylib.kai $(SHIM_C) $(SHIM_H) | build
	$(KAI_ENV) CFLAGS="$(KAI_CFLAGS)" $(KAI_BIN) build boids.kai -o $@

build/mandelbrot: mandelbrot.kai ffi/raylib.kai $(SHIM_C) $(SHIM_H) | build
	$(KAI_ENV) CFLAGS="$(KAI_CFLAGS)" $(KAI_BIN) build mandelbrot.kai -o $@

build/snake: snake.kai ffi/raylib.kai $(SHIM_C) $(SHIM_H) | build
	$(KAI_ENV) CFLAGS="$(KAI_CFLAGS)" $(KAI_BIN) build snake.kai -o $@

build:
	mkdir -p build

run-conway: build/conway
	./build/conway

run-boids: build/boids
	./build/boids

run-mandelbrot: build/mandelbrot
	./build/mandelbrot

run-snake: build/snake
	./build/snake

clean:
	rm -rf build
