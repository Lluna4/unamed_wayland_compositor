#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <thread>

struct wl_compositor *compositor = NULL;
struct wl_shm *shm = NULL;
wl_surface *surface = NULL;
struct wl_buffer *frame_buff;

struct colors
{
	int r, g, b;
};

struct colors random_color()
{
	int r, g, b;
	r = random()%256;
	g = random()%256;
	b = random()%256;
	struct colors a = {r, g, b};

	return a;
}

void reg_global(void *data, struct wl_registry *wl_registry, uint32_t name, const char *interface, uint32_t version)
{
    printf("Interface: %s, Version: %u\n", interface, version);
    
    if (strcmp(interface, "wl_compositor") == 0) 
    {
        compositor = (struct wl_compositor *)wl_registry_bind(wl_registry, name, &wl_compositor_interface, version);
        printf("Compositor bound successfully\n");
    }
    if (strcmp(interface, "wl_shm") == 0)
    {
        shm = (struct wl_shm *)wl_registry_bind(wl_registry, name, &wl_shm_interface, version);
        printf("shm bound successfully\n");
    }
}

void reg_global_remove(void *data, struct wl_registry *wl_registry, uint32_t name)
{
    printf("Global removed: %u\n", name);
}

struct wl_registry_listener listener = {
    .global = reg_global, 
    .global_remove = reg_global_remove
};

int32_t allocate_shared_memory(uint64_t size)
{
    int8_t *name = (int8_t *)calloc(9, sizeof(int8_t));
    name[0] = '/';
    name[7] = 0;
    for (int i = 1; i < 6; i++)
        name[i] = 'a' + i;
    
    int32_t fd = shm_open("/abcd", O_RDWR | O_EXCL | O_CREAT, S_IWUSR | S_IRUSR | S_IWOTH | S_IROTH);
    shm_unlink("/abcd");
    ftruncate(fd, size);
    free(name);
    return fd;
}

int main()
{
    srandom(time(NULL));
    struct wl_display *disp = wl_display_connect(NULL);
    if (!disp) {
        fprintf(stderr, "Failed to connect to Wayland display\n");
        return -1;
    }
    printf("Display connected!\n");

    struct wl_registry *reg = wl_display_get_registry(disp);
    wl_registry_add_listener(reg, &listener, NULL);

    // Dispatch all pending events and wait for the registry to be fully populated
    wl_display_dispatch(disp);
    wl_display_roundtrip(disp);

    if (compositor == NULL) {
        fprintf(stderr, "Failed to bind compositor\n");
        wl_display_disconnect(disp);
        return -1;
    }

    surface = wl_compositor_create_surface(compositor);
    if (surface == NULL) {
        fprintf(stderr, "Failed to create surface\n");
        wl_display_disconnect(disp);
        return -1;
    }
    
    wl_display_roundtrip(disp);
    wl_surface_set_buffer_scale(surface, 20);
    int32_t fd = allocate_shared_memory(1920 * 1080 * 4);
    char *pixel = (char *)mmap(0, 1920 * 1080 * 4, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    struct wl_shm_pool *pool = wl_shm_create_pool(shm, fd, 1920 * 1080 * 4);
    frame_buff = wl_shm_pool_create_buffer(pool, 0, 1920, 1080, 1920 * 4, WL_SHM_FORMAT_ABGR8888);
    while(1)
    {
        colors a = random_color();
        for (int x = 0; x < 1920; x++)
        {
            for (int y = 0; y < 1080; y++)
            {
                int pxl_index = (x + 1920 * y) * 4;

                pixel[pxl_index] = a.b;
                pxl_index++;
                pixel[pxl_index] = a.g;
                pxl_index++;
                pixel[pxl_index] = a.r;
                pxl_index++;
                pixel[pxl_index] = 255;
                pxl_index++;
            }
        }
        wl_surface_attach(surface, frame_buff, 0, 0);
        wl_surface_commit(surface);
        wl_display_roundtrip(disp);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // Cleanup
    wl_surface_destroy(surface);
    wl_compositor_destroy(compositor);
    wl_display_roundtrip(disp);
    wl_display_disconnect(disp);

    return 0;
}