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

#include <eiCORE/ei_network.h>
#include <eiCORE/ei_verbose.h>

eiBool ei_net_startup()
{
#ifdef EI_OS_WINDOWS
	eiInt err;
	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD(1, 1);

	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0)
	{
		ei_error("Failed to startup socket.\n");
		return eiFALSE;
	}

	if (LOBYTE(wsaData.wVersion) != 1 || 
		HIBYTE(wsaData.wVersion) != 1)
	{
		ei_error("Incorrect socket version.\n");
		WSACleanup();
		return eiFALSE;
	}
#endif

	return eiTRUE;
}

eiBool ei_net_shutdown()
{
#ifdef EI_OS_WINDOWS
	eiInt err;

	err = WSACleanup();
	if (err != 0)
	{
		ei_error("Socket was not shutdown properly.\n");
		return eiFALSE;
	}
#endif

	return eiTRUE;
}

eiBool ei_net_get_host_by_name(struct hostent **remoteHost, const char *host_name)
{
	*remoteHost = gethostbyname(host_name);
    
    if (*remoteHost != NULL) {
		return eiTRUE;
	} else {
		ei_error("gethostbyname returns NULL\n");
		return eiFALSE;
	}
}

eiBool ei_net_get_host_by_addr(struct hostent **remoteHost, const char *host_name)
{
	eiInt err = 0;

#ifdef USE_IPV6
	IN6_ADDR addr6 = {0};

	eiInt iResult = inet_pton(AF_INET6, host_name, &addr6);
	if (iResult == 0) {
		return eiFALSE;
	} else {
		*remoteHost = gethostbyaddr((char *)&addr6, 16, AF_INET6);
	}
#else
	struct in_addr addr = {0};

	addr.s_addr = inet_addr(host_name);
	if (addr.s_addr == INADDR_NONE) {
		ei_error("inet_addr returns INADDR_NONE.\n");
		return eiFALSE;
	} else {
		*remoteHost = gethostbyaddr((char *)&addr, 4, AF_INET);
	}
#endif

	if (*remoteHost != NULL) {
		return eiTRUE;
	} else {
#ifdef EI_OS_WINDOWS
		err = WSAGetLastError();
#endif
		ei_error("gethostbyaddr returns NULL with error code %d.\n", err);
		return eiFALSE;
	}
}

eiBool ei_net_get_addr(SOCKADDR_IN *sockAddr, const char *hostName)
{
	struct hostent *remoteHost = NULL;

	if (hostName == NULL)
	{
		return eiFALSE;
	}

	if (isalpha(hostName[0]))
	{
		if (!ei_net_get_host_by_name(&remoteHost, hostName))
		{
			return eiFALSE;
		}
	}
	else
	{
		if (!ei_net_get_host_by_addr(&remoteHost, hostName))
		{
			/* we cannot get address in uniform way, so try to work around. */
			sockAddr->sin_addr.s_addr = inet_addr(hostName);
			if (sockAddr->sin_addr.s_addr == INADDR_NONE) {
				ei_error("inet_addr returns INADDR_NONE.\n");
				return eiFALSE;
			} else {
				return eiTRUE;
			}
		}
	}
	memcpy((eiByte *)&sockAddr->sin_addr.s_addr, remoteHost->h_addr_list[0], remoteHost->h_length);

	return eiTRUE;
}

char *ei_get_addr_string(SOCKADDR_IN *addr)
{
	if (addr != NULL)
	{
		return inet_ntoa(addr->sin_addr);
	}
	else
	{
		return NULL;
	}
}

eiBool ei_get_current_hostname(char *name, int len)
{
	return (gethostname(name, len) == 0);
}

eiBool ei_net_send(SOCKET sock, eiByte *data, eiInt size)
{
	eiInt remainingBytes = size;
	eiInt sentBytes;

	if (sock == INVALID_SOCKET)
	{
		/* ignore invalid socket. this behavior is required 
		   by the job manager to silently disable sending/receiving 
		   data from a socket temporarily by supplying a invalid 
		   socket. */
		return eiFALSE;
	}

	sentBytes = send(sock, (char *)data, remainingBytes, 0);

	if (sentBytes <= 0)
	{
		/* connection broken */
		return eiFALSE;
	}

	while (sentBytes < remainingBytes)
	{
		data += sentBytes;
		remainingBytes -= sentBytes;

		sentBytes = send(sock, (char *)data, remainingBytes, 0);

		if (sentBytes <= 0)
		{
			/* connection broken */
			return eiFALSE;
		}
	}

	return eiTRUE;
}

eiBool ei_net_recv(SOCKET sock, eiByte *data, eiInt size)
{
	eiInt remainingBytes = size;
	eiInt recvBytes;

	if (sock == INVALID_SOCKET)
	{
		/* ignore invalid socket. this behavior is required 
		   by the job manager to silently disable sending/receiving 
		   data from a socket temporarily by supplying a invalid 
		   socket. */
		return eiFALSE;
	}

	recvBytes = recv(sock, (char *)data, remainingBytes, 0);

	if (recvBytes <= 0)
	{
		/* connection broken */
		return eiFALSE;
	}

	while (recvBytes < remainingBytes)
	{
		data += recvBytes;
		remainingBytes -= recvBytes;

		recvBytes = recv(sock, (char *)data, remainingBytes, 0);

		if (recvBytes <= 0)
		{
			/* connection broken */
			return eiFALSE;
		}
	}

	return eiTRUE;
}

