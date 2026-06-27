# Migración al FFI v2 de kaikai — completada

uira estaba escrito contra el FFI v1 (solo `Int`/`Real`/`Bool`/`String`/
`Unit` cruzaban el boundary), con un shim C que empaquetaba `Color` en un
`Int` `0xRRGGBBAA`, descomponía `Vector2` en pares de `Real`, y mantenía
una tabla de slots de `Font`. Con kaikai 0.91.2 esto migró al FFI v2.

## Qué habilita el FFI v2 (kaikai 0.91)

- **Tipos de ancho fijo** `U8 U16 U32 U64 I8 I16 I32 I64 F32`: anotan el
  ancho C exacto en el boundary; del lado kaikai siguen siendo `Int`
  (o `Real` para `F32`) y unifican con literales y aritmética.
- **Struct-by-value** `extern "C" type T = { campo: <ancho-fijo>, ... }`:
  records por valor en ambos sentidos.
- **Opaque handles** `extern "C" opaque T`: `void*` refcounted.

## Arquitectura final: sin shim, salvo fonts

El hallazgo clave es que el shim C es casi totalmente innecesario:

- `Color`/`Vector2`/`Rect` se declaran como `extern "C" type` con campos
  `U8`/`F32`. Su layout es **ABI-idéntico** al de las structs de raylib,
  así que los bindings linkean **directo** a `ClearBackground`,
  `DrawCircleV`, `DrawRectangleRec`, `DrawTriangle`, etc. — sin shim.

- **Invariante que lo hace funcionar:** el `out.c` que kaikai emite NO
  debe ver `raylib.h`. kaikai genera sus propios typedefs `Color`/
  `Vector2` (en su `kai_ffi.h`); si `raylib.h` también entra al mismo
  TU, sus typedefs homónimos chocan (`typedef redefinition`). Por eso el
  Makefile NO pasa `-include raylib.h`: solo enlaza contra `libraylib`.

- **Fonts son la excepción.** `Font` de raylib tiene punteros y una
  `Texture2D` anidada, así que no puede ser un `extern "C" type`. Va por
  `ffi/raylib_fonts.c`: una tabla de slots con entry points escalares.
  Ese shim SÍ incluye `raylib.h` (necesita `LoadFontEx`/`DrawTextEx`),
  pero es un TU separado, y declara `draw_text_ex` con el `Color` de
  raylib — ABI-compatible con el que pasa kaikai. Slot `-1` = fuente
  default (cae a `DrawText`).

## API que ven los demos

Las firmas de dibujo toman `color: Color`. Los colores se siguen
escribiendo como hex vía el helper puro `rgba(p: Int) : Color`:

```
const col_bg : Color = rgba(0x14141eFF)
```

Coordenadas/tamaños siguen siendo `Int`/`Real` en los call sites; el
binding usa `I32`/`F32` solo en el boundary (ABI correcto sin tocar la
aritmética kaikai). Patrones de migración aplicados:

- `const col_x : Int = 0x..` → `const col_x : Color = rgba(0x..)`.
- Hex inline a un `draw_*` → `raylib.rgba(0x..)`.
- Funciones que devolvían un color como `Int` (paletas, `band_color`,
  `side_color`, `class_color`, `body_color`) → retorno `Color`, cada
  rama envuelta en `rgba(...)`. Donde el color vive en una estructura de
  datos (solar), se deja `Int` y se envuelve con `rgba()` en la capa
  VIEW.
- `draw_triangle` (6 escalares) → `draw_triangle_v(Vector2, Vector2,
  Vector2, Color)` — solo boids.

## Notas del toolchain

- **`--backend=c` es obligatorio** (en el Makefile): struct-by-value no
  compila aún en el backend native, que lo reporta limpio y sugiere el
  flag.
- Los bugs #922 (const-en-closure), #923 (segfault native) y #924
  (struct-FFI no expuesta al shim) están **cerrados** en 0.91.2. El
  workaround de #922 resultó innecesario: los `const` capturados en
  closures compilan sin rebinds.
- 0.91.2 endurece la verificación de efectos: varios demos tenían
  funciones que llamaban a externs (`sqrt`) o mutaban sin declarar
  `/ Ffi` / `+ Mutable` en su row — breakages preexistentes que se
  corrigieron al alinear los rows. orderbook además necesitó
  `fiber_yield`/`fiber_spawn` → `spawn.yield`/`spawn.spawn` (renombres de
  stdlib).

## Verificación

`make` compila los 9 demos desde cero sin warnings. Confirmación visual
vía screenshot de snake (formas + texto default) y observatory (fuente
TTF custom por el shim de fonts + colores espectrales): ambos renderizan
correctos.
