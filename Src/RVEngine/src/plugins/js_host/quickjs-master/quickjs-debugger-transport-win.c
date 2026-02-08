/*
 * QuickJS debugger transport - Windows (Winsock2)
 *
 * Adapted from koush/quickjs debugger fork for quickjs-ng.
 * Provides both js_debugger_connect() (outbound) and
 * js_debugger_wait_connection() (listen/accept) over TCP.
 */

#include "quickjs-debugger.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

struct js_transport_data {
    SOCKET handle;
};

static int js_transport_wsa_initialized = 0;

static void js_transport_ensure_wsa(void) {
    if (!js_transport_wsa_initialized) {
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
        js_transport_wsa_initialized = 1;
    }
}

static size_t js_transport_read(void *udata, char *buffer, size_t length) {
    struct js_transport_data* data = (struct js_transport_data *)udata;
    if (data->handle == INVALID_SOCKET)
        return (size_t)-1;

    if (length == 0)
        return (size_t)-2;

    if (buffer == NULL)
        return (size_t)-3;

    int ret = recv(data->handle, buffer, (int)length, 0);

    if (ret == SOCKET_ERROR)
        return (size_t)-4;

    if (ret == 0)
        return (size_t)-5;

    if ((size_t)ret > length)
        return (size_t)-6;

    return (size_t)ret;
}

static size_t js_transport_write(void *udata, const char *buffer, size_t length) {
    struct js_transport_data* data = (struct js_transport_data *)udata;
    if (data->handle == INVALID_SOCKET)
        return (size_t)-1;

    if (length == 0)
        return (size_t)-2;

    if (buffer == NULL)
        return (size_t)-3;

    int ret = send(data->handle, buffer, (int)length, 0);
    if (ret <= 0 || (size_t)ret > length)
        return (size_t)-4;

    return (size_t)ret;
}

static size_t js_transport_peek(void *udata) {
    WSAPOLLFD fds[1];
    int poll_rc;

    struct js_transport_data* data = (struct js_transport_data *)udata;
    if (data->handle == INVALID_SOCKET)
        return (size_t)-1;

    fds[0].fd = data->handle;
    fds[0].events = POLLIN;
    fds[0].revents = 0;

    poll_rc = WSAPoll(fds, 1, 0);
    if (poll_rc < 0)
        return (size_t)-2;
    if (poll_rc > 1)
        return (size_t)-3;
    /* no data */
    if (poll_rc == 0)
        return 0;
    /* has data */
    return 1;
}

static void js_transport_close(JSRuntime* rt, void *udata) {
    struct js_transport_data* data = (struct js_transport_data *)udata;
    (void)rt;

    if (data->handle != INVALID_SOCKET) {
        closesocket(data->handle);
        data->handle = INVALID_SOCKET;
    }

    free(udata);
}

/* Parse "host:port" address string into sockaddr_in */
static struct sockaddr_in js_debugger_parse_sockaddr(const char* address) {
    char* port_string = strstr(address, ":");
    assert(port_string);

    int port = atoi(port_string + 1);
    assert(port);

    char host_string[256];
    strncpy(host_string, address, sizeof(host_string) - 1);
    host_string[sizeof(host_string) - 1] = 0;
    host_string[port_string - address] = 0;

    struct hostent *host = gethostbyname(host_string);
    assert(host);

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    memcpy((char *)&addr.sin_addr.s_addr, (char *)host->h_addr, host->h_length);
    addr.sin_port = htons((u_short)port);

    return addr;
}

/* Connect to a remote debugger (DAP adapter) */
void js_debugger_connect(JSContext *ctx, const char *address) {
    js_transport_ensure_wsa();

    struct sockaddr_in addr = js_debugger_parse_sockaddr(address);

    SOCKET client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    assert(client != INVALID_SOCKET);

    int rc = connect(client, (const struct sockaddr *)&addr, sizeof(addr));
    assert(rc == 0);

    struct js_transport_data *data = (struct js_transport_data *)malloc(sizeof(struct js_transport_data));
    memset(data, 0, sizeof(struct js_transport_data));
    data->handle = client;
    js_debugger_attach(ctx, js_transport_read, js_transport_write, js_transport_peek, js_transport_close, data);
}

/* Listen on address and wait for a debugger to connect (blocking) */
void js_debugger_wait_connection(JSContext *ctx, const char* address) {
    js_transport_ensure_wsa();

    struct sockaddr_in addr = js_debugger_parse_sockaddr(address);

    SOCKET server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    assert(server != INVALID_SOCKET);

    int reuseAddress = 1;
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuseAddress, sizeof(reuseAddress));

    int rc = bind(server, (struct sockaddr *)&addr, sizeof(addr));
    assert(rc == 0);

    listen(server, 1);

    struct sockaddr_in client_addr;
    int client_addr_size = (int)sizeof(client_addr);
    SOCKET client = accept(server, (struct sockaddr *)&client_addr, &client_addr_size);
    closesocket(server);
    assert(client != INVALID_SOCKET);

    struct js_transport_data *data = (struct js_transport_data *)malloc(sizeof(struct js_transport_data));
    memset(data, 0, sizeof(struct js_transport_data));
    data->handle = client;
    js_debugger_attach(ctx, js_transport_read, js_transport_write, js_transport_peek, js_transport_close, data);
}
