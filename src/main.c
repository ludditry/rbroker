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
#include <stdlib.h>
#include <getopt.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

#include <libdaemon/dfork.h>
#include <libdaemon/dpid.h>
#include <libdaemon/dexec.h>
#include <libdaemon/dlog.h>
#include <libdaemon/dexec.h>

#include "main.h"
#include "debug.h"

#define DEFAULT_CONFIG_FILE "/etc/rbroker.conf"

/* Config options */
config_t config = {
    .debug_level = 0,
    .daemonize = 1,
    .admin_backlog = 2,
    .admin_socket = "@rpipes_admin",
    .client_backlog = 5,
    .node_backlog = 2
};

cfg_opt_t opts[] = {
    CFG_SIMPLE_BOOL("debug_level", &config.debug_level),
    CFG_SIMPLE_BOOL("daemonize", &config.daemonize),
    CFG_END()
};

void usage(char *a0) {
    fprintf(stderr, "Usage: %s [options]\n\n", a0);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, " -d <level>    set debug level (1-5)\n");
    fprintf(stderr, " -f            run in foreground\n");
    fprintf(stderr, " -c <config>   use specified config file\n");
    fprintf(stderr, "\n");
}

int main(int argc, char *argv[]) {
    char *config_file = DEFAULT_CONFIG_FILE;
    int debug_level=-1;
    int daemonize = 1;
    int option;
    pid_t pid;
    int res;

    daemon_pid_file_ident = daemon_ident_from_argv0(argv[0]);

    fprintf(stderr, "%s version %s\n", PACKAGE_NAME, VERSION);
    fprintf(stderr, "Released under GPLv3+ license, see LICENSE for details\n");

    debug_set_level(2);

    /* command line opts (mostly for config file) */
    while((option = getopt(argc, argv, "d:c:fk")) != -1) {
        switch(option) {
        case 'd':
            debug_level = atoi(optarg);
            break;

        case 'c':
            config_file = optarg;
            break;

        case 'f':
            daemonize = 0;
            break;

        case 'k':
#ifdef DAEMON_PID_FILE_KILL_WAIT_AVAILABLE
            if((res = daemon_pid_file_kill_wait(SIGINT, 5)) < 0)
#else
            if((res = daemon_pid_file_kill(SIGINT)) < 0)
#endif
                WARN("Failed to kill daemon");
            exit(res < 0 ? EXIT_FAILURE : EXIT_SUCCESS);

        default:
            fprintf(stderr, "Unrecognized option '%c'\n", option);
            usage(argv[0]);
            exit(EXIT_FAILURE);
            break;
        }
    }

    /* read the config file, then daemonize */
    cfg_t *cfg = cfg_init(opts, 0);
    int err;

    err = cfg_parse(cfg, config_file);
    if (err == CFG_FILE_ERROR) {
        ERROR("cannot open config file (%s)", config_file);
        exit(EXIT_FAILURE);
    } else if (err == CFG_PARSE_ERROR) {
        exit(EXIT_FAILURE);
    }

    /* merge command line options... */
    if(debug_level != -1)
        config.debug_level = debug_level;

    if(!daemonize)
        config.daemonize = 0;

    debug_set_level(config.debug_level);

    res = 0;

    /* config is loaded, do daemonization (if necessary) */
    if(config.daemonize) {
        /* make sure daemon isn't already running */
        if((pid = daemon_pid_file_is_running()) >= 0) {
            ERROR("daemon already running on pid file %u", pid);
            exit(EXIT_FAILURE);
        }

        daemon_retval_init();

        if((pid = daemon_fork()) < 0) {
            daemon_retval_done();
            exit(EXIT_FAILURE);
        } else if(pid) { /* parent */
            if ((res = daemon_retval_wait(5)) < 0) {
                ERROR("daemon did not start");
                exit(EXIT_FAILURE);
            }

            DEBUG("daemon start: %i", res);
        } else { /* daemon */
            /* FIXME: switch over to syslog logging */
            res = 0;
            if(daemon_pid_file_create() < 0) {
                ERROR("Could not create pidfile (%s)", strerror(errno));
                res = 1;
                daemon_retval_send(1);
            } else {
                daemon_retval_send(0);
            }
        }
    }

    /* did the daemon start up successfully? */
    if(!res) {
        daemon_pid_file_remove();
    }

    /* do the actual daemon thing here */
    DEBUG("starting processing loop");


    cfg_free(cfg);
    exit(res);
}
