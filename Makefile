# kaikai + raylib demos. kai's native backend classifies the
# by-value FFI structs (Color/Vector2/Rect) into the platform
# calling convention; we feed the raylib pkg-config flags and the
# C shims through CFLAGS. A cc-only checkout falls back to the
# C-text backend on its own.
#
# Defaults to the `kai` on $PATH (kaikai 0.98+). Override to point
# at a sibling kaikai checkout: `make KAI_BIN=../kaikai/bin/kai`.

KAI_BIN ?= kai
KAI_BACKEND :=

RAYLIB_CFLAGS := $(shell pkg-config --cflags raylib)
RAYLIB_LIBS   := $(shell pkg-config --libs raylib)

# Font, Sound and Music all cross the FFI by value now, so they bind
# straight to raylib. The one C shim left is gen_tone (synthesising a
# tone needs a real sample buffer). It's a separate translation unit
# because the emitted code must NOT see raylib.h.
AUDIO_C := ffi/raylib_audio.c
SHIMS   := $(AUDIO_C)

KAI_CFLAGS := -std=c99 -O2 -Wno-unused-function -Wno-unused-variable \
              $(RAYLIB_CFLAGS) $(SHIMS) $(RAYLIB_LIBS)

.PHONY: all conway boids mandelbrot snake kaikai solar portfolio orderbook observatory audiopad sprites \
        run-conway run-boids run-mandelbrot run-snake run-kaikai run-solar run-portfolio run-orderbook run-observatory run-audiopad run-sprites clean

all: conway boids mandelbrot snake kaikai solar portfolio orderbook observatory audiopad sprites

conway: build/conway

boids: build/boids

mandelbrot: build/mandelbrot

snake: build/snake

kaikai: build/kaikai

solar: build/solar

portfolio: build/portfolio

build/conway: conway.kai ffi/raylib.kai $(SHIMS) | build
	CFLAGS="$(KAI_CFLAGS)" $(KAI_BIN) build $(KAI_BACKEND) conway.kai -o $@

build/boids: boids.kai ffi/raylib.kai $(SHIMS) | build
	CFLAGS="$(KAI_CFLAGS)" $(KAI_BIN) build $(KAI_BACKEND) boids.kai -o $@

build/mandelbrot: mandelbrot.kai ffi/raylib.kai $(SHIMS) | build
	CFLAGS="$(KAI_CFLAGS)" $(KAI_BIN) build $(KAI_BACKEND) mandelbrot.kai -o $@

build/snake: snake.kai ffi/raylib.kai $(SHIMS) | build
	CFLAGS="$(KAI_CFLAGS)" $(KAI_BIN) build $(KAI_BACKEND) snake.kai -o $@

build/kaikai: kaikai.kai ffi/raylib.kai $(SHIMS) | build
	CFLAGS="$(KAI_CFLAGS)" $(KAI_BIN) build $(KAI_BACKEND) kaikai.kai -o $@

build/solar: solar.kai ffi/raylib.kai $(SHIMS) | build
	CFLAGS="$(KAI_CFLAGS)" $(KAI_BIN) build $(KAI_BACKEND) solar.kai -o $@

build/portfolio: portfolio.kai ffi/raylib.kai $(SHIMS) | build
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

build/orderbook: orderbook.kai ffi/raylib.kai $(SHIMS) | build
	CFLAGS="$(KAI_CFLAGS)" $(KAI_BIN) build $(KAI_BACKEND) orderbook.kai -o $@

run-orderbook: build/orderbook
	./build/orderbook

observatory: build/observatory

build/observatory: observatory.kai ffi/raylib.kai $(SHIMS) | build
	CFLAGS="$(KAI_CFLAGS)" $(KAI_BIN) build $(KAI_BACKEND) observatory.kai -o $@

run-observatory: build/observatory
	./build/observatory

audiopad: build/audiopad

build/audiopad: audiopad.kai ffi/raylib.kai $(SHIMS) | build
	CFLAGS="$(KAI_CFLAGS)" $(KAI_BIN) build $(KAI_BACKEND) audiopad.kai -o $@

run-audiopad: build/audiopad
	./build/audiopad

sprites: build/sprites

build/sprites: sprites.kai ffi/raylib.kai ffi/keys.kai $(SHIMS) | build
	CFLAGS="$(KAI_CFLAGS)" $(KAI_BIN) build $(KAI_BACKEND) sprites.kai -o $@

run-sprites: build/sprites
	./build/sprites
