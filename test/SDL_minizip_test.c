#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>

#define SDL_MINIZIP_IMPLEMENTATION
#include "../SDL_minizip.h"
#include <stdbool.h>

static SDL_EnumerationResult SDLCALL enum_callback(void *userdata, const char *dirname, const char *fname) {
    bool *found = (bool *)userdata;
    if (SDL_strcmp(fname, "hello.txt") == 0) {
        *found = true;
    }
    return SDL_ENUM_CONTINUE;
}

int main(int argc, char *argv[]) {
    SDL_Init(0);

    // Load test.zip
    SDL_IOStream *io = SDL_IOFromFile("test/resources/test.zip", "rb");
    if (!io) {
        io = SDL_IOFromFile("resources/test.zip", "rb");
        if (!io) {
            SDL_Log("Could not find test.zip: %s", SDL_GetError());
            SDL_Quit();
            return 1;
        }
    }

    // SDL_OpenMinizipStorage_IO()
    SDL_Storage *storage = SDL_OpenMinizipStorage_IO(io, true);
    if (!storage) {
        SDL_Log("Failed to open storage via IOStream: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Ensure it gets hello.txt correctly.
    SDL_PathInfo info;
    if (!SDL_GetStoragePathInfo(storage, "hello.txt", &info)) {
        SDL_Log("hello.txt not found in zip");
        SDL_CloseStorage(storage);
        SDL_Quit();
        return 1;
    }

    if (info.size == 0) {
        SDL_Log("Unexpected empty size for hello.txt");
        SDL_CloseStorage(storage);
        SDL_Quit();
        return 1;
    }

    // Read the file.
    char *content = (char *)SDL_malloc(info.size + 1);
    if (!content) {
        SDL_CloseStorage(storage);
        SDL_Quit();
        return 1;
    }

    if (!SDL_ReadStorageFile(storage, "hello.txt", content, info.size)) {
        SDL_Log("Failed to read file");
        SDL_free(content);
        SDL_CloseStorage(storage);
        SDL_Quit();
        return 1;
    }

    content[info.size] = '\0';
    size_t len = info.size;
    while (len > 0 && (content[len - 1] == '\n' || content[len - 1] == '\r')) {
        content[len - 1] = '\0';
        len--;
    }

    if (SDL_strcmp(content, "Hello World from minizip-ng inside SDL_Storage!") != 0) {
        SDL_Log("Content mismatch! Got: '%s'", content);
        SDL_free(content);
        SDL_CloseStorage(storage);
        SDL_Quit();
        return 1;
    }

    SDL_Log("Successfully tested custom IOStream! Content: %s", content);
    SDL_free(content);

    // Test enumeration
    bool found_hello = false;
    if (!SDL_EnumerateStorageDirectory(storage, "", enum_callback, &found_hello)) {
        SDL_Log("Failed to enumerate directory: %s", SDL_GetError());
        SDL_CloseStorage(storage);
        SDL_Quit();
        return 1;
    }
    
    if (!found_hello) {
        SDL_Log("hello.txt was not found during enumeration!");
        SDL_CloseStorage(storage);
        SDL_Quit();
        return 1;
    }
    
    SDL_Log("Successfully enumerated directory and found hello.txt!");

    // Test SDL_LoadMinizipStorageFile
    size_t loaded_size = 0;
    void *loaded = SDL_LoadMinizipStorageFile(storage, "hello.txt", &loaded_size);
    if (!loaded) {
        SDL_Log("SDL_LoadMinizipStorageFile failed: %s", SDL_GetError());
        SDL_CloseStorage(storage);
        SDL_Quit();
        return 1;
    }
    if (loaded_size == 0) {
        SDL_Log("Expected non-zero loaded_size");
        SDL_free(loaded);
        SDL_CloseStorage(storage);
        SDL_Quit();
        return 1;
    }
    SDL_Log("SDL_LoadMinizipStorageFile: %zu bytes, starts with: %.12s", loaded_size, (char *)loaded);
    SDL_free(loaded);

    // Test without datasize output
    void *loaded2 = SDL_LoadMinizipStorageFile(storage, "hello.txt", NULL);
    if (!loaded2) {
        SDL_Log("SDL_LoadMinizipStorageFile (null size) failed: %s", SDL_GetError());
        SDL_CloseStorage(storage);
        SDL_Quit();
        return 1;
    }
    SDL_free(loaded2);
    SDL_Log("SDL_LoadMinizipStorageFile with NULL datasize: OK");

    // Test SDL_OpenMinizipStorageFile_IO
    SDL_IOStream *entry_io = SDL_OpenMinizipStorageFile_IO(storage, "hello.txt");
    if (!entry_io) {
        SDL_Log("SDL_OpenMinizipStorageFile_IO failed: %s", SDL_GetError());
        SDL_CloseStorage(storage);
        SDL_Quit();
        return 1;
    }

    Sint64 io_size = SDL_GetIOSize(entry_io);
    if (io_size <= 0) {
        SDL_Log("Expected positive IOStream size, got %" SDL_PRIs64, io_size);
        SDL_CloseIO(entry_io);
        SDL_CloseStorage(storage);
        SDL_Quit();
        return 1;
    }

    char *io_content = (char *)SDL_malloc((size_t)io_size + 1);
    if (!io_content) {
        SDL_CloseIO(entry_io);
        SDL_CloseStorage(storage);
        SDL_Quit();
        return 1;
    }
    if ((Sint64)SDL_ReadIO(entry_io, io_content, (size_t)io_size) != io_size) {
        SDL_Log("Failed to read from IOStream: %s", SDL_GetError());
        SDL_free(io_content);
        SDL_CloseIO(entry_io);
        SDL_CloseStorage(storage);
        SDL_Quit();
        return 1;
    }
    io_content[io_size] = '\0';

    // Seek back to start and read again to test seekability
    if (SDL_SeekIO(entry_io, 0, SDL_IO_SEEK_SET) != 0) {
        SDL_Log("Seek failed: %s", SDL_GetError());
        SDL_free(io_content);
        SDL_CloseIO(entry_io);
        SDL_CloseStorage(storage);
        SDL_Quit();
        return 1;
    }

    SDL_Log("SDL_OpenMinizipStorageFile_IO: size=%" SDL_PRIs64 ", seekable, content starts with: %.12s", io_size, io_content);
    SDL_free(io_content);
    SDL_CloseIO(entry_io);

    SDL_CloseStorage(storage);

    SDL_Quit();
    return 0;
}
