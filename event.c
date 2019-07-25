#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>
#include <pthread.h>

#include "event.h"
#include "list.h"


typedef struct event_loop {
    pthread_t tid;
} event_loop_t;


typedef struct poll_field {
    nfds_t nfds;
    struct pollfd pfds[];
} pf_t;


typedef struct event {
    int fd;
    event_e events;
    void *ctx;
    event_cb_t event_cb;
    struct list_head node;
} event_t;


pf_t pf_g; // global pool array that contains every managed fds


LIST_HEAD(event_head); // list of callbacks for managed fds


static int __add_event(int fd, event_e events, event_cb_t event_cb, void *ctx)
{
    event_t *e = calloc(1, sizeof(event_t));
    if (!e)
        return -1;

    e->fd = fd;
    e->events = events;
    e->ctx = ctx;
    e->event_cb = event_cb;
    list_add_tail(&(e->node), &event_head);

    return 0;
}


static int __del_event(int fd)
{
    event_t *e, *safe;
    list_for_each_entry_safe(e, safe, &event_head, node) {
        if (fd == e->fd) {
            list_del(&(e->node));
            free(e);
            return 0;
        }
    }

    return -1;
}


static event_t *__get_event_from_poll(int fd)
{
    event_t *e;
    list_for_each_entry(e, &event_head, node)
        if (fd == e->fd)
            return e;

    return NULL;
}


static int __add_poll_field(int fd, short events)
{
    int i;
    for (i = 0; i <= pf_g.nfds; i++) {
        if (pf_g.pfds[i].fd == -1 || i == pf_g.nfds) {
            pf_g.pfds[i].fd = fd;
            pf_g.pfds[i].events = events;
            if (i == pf_g.nfds)
                pf_g.nfds += 1;
            return 0;
        }
    }

    return -1;
}


static int __del_poll_field(int fd)
{
    int i;
    for (i = 0; i < pf_g.nfds; i++) {
        if (pf_g.pfds[i].fd == fd) {
            pf_g.pfds[i].fd = -1;
            pf_g.pfds[i].events = -1;
            return 0;
        }
    }

    return -1;
}


static void event_loop_manager()
{
    int i;
    for (i = 0; i < pf_g.nfds; i++) {
        if (pf_g.pfds[i].revents & POLLIN) {
            event_t *e = __get_event_from_poll(pf_g.pfds[i].fd);
            if (e->event_cb)
                e->event_cb(e->fd, e->events, e->ctx);
        }
    }
}


static void *event_loop_thread(void *arg)
{
    for (;;) {
        switch (poll(pf_g.pfds, pf_g.nfds, -1)) {
            case -1: printf("poll failed\n"); return NULL;
            case 0: printf("no event ready\n"); return NULL;
            default: event_loop_manager(); continue;
        }
    }
}


static void __debug_print_pf()
{
    printf("[%ld]\n", pf_g.nfds);
    int index;
    for (index = 0; index < pf_g.nfds; index++) {
        printf("%d, fd[%d], nfds[%ld]\n", index, pf_g.pfds[index].fd, pf_g.nfds);
    }
}


event_loop_t *event_base_new()
{
    event_loop_t *evt_loop = calloc(1, sizeof(event_loop_t));
    if (!evt_loop)
        return NULL;
    return evt_loop;
}


void event_base_del(event_loop_t *evt_loop)
{
    if (!evt_loop)
        return;

    // should clean also list of events

    free(evt_loop);
}


int event_base_dispatch(event_loop_t *event_loop)
{
    if (!event_loop)
        return -1;

    if (pthread_create(&(event_loop->tid), NULL, event_loop_thread, NULL)) {
        printf("Error in new thread creation\n");
        return -1;
    }

    pthread_join(event_loop->tid, NULL);

    return 0;
}


int event_add_new(int fd, event_e events, event_cb_t event_cb, void *ctx)
{
    if (__add_poll_field(fd, events) == -1)
        return -1;
    if (__add_event(fd, events, event_cb, ctx) == -1)
        return -1;
    return 0;
}


int event_del_new(int fd)
{
    if (__del_poll_field(fd) == -1)
        return -1;
    if (__del_event(fd) == -1)
        return -1;
    return 0;
}

