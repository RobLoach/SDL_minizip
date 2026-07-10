/*
 * SDL_minizip_test.c — SDL_minizip test suite using the SDL_test harness.
 */
#define SDL_MINIZIP_IMPLEMENTATION
#include "../SDL_minizip.h"

#include <SDL3/SDL_test.h>

#define CHECK(cond, msg)   SDLTest_AssertCheck((cond) ? 1 : 0, "%s", (msg))
#define CHECK_STR(a, b, msg) \
    do { \
        const char *sa_ = (a), *sb_ = (b); \
        SDLTest_AssertCheck(sa_ && sb_ && SDL_strcmp(sa_, sb_) == 0, \
            "%s (got \"%s\", expected \"%s\")", (msg), \
            sa_ ? sa_ : "(null)", sb_ ? sb_ : "(null)"); \
    } while (0)

static const char *zip_path(void)
{
    static const char *paths[] = { "resources/test.zip", "test/resources/test.zip", NULL };
    for (int i = 0; paths[i]; ++i) {
        SDL_IOStream *io = SDL_IOFromFile(paths[i], "rb");
        if (io) { SDL_CloseIO(io); return paths[i]; }
    }
    return paths[0];
}

static SDL_EnumerationResult SDLCALL find_hello(void *userdata, const char *dir, const char *name)
{
    (void)dir;
    if (SDL_strcmp(name, "hello.txt") == 0) *(bool *)userdata = true;
    return SDL_ENUM_CONTINUE;
}

static const char *EXPECTED = "Hello World from minizip-ng inside SDL_Storage!";

static char *read_entry(SDL_Storage *storage)
{
    Uint64 sz = 0;
    if (!SDL_GetStorageFileSize(storage, "hello.txt", &sz) || sz == 0) return NULL;
    char *buf = (char *)SDL_malloc((size_t)sz + 1);
    if (!buf) return NULL;
    if (!SDL_ReadStorageFile(storage, "hello.txt", buf, sz)) { SDL_free(buf); return NULL; }
    buf[sz] = '\0';
    size_t len = sz;
    while (len > 0 && (buf[len-1] == '\n' || buf[len-1] == '\r')) buf[--len] = '\0';
    return buf;
}

static int SDLCALL test_open_iostream(void *arg)
{
    (void)arg;
    SDL_IOStream *io = SDL_IOFromFile(zip_path(), "rb");
    CHECK(io != NULL, "open test.zip as IOStream");
    if (!io) return TEST_COMPLETED;

    SDL_Storage *storage = SDL_OpenMinizipStorage_IO(io, true);
    CHECK(storage != NULL, "SDL_OpenMinizipStorage_IO succeeds");
    if (!storage) return TEST_COMPLETED;

    char *content = read_entry(storage);
    CHECK(content != NULL, "read hello.txt");
    if (content) { CHECK_STR(content, EXPECTED, "hello.txt content matches"); SDL_free(content); }

    SDL_CloseStorage(storage);
    return TEST_COMPLETED;
}

static int SDLCALL test_load_file(void *arg)
{
    (void)arg;
    SDL_Storage *storage = SDL_OpenMinizipStorage(zip_path());
    CHECK(storage != NULL, "open storage for load-file test");
    if (!storage) return TEST_COMPLETED;

    size_t loaded_size = 0;
    void *loaded = SDL_LoadMinizipStorageFile(storage, "hello.txt", &loaded_size);
    CHECK(loaded != NULL, "SDL_LoadMinizipStorageFile succeeds");
    CHECK(loaded_size > 0, "loaded_size is non-zero");
    if (loaded) {
        CHECK(((char *)loaded)[loaded_size] == '\0', "buffer is null-terminated");
        SDL_free(loaded);
    }

    void *loaded2 = SDL_LoadMinizipStorageFile(storage, "hello.txt", NULL);
    CHECK(loaded2 != NULL, "SDL_LoadMinizipStorageFile with NULL datasize succeeds");
    SDL_free(loaded2);

    SDL_CloseStorage(storage);
    return TEST_COMPLETED;
}

static int SDLCALL test_open_filepath(void *arg)
{
    (void)arg;
    SDL_Storage *storage = SDL_OpenMinizipStorage(zip_path());
    CHECK(storage != NULL, "SDL_OpenMinizipStorage succeeds");
    if (!storage) return TEST_COMPLETED;

    char *content = read_entry(storage);
    CHECK(content != NULL, "read hello.txt via file path");
    if (content) { CHECK_STR(content, EXPECTED, "hello.txt content matches"); SDL_free(content); }

    SDL_CloseStorage(storage);
    return TEST_COMPLETED;
}

