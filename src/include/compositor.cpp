#include "compositor.h"

std::map<wl_client *, region> regions;
std::map<wl_client *, surface> surfaces;
std::map<wl_client *, shm_pool> pools;

void shm_create_pool(struct wl_client *client, struct wl_resource *resource, uint32_t id, int32_t fd, int32_t size)
{
    resource = wl_resource_create(client, &wl_shm_pool_interface, wl_resource_get_version(resource), id);
    char *data = (char *)mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (!data)
    {
        printf("Failed to create memory pool\n");
        wl_client_post_no_memory(client);
        return;
    }
    printf("created pool with size %i\n", size);
    pools.emplace(client, shm_pool(fd, size, id, data));
    wl_resource_set_implementation(resource, &wl_shm_pool_implementation, nullptr, NULL);
}

void shm_release_pool(struct wl_client *client, struct wl_resource *resource)
{
    auto pool = pools.find(client);
    if (pool != pools.end())
    {
        if (munmap(0, pool->second.size) != 0)
        {
            printf("Unmapping memory pool failed\n");
            return;
        }
        pools.erase(client);
        printf("Memory pool unmapped succesfully\n");
    }
}

void shm_pool_create_buffer(struct wl_client *client, struct wl_resource *resource, uint32_t id, int32_t offset, int32_t width, int32_t height, int32_t stride, uint32_t format)
{
    if (width < 0 || height < 0 || stride < 0)
    {
        wl_resource_post_error(resource, WL_SHM_ERROR_INVALID_STRIDE, "Sizes are invalid");
        return;
    }

    if (wl_shm_format_is_valid(format, wl_resource_get_version(resource)) == false)
    {
        wl_resource_post_error(resource, WL_SHM_ERROR_INVALID_FORMAT, "Format is invalid");
        return; 
    }
    
    auto pool = pools.find(client);
    if (pool != pools.end())
    {
        if (((width * height) * (stride/width)) + offset > pool->second.available_size)
        {
            wl_resource_post_error(resource, WL_SHM_ERROR_INVALID_STRIDE, "Not enough size to fit buffer");
            return;
        }
        resource = wl_resource_create(client, &wl_buffer_interface, wl_resource_get_version(resource), id);
        pool->second.buffers.emplace(resource, buffer(&pool->second.data[offset], 100, 100, width, height, stride, format));
        if (!resource)
        {
            wl_client_post_no_memory(client);
            return;
        }
        wl_resource_set_implementation(resource, &wl_buffer_implementation, nullptr, NULL);
        printf("created buffer with size %i\n", (width * height) * (stride/width));
        pool->second.available_size -= (width * height) * (stride/width);
    }
}

void shm_pool_destroy(struct wl_client *client, struct wl_resource *resource)
{
    auto pool = pools.find(client);
    if (pool != pools.end())
    {
        if (munmap(pool->second.data, pool->second.size) != 0)
        {
            printf("Memory pool release failed\n");
            return ;
        }
        pools.erase(client);
    }
}

void shm_pool_resize(struct wl_client *client, struct wl_resource *resource, int32_t size)
{
    auto pool = pools.find(client);
    if (pool != pools.end())
    {
        if (size <= pool->second.size)
        {
            printf("Size not valid for resize\n");
            return ;
        }
        pool->second.data = (char *)mmap(pool->second.data, size, PROT_READ | PROT_WRITE, MAP_SHARED, pool->second.fd, 0);
        if (!pool->second.data)
        {
            wl_client_post_no_memory(client);
            printf("Pool resize failed\n");
            return;
        }
    }
}

void destroy_buffer(wl_client *client, wl_resource *resource)
{
    auto pool = pools.find(client);
    if (pool != pools.end())
    {
        auto buff = pool->second.buffers.find(resource);
        if (buff != pool->second.buffers.end())
        {
            pool->second.buffers.erase(resource);
        }
    }
}

void bind_shm(struct wl_client *client, void *data, uint32_t version, uint32_t id) 
{
    struct wl_resource *resource = wl_resource_create(client, &wl_shm_interface, version,id);
    if (!resource) 
    {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &wl_shm_implementation, nullptr, NULL);
    printf("Shm binded with id %i\n", id);
}

void destroy_surface(struct wl_client *client, struct wl_resource *resource)
{
    surfaces.erase(client);
}

void attach_surface(struct wl_client *client, struct wl_resource *resource, struct wl_resource *buff, int32_t x, int32_t y)
{
    auto pool = pools.find(client);
    if (pool != pools.end())
    {
        auto buff = pool->second.buffers.begin();
        surfaces.find(client)->second.pending_buff = buff->second;
        printf("attached buffer\n");
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
        buffer pending = surface->second.pending_buff;
        
        draw_fb(pending.content, pending.width, pending.height, pending.stride, pending.x, pending.y);
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