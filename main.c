#include <wayland-client.h>
#include <unistd.h>

#include "./3rdparty/raylib/raylib.h"
#include "./3rdparty/sqlite/sqlite3.h"
#define CORE_IMPLEMENTATION
#include "./3rdparty/core.h/core.h"

char * read_whole_file(core_Arena * a, const char * filepath) {
    FILE * fp = fopen(filepath, "r");
    assert(fp);
    fseek(fp, 0, SEEK_END);
    int length = ftell(fp);
    char * buf = core_arena_alloc(a, length + 1);
    memset(buf, 0, length + 1);
    assert(buf);
    fseek(fp, 0, SEEK_SET);
    fread(buf, 1, length, fp);
    fclose(fp);
    return buf;
}

void create_new_db(const char * filepath) {
    sqlite3 * db = NULL;
    if(sqlite3_open(filepath, &db) != 0) CORE_FATAL_ERROR("failed to create new db");
    core_Arena arena = {0};
    char * err = NULL;
    sqlite3_exec(db, read_whole_file(&arena, "sql/new_db.sql"), NULL, NULL, &err);
    if(err != NULL) CORE_FATAL_ERROR("%s", err);
    core_arena_free(&arena);
    sqlite3_close(db);
}


struct our_state {
    // ...
    struct wl_compositor *compositor;
    struct wl_display *display;
    struct wl_shm *shm;
    // ...
};

static void
registry_handle_global(void *data, struct wl_registry *wl_registry,
		uint32_t name, const char *interface, uint32_t version)
{
    (void)version;
    struct our_state *state = data;
    if (strcmp(interface, wl_compositor_interface.name) == 0) {
        state->compositor = wl_registry_bind(
            wl_registry, name, &wl_compositor_interface, 4);
    }
    if (strcmp(interface, wl_shm_interface.name) == 0) {
        state->shm = wl_registry_bind(
            wl_registry, name, &wl_shm_interface, 1);
    }

}


static void
registry_handle_global_remove(void *data, struct wl_registry *registry,
		uint32_t name)
{
    (void)data; (void)registry; (void)name;
	// This space deliberately left blank
}

static const struct wl_registry_listener
registry_listener = {
	.global = registry_handle_global,
	.global_remove = registry_handle_global_remove,
};

#include "shm.c"

int main() {
    //    create_new_db("main.db");
    //    InitWindow(1000, 1000, "hello");

    //while(!WindowShouldClose()) {
    //    BeginDrawing();
    //    ClearBackground(RAYWHITE);
    //    EndDrawing();
    //}
    //
    //CloseWindow();

    struct wl_display *display = wl_display_connect(NULL);
    if (!display) {
        fprintf(stderr, "Failed to connect to Wayland display.\n");
        return 1;
    }
    fprintf(stderr, "Connection established!\n");

    struct our_state state = { 0 };
    state.display = display;
	struct wl_registry *registry = wl_display_get_registry(display);
    wl_registry_add_listener(registry, &registry_listener, &state);
	wl_display_roundtrip(display);

    struct wl_surface *surface = wl_compositor_create_surface(state.compositor);

    const int width = 1920, height = 1080;
    const int stride = width * 4;
    const int shm_pool_size = height * stride * 2;

    int fd = allocate_shm_file(shm_pool_size);
    uint8_t *pool_data = mmap(NULL, shm_pool_size,
                              PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    struct wl_shm *shm = state.shm;
    struct wl_shm_pool *pool = wl_shm_create_pool(shm, fd, shm_pool_size);

    int index = 0;
    int offset = height * stride * index;
    struct wl_buffer *buffer = wl_shm_pool_create_buffer(pool, offset,
                                                         width, height, stride, WL_SHM_FORMAT_XRGB8888);


    uint32_t *pixels = (uint32_t *)&pool_data[offset];
    memset(pixels, 0, width * height * 4);

    wl_surface_attach(surface, buffer, 0, 0);
    wl_surface_damage(surface, 0, 0, UINT32_MAX, UINT32_MAX);
    wl_surface_commit(surface);

    while (wl_display_dispatch(state.display)) {
        /* This space deliberately left blank */
    }


    wl_display_disconnect(display);
    return 0;

}
