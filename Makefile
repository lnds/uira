# kaikai + raylib demos. The kai compiler at the sibling kaikai/
# repo emits C and links via cc; we pipe in the FFI shim and the
# raylib pkg-config flags through CFLAGS.

KAI_BIN ?= ../kaikai/bin/kai

RAYLIB_CFLAGS := $(shell pkg-config --cflags raylib)
RAYLIB_LIBS   := $(shell pkg-config --libs raylib)

SHIM_C := ffi/raylib_shim.c
SHIM_H := ffi/raylib_shim.h

# The stdlib prelude is auto-loaded — the demos rely on `loop`
# (repeat / until / forever / while), and `mandelbrot` uses
# `Complex` from `math/complex`. kaikai 0.41 closed the typer
# regression that previously forced `KAI_NO_STDLIB=1` here.
KAI_CFLAGS := -std=c99 -O2 -Wno-unused-function -Wno-unused-variable \
              -include $(SHIM_H) $(RAYLIB_CFLAGS) $(SHIM_C) $(RAYLIB_LIBS)

.PHONY: all conway boids mandelbrot snake kaikai solar portfolio \
        run-conway run-boids run-mandelbrot run-snake run-kaikai run-solar run-portfolio clean

all: conway boids mandelbrot snake kaikai solar portfolio

conway: build/conway

boids: build/boids

mandelbrot: build/mandelbrot

snake: build/snake

kaikai: build/kaikai

solar: build/solar

portfolio: build/portfolio

build/conway: conway.kai ffi/raylib.kai $(SHIM_C) $(SHIM_H) | build
	CFLAGS="$(KAI_CFLAGS)" $(KAI_BIN) build conway.kai -o $@

build/boids: boids.kai ffi/raylib.kai $(SHIM_C) $(SHIM_H) | build
	CFLAGS="$(KAI_CFLAGS)" $(KAI_BIN) build boids.kai -o $@

build/mandelbrot: mandelbrot.kai ffi/raylib.kai $(SHIM_C) $(SHIM_H) | build
	CFLAGS="$(KAI_CFLAGS)" $(KAI_BIN) build mandelbrot.kai -o $@

build/snake: snake.kai ffi/raylib.kai $(SHIM_C) $(SHIM_H) | build
	CFLAGS="$(KAI_CFLAGS)" $(KAI_BIN) build snake.kai -o $@

build/kaikai: kaikai.kai ffi/raylib.kai $(SHIM_C) $(SHIM_H) | build
	CFLAGS="$(KAI_CFLAGS)" $(KAI_BIN) build kaikai.kai -o $@

build/solar: solar.kai ffi/raylib.kai $(SHIM_C) $(SHIM_H) | build
	CFLAGS="$(KAI_CFLAGS)" $(KAI_BIN) build solar.kai -o $@

build/portfolio: portfolio.kai ffi/raylib.kai $(SHIM_C) $(SHIM_H) | build
	CFLAGS="$(KAI_CFLAGS)" $(KAI_BIN) build portfolio.kai -o $@

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

run-kaikai: build/kaikai
	./build/kaikai

run-solar: build/solar
	./build/solar

run-portfolio: build/portfolio
	./build/portfolio

clean:
	rm -rf build
