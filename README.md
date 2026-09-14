# minihdlc

Portable C++ library for HDLC asynchronous framing (byte-stuffed frames with CRC-CCITT).

## Frame structure

![Frame structure](https://www.notblackmagic.com/bitsnpieces/ax.25/img/HDLC_Frame.png)
(image from [notblackmagic.com](https://www.notblackmagic.com/bitsnpieces/ax.25/#hdlc))

## Use as a CMake submodule (firmware)

```bash
git submodule add <repo-url> third_party/minihdlc
```

In the firmware `CMakeLists.txt`, set options **before** `add_subdirectory`:

```cmake
set(MINIHDLC_TINY_MODE ON CACHE BOOL "")
set(MINIHDLC_USE_CALLBACK ON CACHE BOOL "")
set(MINIHDLC_MAX_FRAME_LENGTH 32 CACHE STRING "")
# optional: force embedded flags even if not cross-compiling
# set(MINIHDLC_EMBEDDED_FLAGS ON CACHE BOOL "")

add_subdirectory(third_party/minihdlc)
target_link_libraries(firmware PRIVATE minihdlc::minihdlc)
```

When minihdlc is included this way it does **not** fetch GoogleTest or build its tests.

### Options

| Cache variable | Default | Effect |
|---|---|---|
| `MINIHDLC_TINY_MODE` | `OFF` | Disables pointer/length checks (`MINIHDLC_TINY`) |
| `MINIHDLC_USE_CALLBACK` | `OFF` | Callback RX API instead of polling |
| `MINIHDLC_MAX_FRAME_LENGTH` | `64` | Max payload / RX buffer size |
| `MINIHDLC_EMBEDDED_FLAGS` | `OFF` | Add `-fno-exceptions -fno-rtti -fno-threadsafe-statics` to the library (also auto-on when `CMAKE_SYSTEM_NAME=Generic`) |

### Headers

```cpp
#include "minihdlc.h"      // MiniHDLCController
#include "crc_ccitt.h"     // only if you call CrcUpdate / CrcBlock directly
```

## STM32 / flash tips

Recommended profile: **TINY + CALLBACK**, small `MINIHDLC_MAX_FRAME_LENGTH`.

On the firmware target / linker:

- compile with `-ffunction-sections -fdata-sections`
- link with `--gc-sections` (unused methods like bulk `FeedFromIsr` drop out)
- use `-fno-exceptions -fno-rtti` (library applies these automatically for Generic/`arm-none-eabi` toolchains)

## Standalone build / tests

```bash
cmake --preset tests
cmake --build --preset tests
ctest --preset tests
```

Other presets: `tests-tiny`, `tests-callback`, `tests-tiny-callback`, and `arm-no-tests*` for cross size checks.
