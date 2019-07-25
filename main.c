#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "event.h"
#include "list.h"


LIST_HEAD(client_head);


struct client {
    int fd;
    struct list_head node;
};


static int get_listening_socket(uint32_t ip, uint16_t port)
{
    int sock = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (sock == -1)
        return -1;

    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) == -1)
        goto clean_sock;

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr.s_addr = htonl(ip)
    };

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1)
        goto clean_sock;

    if (listen(sock, SOMAXCONN) == -1)
        goto clean_sock;

    return sock;

clean_sock:
    close(sock);
    return -1;
}


void evt_read(int fd, event_e events, void *ctx);
static int add_client(int clifd)
{
    struct client *cli = calloc(1, sizeof(struct client));
    if (!cli)
        return -1;

    cli->fd = clifd;
    list_add_tail(&(cli->node), &client_head);
    event_add_new(clifd, EV_READ, evt_read, NULL);

    return 0;
}


static int del_client(int clifd)
{
    struct client *p, *safe;
    list_for_each_entry_safe(p, safe, &client_head, node) {
        if (p->fd == clifd) {
            close(p->fd);
            list_del(&(p->node));
            free(p);
            event_del_new(clifd);
            return 0;
        }
    }

    return -1;
}


void evt_read(int fd, event_e events, void *ctx)
{
    ctx = ctx;

    char buf[256] = {0};
    ssize_t sread = read(fd, buf, 256);
    switch (sread) {
        case -1: perror("read"); break;
        case 0:
                 del_client(fd);
                 printf("Client %d removed\n", fd);
                 break;
        default:
                 buf[sread-1] = '\0';
                 printf("buf[%s]\n", buf);
    }
}


void evt_accept(int fd, event_e events, void *ctx)
{
    ctx = ctx;

    int clifd = accept(fd, NULL, NULL);
    if (clifd == -1) {
        printf("Accept failed\n");
        return;
    }

    add_client(clifd);
    printf("Client %d added\n", clifd);
}


int main()
{
    int sock = get_listening_socket(0x7f000001, 9999);
    if (sock == -1)
        return -1;

    // create event loop (only one event_loop instance available yet)
    event_loop_t *evt_loop = event_base_new();
    if (!evt_loop) {
        close(sock);
        return -1;
    }

    // add fd to event loop
    event_add_new(sock, EV_READ, evt_accept, NULL);
    event_add_new(0, EV_READ, evt_read, NULL);

    // launch event loop
    if (event_base_dispatch(evt_loop) == -1) {
        close(sock);
        event_base_del(evt_loop);
        return -1;
    }

    return 0;
}

