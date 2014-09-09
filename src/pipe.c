/*
 * Distributed pipe broker
 *
 * Copyright (C) 2014 Ron Pedde <ron@pedde.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#include "platform.h"

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <sys/queue.h>

#include <ev.h>

#include "main.h"
#include "debug.h"

#define ADMIN_BUFFER_MAX 8192

typedef struct control_client_t {
    TAILQ_ENTRY (control_client_t) entries;
} control_client_t;

typedef struct admin_client_t {
    ev_io io;
    int fd;
    TAILQ_ENTRY (admin_client_t) entries;
} admin_client_t;

typedef struct node_client_t {
    TAILQ_ENTRY (node_client_t) entries;
} node_client_t;

TAILQ_HEAD (, control_client_t) client_list;
TAILQ_HEAD (, admin_client_t) admin_list;
TAILQ_HEAD (, node_client_t) node_list;

static int set_non_block(int fd) {
    int flags;

    DEBUG("setting fd %d to non-blocking", fd);

    flags = fcntl(fd, F_GETFL);
    if(flags < 0) {
        WARN("error getting socket flags: %s", strerror(errno));
        return flags;
    }

    flags |= O_NONBLOCK;
    if(fcntl(fd, F_SETFL, flags) < 0) {
        WARN("error getting socket flags: %s", strerror(errno));
        return -1;
    }

    DEBUG("set non-blocking mode successfully");
    return 0;
}

/* FIXME: this should be line buffered! */
static void on_admin_read(struct ev_loop *loop,
                          struct ev_io *w,
                          int revents) {
    uint8_t buffer[ADMIN_BUFFER_MAX];
    int offset = 0;
    ssize_t read;

    admin_client_t *admin_client = (admin_client_t *)w;

    DEBUG("admin fd %d has become readable", admin_client->fd);
    memset(&buffer, 0, sizeof(buffer));

    while(offset < ADMIN_BUFFER_MAX) {
        read = recv(admin_client->fd, &buffer[offset], 1, 0);
        if(read < 0) /* EINTR? */
            WARN("unexpected error on admin fd %d", admin_client->fd);

        if(read == 0)
            WARN("unexpected disconnect on admin fd %d", admin_client->fd);

        if(read <= 0) {
            DEBUG("Closing admin fd %d", admin_client->fd);
            ev_io_stop(loop, &admin_client->io);
            TAILQ_REMOVE(&admin_list, admin_client, entries);
            shutdown(admin_client->fd, 2);
            close(admin_client->fd);
            free(admin_client);
            return;
        }

        /* soak up leading cr/lf */
        if(buffer[offset] == '\n' || buffer[offset] == '\r') {
            buffer[offset] = 0;
            if(offset)
                break;
        }


        offset++;
    }

    DEBUG("%d bytes in admin fd %d: (%s)", offset, admin_client->fd, buffer);

    if(!strcasecmp(buffer, "QUIT")) {
        DEBUG("exiting event loop");
        ev_unloop(loop, EVUNLOOP_ALL);
        return;
    }

    DEBUG("ignoring command");
}

static void on_admin_accept(struct ev_loop *loop,
                            struct ev_io *w,
                            int revents) {
    admin_client_t *accept_client = (admin_client_t *)w;
    int admin_client_fd;
    struct sockaddr_un admin_client_addr;
    socklen_t admin_client_len = sizeof(admin_client_addr);
    admin_client_t *admin_client;

    admin_client_fd = accept(accept_client->fd,
                             (struct sockaddr *)&admin_client_addr,
                             &admin_client_len);

    DEBUG("accepted admin connection on fd %d", admin_client_fd);

    if(admin_client_fd < 0) {
        WARN("admin client accept failed: %s", strerror(errno));
        return;
    }

    if(set_non_block(admin_client_fd) < 0) {
        WARN("cannot set non-block on admin client");
    }

    admin_client = (admin_client_t *)malloc(sizeof(admin_client_t));
    if(!admin_client)
        FATAL("malloc error");

    admin_client->fd = admin_client_fd;

    DEBUG("inserting client");
    TAILQ_INSERT_TAIL (&admin_list, admin_client, entries);

    DEBUG("starting recv event loop");

    ev_io_init(&admin_client->io, on_admin_read, admin_client->fd, EV_READ);
    ev_io_start(loop, &admin_client->io);

    DEBUG("accept processed");
}



static int create_admin_socket(char *path, int backlog) {
    int fd;
    struct sockaddr_un admin = { .sun_family = AF_UNIX };

    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(fd == -1)
        FATAL("cannot create admin socket: %s", strerror(errno));

    DEBUG("created AF_UNIX admin socket on %s as fd %d", path, fd);

    strcpy(admin.sun_path, path);

    int len = sizeof(sa_family_t) + strlen(admin.sun_path);
    if(admin.sun_path[0] == '@') {
        admin.sun_path[0] = 0;
    }


    if(bind(fd, (struct sockaddr *)&admin, len))
        FATAL("cannot bind admin socket: %s", strerror(errno));

    if(listen(fd, 5) == -1)
        FATAL("cannot listen on admin socket: %s", strerror(errno));

    return fd;
}

int pipe_run(void) {
    struct ev_loop *loop;
    admin_client_t admin_client;

    TAILQ_INIT(&admin_list);
    TAILQ_INIT(&client_list);
    TAILQ_INIT(&node_list);

    admin_client.fd = create_admin_socket(config.admin_socket,
                                          config.admin_backlog);

    if(set_non_block(admin_client.fd) == -1)
        FATAL("cannot set admin socket to non-block: %s", strerror(errno));

    loop = ev_default_loop(0);
    ev_io_init((ev_io *)&admin_client, on_admin_accept, admin_client.fd, EV_READ);
    ev_io_start(loop, (ev_io *)&admin_client);

    DEBUG("starting event loop");

    ev_loop(loop, 0);
    return 0;
}
