#pragma once
#include <wayland-server.h>
#include <stdio.h>
#include <map>
#include <sys/mman.h>
#include "backend.h"

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

struct surface
{
    surface(uint32_t id_, int x_, int y_)
    :id(id_), x(x_), y(y_)
    {
        damage_x = 0;
        damage_y = 0; 
        damage_width = 0;
        damage_height = 0;
    }
    uint32_t id;
    int x;
    int y;
    int damage_x;
    int damage_y;
    int damage_width;
    int damage_height;
    buffer pending_buff;
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

struct shm_pool
{
    shm_pool(int fd_, int size_, unsigned id_, char *data_)
    :fd(fd_), size(size_), id(id_), data(data_)
    {
        available_size = size;
    }
    int fd;
    int size;
    int available_size;
    unsigned id;
    char *data;
    std::map<wl_resource *, buffer> buffers;
};

void shm_create_pool(struct wl_client *client, struct wl_resource *resource, uint32_t id, int32_t fd, int32_t size);
void shm_release_pool(struct wl_client *client, struct wl_resource *resource);
void shm_pool_create_buffer(struct wl_client *client, struct wl_resource *resource, uint32_t id, int32_t offset, int32_t width, int32_t height, int32_t stride, uint32_t format);
void shm_pool_destroy(struct wl_client *client, struct wl_resource *resource);
void shm_pool_resize(struct wl_client *client, struct wl_resource *resource, int32_t size);
void destroy_buffer(struct wl_client *client, struct wl_resource *resource);
void bind_shm(struct wl_client *client,void *data,uint32_t version,uint32_t id);

static const struct wl_shm_interface wl_shm_implementation = 
{
    .create_pool = shm_create_pool,
    .release = shm_release_pool
};

static const struct wl_buffer_interface wl_buffer_implementation =
{
    .destroy = destroy_buffer,
};

static const struct wl_shm_pool_interface wl_shm_pool_implementation = 
{
    .create_buffer = shm_pool_create_buffer,
    .destroy = shm_pool_destroy,
    .resize = shm_pool_resize
};
void destroy_surface(struct wl_client *client, struct wl_resource *resource);
void attach_surface(struct wl_client *client, struct wl_resource *resource, struct wl_resource *buff, int32_t x, int32_t y);
void damage_surface(struct wl_client *client, struct wl_resource *resource, int32_t x, int32_t y, int32_t width, int32_t height);
void damage_buffer_surface(struct wl_client *client, struct wl_resource *resource, int32_t x, int32_t y, int32_t width, int32_t height);
void frame_surface(struct wl_client *client, struct wl_resource *resource, uint32_t callback); //NOT implmented
void set_opaque_region_surface(struct wl_client *client, struct wl_resource *resource, struct wl_resource *region); //NOT implemented
void set_input_region_surface(struct wl_client *client, struct wl_resource *resource, struct wl_resource *region); //NOT implemented
void commit_surface(struct wl_client *client, struct wl_resource *resource);
void set_buffer_transform_surface(struct wl_client *client, struct wl_resource *resource, int32_t transform);//NOT implemented
void set_buffer_scale_surface(struct wl_client *client, struct wl_resource *resource, int32_t scale);
void offset_surface(struct wl_client *client, struct wl_resource *resource, int32_t x, int32_t y);
void create_surface(struct wl_client *client, struct wl_resource *resource, uint32_t id);
void destroy_region(struct wl_client *client, struct wl_resource *resource);
void create_region(struct wl_client *client, struct wl_resource *resource,uint32_t id);

static const struct wl_compositor_interface wl_compositor_implementation =
{
    .create_surface = create_surface,
    .create_region = create_region
};

void bind_compositor(struct wl_client *client,void *data,uint32_t version,uint32_t id);

static const struct wl_surface_interface wl_surface_implementation = 
{
    .destroy = destroy_surface,
    .attach = attach_surface,
    .damage = damage_surface,
    .frame = frame_surface,
    .set_opaque_region = set_opaque_region_surface,
    .set_input_region = set_input_region_surface,
    .commit = commit_surface,
    .set_buffer_transform = set_buffer_transform_surface,
    .set_buffer_scale = set_buffer_scale_surface,
    .damage_buffer = damage_buffer_surface,
    .offset = offset_surface
};
