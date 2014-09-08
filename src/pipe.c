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

#include "main.h"
#include "debug.h"

typedef struct control_client_t {
} control_client_t;

typedef struct admin_client_t {
} admin_client_t;

typedef struct node_client_t {
} node_client_t;

static int create_admin_socket(char *path, int len) {
    int fd;

    fd = socket(AF_UNIX, SOCK_STREAM< 0);
    if(fd == -1)
        FATAL("cannot create admin socket: %s", strerror(errno));



}
