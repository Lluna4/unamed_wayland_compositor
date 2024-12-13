#include "compositor.h"

std::map<wl_client *, region> regions;
std::map<wl_client *, surface> surfaces;
void destroy_surface(struct wl_client *client, struct wl_resource *resource)
{
    surfaces.erase(client);
}

void attach_surface(struct wl_client *client, struct wl_resource *resource, struct wl_resource *buff, int32_t x, int32_t y)
{
    auto pool = pools.find(client);
    if (pool != pools.end())
    {
        auto buff = pool->second.buffers.find(resource);
        if (buff != pool->second.buffers.end())
        {
            surfaces.find(client)->second.pending_buff = buff->second;
        }
    }
}
void damage_surface(struct wl_client *client, struct wl_resource *resource, int32_t x, int32_t y, int32_t width, int32_t height)
{
    auto surface = surfaces.find(client);
    if (surface != surfaces.end())
    {
        surface->second.damage_height = height;
        surface->second.damage_width = width;
        surface->second.damage_x = x;
        surface->second.damage_y = y;
    }
}

void damage_buffer_surface(struct wl_client *client, struct wl_resource *resource, int32_t x, int32_t y, int32_t width, int32_t height)
{
    auto surface = surfaces.find(client);
    if (surface != surfaces.end())
    {
        surface->second.pending_buff.damage_height = height;
        surface->second.pending_buff.damage_width = width;
        surface->second.pending_buff.damage_x = x;
        surface->second.pending_buff.damage_y = y;
    }
}

void frame_surface(struct wl_client *client, struct wl_resource *resource, uint32_t callback)
{
    printf("wl_surface.frame() is not implmented\n");

}

void set_opaque_region_surface(struct wl_client *client, struct wl_resource *resource, struct wl_resource *region)
{
    printf("wl_surface.set_opaque_region() is not implmented\n");
}

void set_input_region_surface(struct wl_client *client, struct wl_resource *resource, struct wl_resource *region)
{
    printf("wl_surface.set_input_region() is not implmented\n");
}

void commit_surface(struct wl_client *client, struct wl_resource *resource)
{
    auto surface = surfaces.find(client);
    if (surface != surfaces.end())
    {
        //send signal to backend to print to buffer and switch buffers
    }
}

void set_buffer_transform_surface(struct wl_client *client, struct wl_resource *resource, int32_t transform)
{
    printf("wl_surface.set_buffer_transform() is not implmented %i\n", transform);
}

void set_buffer_scale_surface(struct wl_client *client, struct wl_resource *resource, int32_t scale)
{
    auto surface = surfaces.find(client);
    if (surface != surfaces.end())
    {
        surface->second.pending_buff.buffer_scale = scale;
        printf("New buffer scale is %i\n", scale);
    }
}

void offset_surface(struct wl_client *client, struct wl_resource *resource, int32_t x, int32_t y)
{
    auto surface = surfaces.find(client);
    if (surface != surfaces.end()) 
    {
        surface->second.x = x;
        surface->second.y = y;
        printf("New offset x: %i, y: %i\n", x, y);
    }
}

void create_surface(struct wl_client *client, struct wl_resource *resource, uint32_t id)
{
    resource = wl_resource_create(client, &wl_surface_interface, wl_resource_get_version(resource), id);
    if (!resource)
    {
        printf("surface creation failed!\n");
        return;
    }
    printf("created surface with id %i\n", id);
    surfaces.emplace(client, surface(id, 0, 0));
    wl_resource_set_implementation(resource, &wl_surface_implementation, nullptr, NULL);
}

void destroy_region(struct wl_client *client, struct wl_resource *resource)
{
    regions.erase(client);
}

void create_region(struct wl_client *client,struct wl_resource *resource, uint32_t id)
{
    resource = wl_resource_create(client, &wl_region_interface, wl_resource_get_version(resource), id);
    if (!resource)
    {
        printf("region creation failed!\n");
        return;
    }
    regions.emplace(client, region(id, 0, 0));
    //wl_resource_set_implementation(ret, &wl_region_interface, id_ptr, destroy_region); TODO set actual implementation
}


void bind_compositor(struct wl_client *client,void *data,uint32_t version,uint32_t id) 
{
    struct wl_resource *resource = wl_resource_create(client, &wl_compositor_interface, version,id);
    if (!resource) 
    {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &wl_compositor_implementation, nullptr, NULL);
    printf("compositor binded with id %i\n", id);
}