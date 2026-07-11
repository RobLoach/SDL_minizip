# SDL_minizip

A single-header C library for reading ZIP archives using [minizip-ng](https://github.com/zlib-ng/minizip-ng) and [SDL3](https://github.com/libsdl-org/SDL)'s [`SDL_Storage`](https://wiki.libsdl.org/SDL3/CategoryStorage) API.

## Features

- Read ZIP archives through [`SDL_Storage`](https://wiki.libsdl.org/SDL3/CategoryStorage)
- Uses native `SDL_IOStream` for all underlying I/O operations
- Single-header `stb`-style library
- Thread-safe

## Dependencies
- [SDL3](https://github.com/libsdl-org/SDL)
- [minizip-ng](https://github.com/zlib-ng/minizip-ng)

## Installation

In exactly one `.c` or `.cpp` file in your project, define the implementation macro before including the header:

```c
#define SDL_MINIZIP_IMPLEMENTATION
#include "SDL_minizip.h"
```

## Example

```c
#include <SDL3/SDL.h>

#define SDL_MINIZIP_IMPLEMENTATION
#include "SDL_minizip.h"

int main(int argc, char* argv[]) {
    SDL_Init(0);

    // Open a zip archive
    SDL_Storage *storage = SDL_OpenMinizipStorage("archive.zip");
    if (storage) {
        // Use SDL_Storage APIs
        SDL_PathInfo info;
        if (SDL_GetStoragePathInfo(storage, "file.txt", &info)) {
            char* data = SDL_malloc(info.size);
            SDL_ReadStorageFile(storage, "file.txt", data, info.size);
            // ...
            SDL_free(data);
        }

        SDL_CloseStorage(storage);
    }

    SDL_Quit();
    return 0;
}
```

## API

```c
SDL_Storage *SDL_OpenMinizipStorage(const char *file_path);
SDL_Storage *SDL_OpenMinizipStorage_IO(SDL_IOStream *src, bool closeio);
SDL_Storage *SDL_OpenMinizipStorage_Mem(const void *mem, size_t size);
void *SDL_LoadMinizipStorageFile(SDL_Storage *storage, const char *path, size_t *datasize);
SDL_IOStream *SDL_OpenMinizipStorageFile_IO(SDL_Storage *storage, const char *path);
```

## License

[Zlib](LICENSE)
