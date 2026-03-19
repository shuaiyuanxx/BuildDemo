#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 9000
#define BUF_SIZE 1024

int main()
{
    int serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd < 0) { perror("socket"); return 1; }

    int opt = 1;
    setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(serverFd, (sockaddr*)&addr, sizeof(addr)) < 0) { perror("bind"); close(serverFd); return 1; }
    if (listen(serverFd, 5) < 0) { perror("listen"); close(serverFd); return 1; }

    printf("[SERVER] Echo server listening on port %d...\n", PORT);
    fflush(stdout);

    while (true)
    {
        sockaddr_in clientAddr{};
        socklen_t clientLen = sizeof(clientAddr);
        int clientFd = accept(serverFd, (sockaddr*)&clientAddr, &clientLen);
        if (clientFd < 0) { perror("accept"); continue; }

        printf("[SERVER] Client connected: %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
        fflush(stdout);

        char buf[BUF_SIZE];
        while (true)
        {
            int n = recv(clientFd, buf, sizeof(buf) - 1, 0);
            if (n <= 0) break;
            buf[n] = '\0';
            printf("[SERVER] Echo: %s", buf);
            fflush(stdout);
            send(clientFd, buf, n, 0);
        }

        printf("[SERVER] Client disconnected--.\n");
        fflush(stdout);
        close(clientFd);
    }

    close(serverFd);
    return 0;
}

