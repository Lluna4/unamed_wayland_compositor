#pragma once
#include <sys/mman.h>
#include <wayland-server.h>
#include <map>
#include <stdio.h>

struct buffer
{
    buffer()
    {
        content = nullptr;
        x = 0;
        y = 0;
        width = 0;
        height = 0;
        stride = 0;
        format = 0;
        damage_x = 0;
        damage_y = 0; 
        damage_width = 0;
        damage_height = 0;
    }
    buffer(char *content_, int x_, int y_, int width_, int height_, int stride_, unsigned format_)
    :content(content_), x(x_), y(y_), width(width_), height(height_), stride(stride_), format(format_)
    {}
    char *content;
    int width;
    int height;
    int stride;
    unsigned format;
    int x;
    int y;
    int damage_x;
    int damage_y;
    int damage_width;
    int damage_height;
    int buffer_scale;
};

struct shm_pool
{
    shm_pool(int fd_, int size_, unsigned id_, char *data_)
    :fd(fd_), size(size_), id(id_), data(data_)
    {}
    int fd;
    int size;
    unsigned id;
    char *data;
    std::map<wl_resource *, buffer> buffers;
};

static std::map<wl_client *, shm_pool> pools;

void shm_create_pool(struct wl_client *client, struct wl_resource *resource, uint32_t id, int32_t fd, int32_t size);
void shm_release_pool(struct wl_client *client, struct wl_resource *resource);
void shm_pool_create_buffer(struct wl_client *client, struct wl_resource *resource, uint32_t id, int32_t offset, int32_t width, int32_t height, int32_t stride, uint32_t format);
void destroy_buffer(struct wl_client *client, struct wl_resource *resource);
void bind_shm(struct wl_client *client,void *data,uint32_t version,uint32_t id);

static const struct wl_shm_interface wl_shm_implementation = 
{
    .create_pool = shm_create_pool,
    .release = shm_release_pool
};

static const struct wl_buffer_interface wl_buffer_implementation =
{
    .destroy = destroy_buffer
};

static const struct wl_shm_pool_interface wl_shm_pool_implementation = 
{
    .create_buffer = shm_pool_create_buffer,
};