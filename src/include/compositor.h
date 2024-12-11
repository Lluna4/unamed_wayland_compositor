#pragma once
#include <wayland-server.h>
#include <stdio.h>
#include <map>

struct buffer
{
    buffer()
    {
        content = nullptr;
        x = 0;
        y = 0;
        damage_x = 0;
        damage_y = 0; 
        damage_width = 0;
        damage_height = 0;
    }
    buffer(wl_resource *content_, int x_, int y_)
    :content(content_), x(x_), y(y_)
    {}
    wl_resource *content;
    int x;
    int y;
    int damage_x;
    int damage_y;
    int damage_width;
    int damage_height;
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

void destroy_surface(struct wl_resource *resource);
void attach_surface(struct wl_client *client, struct wl_resource *resource, struct wl_resource *buff, int32_t x, int32_t y);
void create_surface(struct wl_client *client, struct wl_resource *resource, uint32_t id);
void destroy_region(struct wl_resource *resource);
void create_region(struct wl_client *client,struct wl_resource *resource,uint32_t id);

static const struct wl_compositor_interface wl_compositor_implementation =
{
    .create_surface = create_surface,
    .create_region = create_region
};

void bind_compositor(struct wl_client *client,void *data,uint32_t version,uint32_t id);
