#pragma once
#include <wayland-server.h>
#include <stdio.h>
#include <map>
#include "shm.h"

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
