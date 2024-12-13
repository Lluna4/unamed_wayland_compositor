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
    pools.emplace(client, shm_pool(fd, size, id, data));
    //wl_resource_set_implementation(resource, &wl_shm_pool_implementation, nullptr, NULL);
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
        if (((width * height) * stride) + offset > pool->second.size)
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