eiBool ei_net_init_server(SOCKET *srvSock, eiUshort port, eiInt maxNumClients)
{
	SOCKADDR_IN srvAddr;

	*srvSock = socket(PF_INET, SOCK_STREAM, 0);

	if (*srvSock == INVALID_SOCKET)
	{
		return eiFALSE;
	}

	ei_net_set_nodelay(srvSock);

	memset(&srvAddr, 0, sizeof(SOCKADDR_IN));
	srvAddr.sin_family = AF_INET;
	srvAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	srvAddr.sin_port = htons(port);

	if (bind(*srvSock, (SOCKADDR *)&srvAddr, sizeof(SOCKADDR)) < 0)
	{
		return eiFALSE;
	}

	if (listen(*srvSock, maxNumClients) < 0)
	{
		return eiFALSE;
	}

	return eiTRUE;
}

SOCKET ei_net_accept(SOCKET srvSock, SOCKADDR_IN *clientAddr)
{
	socklen_t len = sizeof(SOCKADDR);

	return accept(srvSock, (SOCKADDR *)clientAddr, &len);
}

SOCKET ei_net_time_accept(SOCKET srvSock, SOCKADDR_IN *clientAddr)
{
	socklen_t		len = sizeof(SOCKADDR);
	fd_set			fds;
	struct timeval	timeout;

	FD_ZERO(&fds);
	FD_SET(srvSock, &fds);
	timeout.tv_sec	= 5;
	timeout.tv_usec = 0;

	if (select(srvSock + 1, &fds, NULL, NULL, &timeout) <= 0)
	{
		ei_warning("Time out waiting for connection.\n");

		return INVALID_SOCKET;
	}

	return accept(srvSock, (SOCKADDR *)clientAddr, &len);
}

eiBool ei_net_init_client(SOCKET *clientSock, eiUshort port, const char *srvName)
{
	SOCKADDR_IN srvAddr;
	eiInt err = 0;

	*clientSock = socket(PF_INET, SOCK_STREAM, 0);

	if (*clientSock == INVALID_SOCKET)
	{
		ei_error("Failed to create client socket.\n");
		ei_net_shutdown();
		return eiFALSE;
	}

	ei_net_set_nodelay(clientSock);

	memset(&srvAddr, 0, sizeof(SOCKADDR_IN));
	srvAddr.sin_family = AF_INET;
	if (!ei_net_get_addr(&srvAddr, srvName))
	{
		ei_error("Failed to resolve server address.\n");
		ei_net_shutdown();
		return eiFALSE;
	}
	srvAddr.sin_port = htons(port);

	if (connect(*clientSock, (SOCKADDR *)&srvAddr, sizeof(SOCKADDR)) < 0)
	{
#ifdef EI_OS_WINDOWS
		err = WSAGetLastError();
#endif
		ei_error("Failed to connect to server with error code %d.\n", err);
		return eiFALSE;
	}

	return eiTRUE;
}

eiBool ei_net_close_socket(SOCKET *sock)
{
	if (*sock == INVALID_SOCKET)
	{
		return eiFALSE;
	}

	closesocket(*sock);

	return eiTRUE;
}

void ei_net_set_nodelay(SOCKET *sock)
{
	eiInt val;
	eiInt sock_buf_size;

	if (*sock == INVALID_SOCKET)
	{
		return;
	}
	
	val = 1;
	setsockopt(*sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&val, sizeof(eiInt));
	/* disable the Nagle (TCP no delay) algorithm */
	val = 1;
	setsockopt(*sock, IPPROTO_TCP, TCP_NODELAY, (const char *)&val, sizeof(eiInt));

	/* set to Bandwidth Delay Product (BDP), BDP = link_bandwidth * RTT, 
	   link_bandwidth is the rate at which packets can be transmitted on the network, 
	   round-trip time (RTT) is the delay between a segment being sent and its 
	   acknowledgement from the peer. */
	/* assume the network has 100 Mbps bandwidth with 50 ms RTT */
	sock_buf_size = 625 * 1024; /* 625 KB */
	setsockopt(*sock, SOL_SOCKET, SO_SNDBUF, (const char *)&sock_buf_size, sizeof(eiInt));
	setsockopt(*sock, SOL_SOCKET, SO_RCVBUF, (const char *)&sock_buf_size, sizeof(eiInt));
}

eiShort ei_net_hton_short(const eiShort v)
{
	return htons(v);
}

eiUshort ei_net_hton_ushort(const eiUshort v)
{
	return htons(v);
}

eiInt ei_net_hton_int(const eiInt v)
{
	return htonl(v);
}

eiUint ei_net_hton_uint(const eiUint v)
{
	return htonl(v);
}

eiShort ei_net_ntoh_short(const eiShort v)
{
	return ntohs(v);
}

eiUshort ei_net_ntoh_ushort(const eiUshort v)
{
	return ntohs(v);
}

eiInt ei_net_ntoh_int(const eiInt v)
{
	return ntohl(v);
}

eiUint ei_net_ntoh_uint(const eiUint v)
{
	return ntohl(v);
}
