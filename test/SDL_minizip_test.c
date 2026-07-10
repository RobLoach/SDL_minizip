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
    SDL_IOStream *io = SDL_IOFromFile("test/resources/test.zip", "r");
    if (!io) {
        io = SDL_IOFromFile("resources/test.zip", "r");
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

    // Test flat file count
    int count = SDL_GetMinizipStorageFileCount(storage);
    if (count < 1) {
        SDL_Log("Expected at least 1 file, got %d: %s", count, SDL_GetError());
        SDL_CloseStorage(storage);
        SDL_Quit();
        return 1;
    }
    SDL_Log("SDL_GetMinizipStorageFileCount: %d", count);

    // Test flat file path retrieval
    char *path = SDL_GetMinizipStorageFilePath(storage, 0);
    if (!path) {
        SDL_Log("SDL_GetMinizipStorageFilePath(0) returned NULL: %s", SDL_GetError());
        SDL_CloseStorage(storage);
        SDL_Quit();
        return 1;
    }
    SDL_Log("SDL_GetMinizipStorageFilePath(0): %s", path);
    SDL_free(path);

    // Out-of-range should return NULL
    if (SDL_GetMinizipStorageFilePath(storage, count) != NULL) {
        SDL_Log("Expected NULL for out-of-range index");
        SDL_CloseStorage(storage);
        SDL_Quit();
        return 1;
    }
    SDL_Log("Out-of-range index correctly returned NULL");

    SDL_CloseStorage(storage);

    SDL_Quit();
    return 0;
}
