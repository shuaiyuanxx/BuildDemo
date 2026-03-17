// TestWSLCSDK.cpp - Echo Client Demo
//
// Phase 1 (current): Uses Docker CLI to manage container
// Phase 2 (future):  Switch USE_WSLC_SDK to 1 to use WSLC SDK
//
// Flow:
//   1. Start echo server container (docker run / wslc SDK)
//   2. Connect via TCP socket to 127.0.0.1:9000
//   3. Send "Hello World!" every 2 seconds, print echo response
//   4. Ctrl+C to stop and clean up

#define USE_WSLC_SDK 0  // Set to 1 when WSLC SDK environment is ready

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if USE_WSLC_SDK
#include "wslcsdk.h"
#endif

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <objbase.h>

#pragma comment(lib, "ws2_32.lib")

#define SERVER_PORT 9000
#define SEND_INTERVAL_MS 2000
#define BUF_SIZE 1024

static const char* IMAGE_NAME = "testwslcsdk:latest";
static const char* CONTAINER_NAME = "echo-server";

static volatile bool g_running = true;

static void CleanupAll();

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
        // Window closing / logoff / shutdown - cleanup immediately
        CleanupAll();
        return TRUE;
    }
    return FALSE;
}

// ============================================================================
// Docker CLI backend
// ============================================================================
#if !USE_WSLC_SDK

static bool DockerRun()
{
    // Remove any existing container with the same name
    char cmd[512];
    sprintf_s(cmd, "docker rm -f %s >nul 2>&1", CONTAINER_NAME);
    system(cmd);

    // Run container: detached, port mapped, auto-remove on stop
    sprintf_s(cmd, "docker run -d --name %s -p %d:%d %s",
        CONTAINER_NAME, SERVER_PORT, SERVER_PORT, IMAGE_NAME);

    printf("Running: %s\n", cmd);
    int rc = system(cmd);
    return rc == 0;
}

static void DockerLogs()
{
    char cmd[256];
    sprintf_s(cmd, "docker logs %s", CONTAINER_NAME);
    system(cmd);
}

static void DockerCleanup()
{
    printf("Stopping container...\n");
    char cmd[256];
    sprintf_s(cmd, "docker rm -f %s >nul 2>&1", CONTAINER_NAME);
    system(cmd);
}

#endif

// ============================================================================
// WSLC SDK backend
// ============================================================================
#if USE_WSLC_SDK

static WslcSession g_session = nullptr;
static WslcContainer g_container = nullptr;

static void CALLBACK StdoutCallback(const BYTE* data, uint32_t dataSize, PVOID context)
{
    printf("[CONTAINER] ");
    fwrite(data, 1, dataSize, stdout);
}

static bool WslcRun()
{
    HRESULT hr;
    PWSTR errorMsg = nullptr;

    // Create session
    WslcSessionSettings sessionSettings;
    hr = WslcInitSessionSettings(L"EchoDemo", L"C:\\temp\\wslc-echo-demo", &sessionSettings);
    if (FAILED(hr)) { printf("WslcInitSessionSettings failed: 0x%08x\n", hr); return false; }

    WslcSetSessionSettingsCpuCount(&sessionSettings, 2);
    WslcSetSessionSettingsMemory(&sessionSettings, 1024);

    hr = WslcCreateSession(&sessionSettings, &g_session, &errorMsg);
    if (FAILED(hr))
    {
        printf("WslcCreateSession failed: 0x%08x %ls\n", hr, errorMsg ? errorMsg : L"");
        CoTaskMemFree(errorMsg);
        return false;
    }

    // Create container
    WslcContainerSettings containerSettings;
    WslcContainerInitSettings(IMAGE_NAME, &containerSettings);
    WslcContainerSettingsSetName(&containerSettings, CONTAINER_NAME);
    WslcContainerSettingsSetNetworkingMode(&containerSettings, WSLC_CONTAINER_NETWORKING_MODE_BRIDGED);

    WslcContainerPortMapping port = {};
    port.windowsPort = SERVER_PORT;
    port.containerPort = SERVER_PORT;
    port.protocol = WSLC_PORT_PROTOCOL_TCP;
    WslcContainerSettingsSetPortMapping(&containerSettings, &port, 1);

    WslcProcessSettings initProc;
    WslcProcessInitSettings(&initProc);
    PCSTR argv[] = { "/app/server" };
    WslcProcessSettingsSetCmdLineArgs(&initProc, argv, 1);
    WslcProcessSettingsSetIoCallback(&initProc, WSLC_PROCESS_IO_HANDLE_STDOUT, StdoutCallback, nullptr);
    WslcProcessSettingsSetIoCallback(&initProc, WSLC_PROCESS_IO_HANDLE_STDERR, StdoutCallback, nullptr);
    WslcContainerSettingsSetInitProcess(&containerSettings, &initProc);

    hr = WslcContainerCreate(g_session, &containerSettings, &g_container, &errorMsg);
    if (FAILED(hr))
    {
        printf("WslcContainerCreate failed: 0x%08x %ls\n", hr, errorMsg ? errorMsg : L"");
        CoTaskMemFree(errorMsg);
        return false;
    }

    hr = WslcContainerStart(g_container, WSLC_CONTAINER_START_FLAG_NONE);
    if (FAILED(hr)) { printf("WslcContainerStart failed: 0x%08x\n", hr); return false; }

    return true;
}

