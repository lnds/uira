# kaikai + raylib demos. The kai compiler emits C and links via
# cc; we feed the raylib pkg-config flags and the font shim through
# CFLAGS.
#
# Defaults to the `kai` on $PATH (e.g. `brew install
# lnds/kaikai/kaikai`). Override to point at a sibling kaikai
# checkout: `make KAI_BIN=../kaikai/bin/kai`.
#
# `--backend=c` is required: Color/Vector2 cross the FFI by value
# (struct-by-value), which the native backend does not support yet.

KAI_BIN ?= kai
KAI_BACKEND := --backend=c

RAYLIB_CFLAGS := $(shell pkg-config --cflags raylib)
RAYLIB_LIBS   := $(shell pkg-config --libs raylib)

# Only fonts need a C shim — raylib's Font isn't a by-value struct.
# Everything else binds straight to raylib. The emitted out.c must
# NOT see raylib.h, so the shim is a separate translation unit.
FONTS_C := ffi/raylib_fonts.c

KAI_CFLAGS := -std=c99 -O2 -Wno-unused-function -Wno-unused-variable \
              $(RAYLIB_CFLAGS) $(FONTS_C) $(RAYLIB_LIBS)

.PHONY: all conway boids mandelbrot snake kaikai solar portfolio orderbook observatory \
        run-conway run-boids run-mandelbrot run-snake run-kaikai run-solar run-portfolio run-orderbook run-observatory clean

all: conway boids mandelbrot snake kaikai solar portfolio orderbook observatory

conway: build/conway

boids: build/boids

mandelbrot: build/mandelbrot

snake: build/snake

kaikai: build/kaikai

solar: build/solar

portfolio: build/portfolio

build/conway: conway.kai ffi/raylib.kai $(FONTS_C) | build
	CFLAGS="$(KAI_CFLAGS)" $(KAI_BIN) build $(KAI_BACKEND) conway.kai -o $@

build/boids: boids.kai ffi/raylib.kai $(FONTS_C) | build
	CFLAGS="$(KAI_CFLAGS)" $(KAI_BIN) build $(KAI_BACKEND) boids.kai -o $@

build/mandelbrot: mandelbrot.kai ffi/raylib.kai $(FONTS_C) | build
	CFLAGS="$(KAI_CFLAGS)" $(KAI_BIN) build $(KAI_BACKEND) mandelbrot.kai -o $@

build/snake: snake.kai ffi/raylib.kai $(FONTS_C) | build
	CFLAGS="$(KAI_CFLAGS)" $(KAI_BIN) build $(KAI_BACKEND) snake.kai -o $@

build/kaikai: kaikai.kai ffi/raylib.kai $(FONTS_C) | build
	CFLAGS="$(KAI_CFLAGS)" $(KAI_BIN) build $(KAI_BACKEND) kaikai.kai -o $@

build/solar: solar.kai ffi/raylib.kai $(FONTS_C) | build
	CFLAGS="$(KAI_CFLAGS)" $(KAI_BIN) build $(KAI_BACKEND) solar.kai -o $@

build/portfolio: portfolio.kai ffi/raylib.kai $(FONTS_C) | build
	CFLAGS="$(KAI_CFLAGS)" $(KAI_BIN) build $(KAI_BACKEND) portfolio.kai -o $@

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

orderbook: build/orderbook

build/orderbook: orderbook.kai ffi/raylib.kai $(FONTS_C) | build
	CFLAGS="$(KAI_CFLAGS)" $(KAI_BIN) build $(KAI_BACKEND) orderbook.kai -o $@

run-orderbook: build/orderbook
	./build/orderbook

observatory: build/observatory

build/observatory: observatory.kai ffi/raylib.kai $(FONTS_C) | build
	CFLAGS="$(KAI_CFLAGS)" $(KAI_BIN) build $(KAI_BACKEND) observatory.kai -o $@

run-observatory: build/observatory
	./build/observatory
