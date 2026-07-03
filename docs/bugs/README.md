# kaikai compiler bugs found while building uira

## native backend corrupts a persistent Array of a String-payload sum type

**Version:** kaikai 0.98.0 (hanga-roa, stage 2), arm64 Darwin.
**Repro:** [`repro_native_rc_sumtype.kai`](repro_native_rc_sumtype.kai) —
46 lines, pure kaikai, no FFI.

An `Array[Body]` where `Body` is a sum type whose first payload is a
`String` is built once and then read/matched across loop iterations,
never mutated. On the **native (libLLVM) backend** the array is
corrupted after the first pass:

```
kai run --backend=native repro_native_rc_sumtype.kai
  frame 0:  body 0: 5   body 1: 5   body 2: 2.66667      # correct
  frame 1:  body 0: 5
  kai: type mismatch in /                                # panic
```

The same program on the **C backend** is correct — all three frames
print identical values:

```
kai run --backend=c repro_native_rc_sumtype.kai
  frame 0/1/2:  body 0: 5   body 1: 5   body 2: 2.66667  # all correct
```

Same source, opposite result per backend ⇒ a native-backend codegen
bug, most likely Perceus reference counting freeing the heap `String`
payload while the array still holds it. The `type mismatch in /` is a
downstream symptom: the `Real` pulled out of the now-garbage `Body`
no longer carries its type tag.

**Where it bit us:** `solar.kai` reads its persistent `Array[Body]`
every frame and panicked on the second frame. Rebuilding the array
each frame avoids it. Left unfixed in the demo pending an upstream
fix, so the demo keeps exercising the real path.
