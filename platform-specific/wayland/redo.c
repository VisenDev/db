#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wayland-client.h>

struct wl_registry_listener {
	void (*global)(void *data,
		       struct wl_registry *wl_registry,
		       uint32_t name,
		       const char *interface,
		       uint32_t version);
	void (*global_remove)(void *data,
			      struct wl_registry *wl_registry,
			      uint32_t name);
};


typedef struct wl_display * Display;
typedef struct wl_registry * Registry;
typedef struct wl_registry_listener RegistryListener;
typedef struct wl_surface Surface;
typedef struct wl_compositor * Compositor;
typedef struct wl_interface Interface;
typedef struct wl_seat * Seat;

//// STATE

typedef struct {
//    Registry r;
    Compositor c;
    Seat s;
} State;



//// REGISTRY

void registry_listener_global (
    void * data,
    Registry r,
    uint32_t name,
    const char * interface,
    uint32_t version
) {
    State * state = data;
    printf("Found global: %s\n", interface);

    #define BIND(wl_interface, dest) \
        if(strcmp(interface, wl_interface.name) == 0) {                 \
            dest = wl_registry_bind(r, name, &wl_interface, version); \
            return;                                                     \
        }

    BIND(wl_compositor_interface, state->c);
    BIND(wl_seat_interface, state->s);

    #undef BIND
}

void registry_listener_global_remove (
    void * data,
    Registry r,
    uint32_t name
) {
    /*TODO*/
}


int main() {
    State state = {0};
    Display display = wl_display_connect(NULL);
    Registry registry = wl_display_get_registry(display);
//    state.r = registry;
    wl_registry_add_listener(
        registry,
        &(RegistryListener){
            .global = registry_listener_global,
            .global_remove = registry_listener_global_remove
        },
        &state
    );
    wl_display_roundtrip(display);

    //   Surface surface = wl_compositor_create_surface(


 }
