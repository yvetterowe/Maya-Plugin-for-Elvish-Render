/*
 * Copyright 2010 elvish render Team
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at

 * http://www.apache.org/licenses/LICENSE-2.0

 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef EI_NETWORK_H
#define EI_NETWORK_H

/** \brief Network API wrappers.
 * \file ei_network.h
 * \author Elvic Liang
 */

#include <eiCORE/ei_core.h>
#include <stdio.h>

#ifdef EI_OS_WINDOWS

	#include <winsock2.h>
    #include <ws2tcpip.h>
    #include <malloc.h>
    #include <process.h>

    #define	socklen_t int

#else

    #include <unistd.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <netinet/tcp.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <ctype.h>
    #include <signal.h>
    #include <errno.h>

    #define SOCKET int
    #define SOCKADDR struct sockaddr
    #define SOCKADDR_IN struct sockaddr_in
    #define	closesocket close
    #define	INVALID_SOCKET -1

#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Use Ipv4 by default, since the minimum supported platform
   for Ipv6 is Windows Vista. */
/* #define USE_IPV6 */

eiCORE_API eiBool ei_net_startup();
eiCORE_API eiBool ei_net_shutdown();
eiCORE_API eiBool ei_net_get_host_by_name(struct hostent **remoteHost, const char *host_name);
eiCORE_API eiBool ei_net_get_host_by_addr(struct hostent **remoteHost, const char *host_name);
eiCORE_API eiBool ei_net_get_addr(SOCKADDR_IN *sockAddr, const char *hostName);
eiCORE_API char *ei_get_addr_string(SOCKADDR_IN *addr);
eiCORE_API eiBool ei_get_current_hostname(char *name, int len);
eiCORE_API eiBool ei_net_send(SOCKET sock, eiByte *data, eiInt size);
eiCORE_API eiBool ei_net_recv(SOCKET sock, eiByte *data, eiInt size);
eiCORE_API eiBool ei_net_init_server(SOCKET *srvSock, eiUshort port, eiInt maxNumClients);
eiCORE_API SOCKET ei_net_accept(SOCKET srvSock, SOCKADDR_IN *clientAddr);
eiCORE_API SOCKET ei_net_time_accept(SOCKET srvSock, SOCKADDR_IN *clientAddr);
eiCORE_API eiBool ei_net_init_client(SOCKET *clientSock, eiUshort port, const char *srvName);
eiCORE_API eiBool ei_net_close_socket(SOCKET *sock);
eiCORE_API void ei_net_set_nodelay(SOCKET *sock);
eiCORE_API eiShort ei_net_hton_short(const eiShort v);
eiCORE_API eiUshort ei_net_hton_ushort(const eiUshort v);
eiCORE_API eiInt ei_net_hton_int(const eiInt v);
eiCORE_API eiUint ei_net_hton_uint(const eiUint v);
eiCORE_API eiShort ei_net_ntoh_short(const eiShort v);
eiCORE_API eiUshort ei_net_ntoh_ushort(const eiUshort v);
eiCORE_API eiInt ei_net_ntoh_int(const eiInt v);
eiCORE_API eiUint ei_net_ntoh_uint(const eiUint v);

#ifdef __cplusplus
}
#endif

#endif
