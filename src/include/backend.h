#pragma once
#include <libdrm/drm.h>
#include <libdrm/drm_mode.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <sys/epoll.h>
#include <unistd.h>
#include <linux/input.h>
#include <filesystem>
#include <chrono>

void page_flip_handler(int fd, unsigned int sequence, unsigned int tv_sec, unsigned int tv_usec, void *user_data);
void add_to_list(int fd, int epfd);
void delete_cursor(int m_pos_x, int m_pos_y, int width ,char *framebuffer);
void draw_cursor(int m_pos_x, int m_pos_y, int width ,char *framebuffer);
float lerp(float v0, float v1, float t);
void mouse_read();
void keyboard_read();
void wait_ep(int epfd, int fd);
void draw_fb(char *data, int width, int height, int stride, int x, int y);
int start(int target_refresh);
