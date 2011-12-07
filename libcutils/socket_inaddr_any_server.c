/* libs/cutils/socket_inaddr_any_server.c
**
** Copyright 2006, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
*/

#include <cutils/sockets.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stddef.h>

#ifndef HAVE_WINSOCK
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <netinet/in.h>
#endif

#define LISTEN_BACKLOG 4

/* open listen() port on any interface */
int socket_inaddr_any_server(int port, int type)
{
    union {
        struct sockaddr_in in;
        struct sockaddr generic;
    } addr;
    size_t alen;
    int s, n;

    memset(&addr, 0, sizeof(addr));
    addr.in.sin_family = AF_INET;
    addr.in.sin_port = htons(port);
    addr.in.sin_addr.s_addr = htonl(INADDR_ANY);

    s = socket(AF_INET, type, 0);
    if(s < 0) return -1;

    n = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &n, sizeof(n));

    if(bind(s, &addr.generic, sizeof(addr.in)) < 0) {
        close(s);
        return -1;
    }

    if (type == SOCK_STREAM) {
        int ret;

        ret = listen(s, LISTEN_BACKLOG);

        if (ret < 0) {
            close(s);
            return -1; 
        }
    }

    return s;
}