static void WslcCleanup()
{
    if (g_container)
    {
        WslcContainerStop(g_container, WSLC_SIGNAL_SIGTERM, 5);
        WslcContainerDelete(g_container, WSLC_DELETE_CONTAINER_FLAG_FORCE);
        WslcContainerRelease(g_container);
        g_container = nullptr;
    }
    if (g_session)
    {
        WslcTerminateSession(g_session);
        WslcReleaseSession(g_session);
        g_session = nullptr;
    }
}

#endif

// ============================================================================
// TCP Client - shared by both backends
// ============================================================================
static int RunClient()
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET sock = INVALID_SOCKET;
    int sendCount = 0;

    while (g_running)
    {
        // Connect if not connected
        if (sock == INVALID_SOCKET)
        {
            sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (sock == INVALID_SOCKET)
            {
                printf("Failed to create socket: %d\n", WSAGetLastError());
                Sleep(SEND_INTERVAL_MS);
                continue;
            }

            sockaddr_in serverAddr = {};
            serverAddr.sin_family = AF_INET;
            serverAddr.sin_port = htons(SERVER_PORT);
            inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

            if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
            {
                printf("Connect failed: %d, retrying...\n", WSAGetLastError());
                closesocket(sock);
                sock = INVALID_SOCKET;
                Sleep(SEND_INTERVAL_MS);
                continue;
            }

            printf("Connected to echo server!\n\n");

            DWORD timeout = SEND_INTERVAL_MS;
            setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
        }

        // Send
        const char* message = "Hello World!\n";
        int msgLen = (int)strlen(message);
        int sent = send(sock, message, msgLen, 0);
        if (sent == SOCKET_ERROR)
        {
            printf("Send failed: %d, reconnecting...\n", WSAGetLastError());
            closesocket(sock);
            sock = INVALID_SOCKET;
            continue;
        }

        sendCount++;
        printf("[SEND #%d] %s", sendCount, message);

        // Receive echo
        char buf[BUF_SIZE] = {};
        int received = recv(sock, buf, sizeof(buf) - 1, 0);
        if (received > 0)
        {
            buf[received] = '\0';
            printf("[RECV #%d] %s", sendCount, buf);
        }
        else if (received == 0)
        {
            printf("Server closed connection, reconnecting...\n");
            closesocket(sock);
            sock = INVALID_SOCKET;
            continue;
        }

        printf("\n");
        Sleep(SEND_INTERVAL_MS);
    }

    if (sock != INVALID_SOCKET)
        closesocket(sock);
    WSACleanup();
    return 0;
}

// ============================================================================
// Unified cleanup - called from all exit paths
// ============================================================================
static bool g_cleanedUp = false;

static void CleanupAll()
{
    if (g_cleanedUp) return;
    g_cleanedUp = true;

#if USE_WSLC_SDK
    WslcCleanup();
#else
    DockerCleanup();
#endif
}

// ============================================================================
// Main
// ============================================================================
int wmain()
{
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    SetConsoleCtrlHandler(ConsoleHandler, TRUE);
    atexit(CleanupAll);

    printf("========================================\n");
#if USE_WSLC_SDK
    printf("  Echo Client Demo (WSLC SDK backend)\n");
#else
    printf("  Echo Client Demo (Docker backend)\n");
#endif
    printf("========================================\n\n");

    // --- Start container ---
    printf("[1] Starting echo server container...\n");
    bool ok;

#if USE_WSLC_SDK
    ok = WslcRun();
#else
    ok = DockerRun();
#endif

    if (!ok)
    {
        printf("FATAL: Failed to start container.\n");
        CoUninitialize();
        return 1;
    }

    printf("Container started.\n\n");

    // --- Wait for server to be ready ---
    printf("[2] Waiting for server to start...\n");
    Sleep(2000);

#if !USE_WSLC_SDK
    printf("\nServer logs:\n");
    DockerLogs();
    printf("\n");
#endif

    // --- Run client loop ---
    printf("[3] Sending messages to 127.0.0.1:%d (Ctrl+C to stop)\n\n", SERVER_PORT);
    RunClient();

    // --- Cleanup ---
    printf("\n[4] Cleaning up...\n");
    CleanupAll();
    CoUninitialize();
    printf("Done.\n");
    return 0;
}
