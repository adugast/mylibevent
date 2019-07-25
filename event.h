#ifndef __EVENT_H__
#define __EVENT_H__


typedef struct event_loop event_loop_t;


typedef enum {
    EV_READ = 1,
} event_e;
// EV_WRITE,


typedef void (*event_cb_t)(int fd, event_e events, void *ctx);


event_loop_t *event_base_new();
void event_base_del(event_loop_t *evt_loop);

int event_add_new(int fd, event_e events, event_cb_t cb, void *ctx);
int event_del_new(int fd);

int event_base_dispatch(event_loop_t *event_loop);


#endif /*  __EVENT_H__ */

