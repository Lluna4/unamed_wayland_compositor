#include "shm.h"

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
        pool->second.buffers.emplace(resource, buffer(&pool->second.data[offset], 0, 0, width, height, stride, format));
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