static int SDLCALL test_open_memory(void *arg)
{
    (void)arg;
    size_t zip_size = 0;
    void *zip_data = SDL_LoadFile(zip_path(), &zip_size);
    CHECK(zip_data != NULL, "load zip into memory");
    if (!zip_data) return TEST_COMPLETED;

    SDL_Storage *storage = SDL_OpenMinizipStorage_Mem(zip_data, zip_size);
    CHECK(storage != NULL, "SDL_OpenMinizipStorage_Mem succeeds");
    if (storage) {
        char *content = read_entry(storage);
        CHECK(content != NULL, "read hello.txt from memory");
        if (content) { CHECK_STR(content, EXPECTED, "hello.txt content matches"); SDL_free(content); }
        SDL_CloseStorage(storage);
    }

    SDL_free(zip_data);
    return TEST_COMPLETED;
}

static int SDLCALL test_enumerate(void *arg)
{
    (void)arg;
    SDL_Storage *storage = SDL_OpenMinizipStorage(zip_path());
    CHECK(storage != NULL, "open storage for enumeration");
    if (!storage) return TEST_COMPLETED;

    bool found = false;
    bool ok = SDL_EnumerateStorageDirectory(storage, "", find_hello, &found);
    CHECK(ok, "SDL_EnumerateStorageDirectory returns true");
    CHECK(found, "hello.txt found during enumeration");

    SDL_CloseStorage(storage);
    return TEST_COMPLETED;
}

static int SDLCALL test_pathinfo(void *arg)
{
    (void)arg;
    SDL_Storage *storage = SDL_OpenMinizipStorage(zip_path());
    CHECK(storage != NULL, "open storage for pathinfo");
    if (!storage) return TEST_COMPLETED;

    SDL_PathInfo info;
    SDL_memset(&info, 0, sizeof(info));
    bool ok = SDL_GetStoragePathInfo(storage, "hello.txt", &info);
    CHECK(ok, "SDL_GetStoragePathInfo returns true");
    CHECK(info.type == SDL_PATHTYPE_FILE, "type is SDL_PATHTYPE_FILE");
    CHECK(info.size > 0, "size is non-zero");

    SDL_CloseStorage(storage);
    return TEST_COMPLETED;
}

static int SDLCALL test_null_safety(void *arg)
{
    (void)arg;
    SDL_Storage *s;

    s = SDL_OpenMinizipStorage(NULL);
    CHECK(s == NULL, "SDL_OpenMinizipStorage(NULL) returns NULL");

    s = SDL_OpenMinizipStorage_IO(NULL, false);
    CHECK(s == NULL, "SDL_OpenMinizipStorage_IO(NULL) returns NULL");

    s = SDL_OpenMinizipStorage_Mem(NULL, 0);
    CHECK(s == NULL, "SDL_OpenMinizipStorage_Mem(NULL, 0) returns NULL");

    return TEST_COMPLETED;
}

static int SDLCALL test_missing_file(void *arg)
{
    (void)arg;
    SDL_Storage *storage = SDL_OpenMinizipStorage(zip_path());
    CHECK(storage != NULL, "open storage for missing-file test");
    if (!storage) return TEST_COMPLETED;

    Uint64 sz = 0;
    bool ok = SDL_GetStorageFileSize(storage, "does_not_exist.txt", &sz);
    CHECK(!ok, "GetStorageFileSize on missing file returns false");

    SDL_CloseStorage(storage);
    return TEST_COMPLETED;
}

#define CASE(fn, desc) &(const SDLTest_TestCaseReference){ fn, #fn, desc, TEST_ENABLED }

static const SDLTest_TestCaseReference *minizipTestCases[] = {
    CASE(test_open_iostream, "Open via SDL_IOStream"),
    CASE(test_open_filepath, "Open via file path"),
    CASE(test_open_memory,   "Open via memory buffer"),
    CASE(test_load_file,     "SDL_LoadMinizipStorageFile"),
    CASE(test_enumerate,     "Enumerate root directory"),
    CASE(test_pathinfo,      "SDL_GetStoragePathInfo"),
    CASE(test_null_safety,   "NULL argument safety"),
    CASE(test_missing_file,  "Missing file returns false"),
    NULL
};

static SDLTest_TestSuiteReference minizipSuite = {
    "SDL_minizip", NULL, minizipTestCases, NULL
};

static SDLTest_TestSuiteReference *testSuites[] = { &minizipSuite, NULL };

int main(int argc, char *argv[])
{
    SDLTest_CommonState *state = SDLTest_CommonCreateState(argv, 0);
    if (!state) return 1;

    SDLTest_TestSuiteRunner *runner = SDLTest_CreateTestSuiteRunner(state, testSuites);
    int result = SDLTest_ExecuteTestSuiteRunner(runner);
    SDLTest_DestroyTestSuiteRunner(runner);
    SDLTest_CommonDestroyState(state);
    return result;
}
