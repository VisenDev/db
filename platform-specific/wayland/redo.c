#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wayland-client.h>
#include <xkbcommon/xkbcommon.h>

#include "xdg-shell-client-protocol.h"
#include "xdg-shell-protocol.c"

typedef struct wl_display * Display;
typedef struct wl_registry * Registry;
typedef struct wl_surface * Surface;
typedef struct wl_compositor * Compositor;
typedef struct wl_interface Interface;
typedef struct wl_seat * Seat;
typedef struct wl_shm * Shm;
typedef struct xdg_wm_base * Base;

//// STATE

typedef struct {
    Compositor c;
    Seat s;
    Shm m;
    Base b;
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
    
    #define BIND(wl_interface, dest) \
        if(strcmp(interface, wl_interface.name) == 0) {                 \
            dest = wl_registry_bind(r, name, &wl_interface, version);   \
            printf("Bound global: %s\n", interface);                    \
            return;                                                     \
        }

    BIND(wl_compositor_interface, state->c);
    BIND(wl_seat_interface, state->s);
    BIND(wl_shm_interface, state->m);
    BIND(xdg_wm_base_interface, state->b);

    #undef BIND
}

void registry_listener_global_remove (
    void * data,
    Registry r,
    uint32_t name
) {
    /*TODO*/
}

static void xdg_wm_base_ping(void *data, Base b, uint32_t serial){
    xdg_wm_base_pong(b, serial);
}

int main() {
    State state = {0};
    Display display = wl_display_connect(NULL);
    Registry registry = wl_display_get_registry(display);
    wl_registry_add_listener(
        registry,
        &(struct wl_registry_listener){
            .global = registry_listener_global,
            .global_remove = registry_listener_global_remove
        },
        &state
    );
    wl_display_roundtrip(display);
    xdg_wm_base_add_listener(
        state.b,
        &(struct xdg_wm_base_listener){
            .ping = xdg_wm_base_ping
        },
        &state
    );

    Surface surface = wl_compositor_create_surface(state.c);


 }
