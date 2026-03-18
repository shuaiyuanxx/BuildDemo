// TestApp.cpp - Multi-Container Demo
//
// Demonstrates two containers running simultaneously:
//   - echo-server (port 9000): echoes messages back unchanged
//   - uppercase-worker (port 9001): returns messages in UPPERCASE
//
// The Windows client sends "Hello World!" to both and prints responses.
// Ctrl+C to stop and clean up.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

#define SERVER_PORT 9000
#define WORKER_PORT 9001
#define SEND_INTERVAL_MS 2000
#define BUF_SIZE 1024

struct ContainerInfo
{
    const char* name;
    const char* image;
    int port;
};

static ContainerInfo g_containers[] = {
    {"echo-server",       "echo-server:latest",       SERVER_PORT},
    {"uppercase-worker",  "uppercase-worker:latest",  WORKER_PORT},
};

static const int g_containerCount = sizeof(g_containers) / sizeof(g_containers[0]);

static volatile bool g_running = true;
static bool g_cleanedUp = false;

static void DockerCleanup()
{
    if (g_cleanedUp) return;
    g_cleanedUp = true;

    for (int i = 0; i < g_containerCount; i++)
    {
        printf("Stopping %s...\n", g_containers[i].name);
        char cmd[256];
        sprintf_s(cmd, "docker rm -f %s >nul 2>&1", g_containers[i].name);
        system(cmd);
    }
}

static BOOL WINAPI ConsoleHandler(DWORD ctrlType)
{
    switch (ctrlType)
    {
    case CTRL_C_EVENT:
    case CTRL_BREAK_EVENT:
        printf("\nShutting down...\n");
        g_running = false;
        return TRUE;
    case CTRL_CLOSE_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:
        DockerCleanup();
        return TRUE;
    }
    return FALSE;
}

static bool DockerRun(const ContainerInfo& c)
{
    char cmd[512];
    sprintf_s(cmd, "docker rm -f %s >nul 2>&1", c.name);
    system(cmd);

    sprintf_s(cmd, "docker run -d --name %s -p %d:%d %s", c.name, c.port, c.port, c.image);
    printf("  %s: %s\n", c.name, cmd);
    return system(cmd) == 0;
}

static SOCKET ConnectTo(int port)
{
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) return INVALID_SOCKET;

    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons((u_short)port);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    if (connect(sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
    {
        closesocket(sock);
        return INVALID_SOCKET;
    }

    DWORD timeout = SEND_INTERVAL_MS;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
    return sock;
}

int wmain()
{
    SetConsoleCtrlHandler(ConsoleHandler, TRUE);
    atexit(DockerCleanup);

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    printf("========================================\n");
    printf("  Multi-Container Demo\n");
    printf("  Server (echo)     -> port %d\n", SERVER_PORT);
    printf("  Worker (uppercase) -> port %d\n", WORKER_PORT);
    printf("========================================\n\n");

    // --- Start all containers ---
    printf("[1] Starting containers...\n");
    for (int i = 0; i < g_containerCount; i++)
    {
        if (!DockerRun(g_containers[i]))
        {
            printf("FATAL: Failed to start %s\n", g_containers[i].name);
            DockerCleanup();
            WSACleanup();
            return 1;
        }
    }
    printf("All containers started.\n\n");

    // --- Wait for servers to be ready ---
    printf("[2] Waiting for services...\n");
    Sleep(3000);

    // --- Connect to both ---
    printf("[3] Connecting...\n");

    SOCKET sockets[2] = {INVALID_SOCKET, INVALID_SOCKET};
    for (int i = 0; i < g_containerCount; i++)
    {
        for (int attempt = 0; attempt < 5 && g_running; attempt++)
        {
            sockets[i] = ConnectTo(g_containers[i].port);
            if (sockets[i] != INVALID_SOCKET)
            {
                printf("  Connected to %s (port %d)\n", g_containers[i].name, g_containers[i].port);
                break;
            }
            printf("  Retrying %s...\n", g_containers[i].name);
            Sleep(1000);
        }

        if (sockets[i] == INVALID_SOCKET)
        {
            printf("FATAL: Cannot connect to %s\n", g_containers[i].name);
            DockerCleanup();
            WSACleanup();
            return 1;
        }
    }

    // --- Send/Receive loop ---
    printf("\n[4] Sending messages (Ctrl+C to stop)\n\n");

    int count = 0;
    char buf[BUF_SIZE];

    while (g_running)
    {
        count++;
        const char* message = "Hello World!\n";
        int msgLen = (int)strlen(message);

        for (int i = 0; i < g_containerCount; i++)
        {
            int sent = send(sockets[i], message, msgLen, 0);
            if (sent == SOCKET_ERROR)
            {
                printf("[%s] Send failed, reconnecting...\n", g_containers[i].name);
                closesocket(sockets[i]);
                sockets[i] = ConnectTo(g_containers[i].port);
                continue;
            }

            int received = recv(sockets[i], buf, sizeof(buf) - 1, 0);
            if (received > 0)
            {
                buf[received] = '\0';
                printf("[#%d %s] Sent: %.*s -> Recv: %s",
                    count, g_containers[i].name,
                    msgLen - 1, message,  // trim \n from sent
                    buf);
            }
            else if (received == 0)
            {
                printf("[%s] Disconnected, reconnecting...\n", g_containers[i].name);
                closesocket(sockets[i]);
                sockets[i] = ConnectTo(g_containers[i].port);
            }
        }

        printf("\n");
        Sleep(SEND_INTERVAL_MS);
    }

    // --- Cleanup ---
    printf("\n[5] Cleaning up...\n");
    for (int i = 0; i < g_containerCount; i++)
    {
        if (sockets[i] != INVALID_SOCKET)
            closesocket(sockets[i]);
    }
    WSACleanup();
    DockerCleanup();

    printf("Done.\n");
    return 0;
}
