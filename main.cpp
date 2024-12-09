#include <wayland-server.h>
#include <stdio.h>
#include <map>

struct surface
{
    surface(uint32_t id_, int x_, int y_)
    :id(id_), x(x_), y(y_)
    {}
    uint32_t id;
    int x;
    int y;
};

struct region
{
    region(uint32_t id_, int x_, int y_)
    :id(id_), x(x_), y(y_)
    {}
    uint32_t id;
    int x;
    int y;
};

std::map<uint32_t, surface> surfaces;
std::map<uint32_t, region> regions;

void destroy_surface(struct wl_resource *resource)
{
    surfaces.erase(*(uint32_t *)(resource->data));
    free(resource->data);
}

void destroy_region(struct wl_resource *resource)
{
    regions.erase(*(uint32_t *)(resource->data));
    free(resource->data);
}

void create_surface(struct wl_client *client, struct wl_resource *resource, uint32_t id)
{
    wl_resource *ret = wl_resource_create(client, &wl_surface_interface, wl_resource_get_version(resource), id);
    if (!ret)
    {
        printf("surface creation failed!\n");
        return;
    }
    uint32_t *id_ptr = (uint32_t *)malloc(1 * sizeof(uint32_t));
    surfaces.emplace(id, surface(id, 0, 0));
    *id_ptr = id;
    wl_resource_set_implementation(ret, &wl_surface_interface, id_ptr, destroy_surface);
}

void create_region(struct wl_client *client,struct wl_resource *resource,uint32_t id)
{
    wl_resource *ret = wl_resource_create(client, &wl_region_interface, wl_resource_get_version(resource), id);
    if (!ret)
    {
        printf("surface creation failed!\n");
        return;
    }
    uint32_t *id_ptr = (uint32_t *)malloc(1 * sizeof(uint32_t));
    regions.emplace(id, region(id, 0, 0));
    *id_ptr = id;
    wl_resource_set_implementation(ret, &wl_surface_interface, id_ptr, destroy_region);
}

static const struct wl_compositor_interface wl_compositor_implementation =
{
    .create_surface = create_surface,
    .create_region = create_region
};

static void bind_compositor(struct wl_client *client,void *data,uint32_t version,uint32_t id) 
{
    struct wl_resource *resource = wl_resource_create(client, &wl_compositor_interface,version,id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &wl_compositor_implementation,data, NULL);
}
int main()
{
    wl_display *disp = wl_display_create();
    if (!disp)
    {
        printf("Failed to start wayland display\n");
        return -1;
    }
    const char *sock = wl_display_add_socket_auto(disp);
    if (!sock)
    {
        printf("Failed to bind to socket\n");
        return -1;
    }
    wl_global *global_compositor = wl_global_create(disp, &wl_compositor_interface, 4, NULL, bind_compositor);
    if (!global_compositor)
    {
        printf("Failed to create compositor global\n");
        return -1;
    }
    printf("Wayland display is running on %s!\n", sock);

    wl_display_run(disp);
    wl_display_destroy(disp);
    return 0;
}