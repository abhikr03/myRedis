#include "common.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

void die(const char* msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

std::string to_lower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c){ return std::tolower(c); });
    return result;
}

int recv_exactly(int fd, char* buf, size_t n_bytes) {
    while (n_bytes > 0) {
        int bytesRead = recv(fd, buf, n_bytes, 0);
        if (bytesRead <= 0) {
            perror("recv");
            return -1;
        }
        assert((size_t)bytesRead <= n_bytes);
        n_bytes -= (size_t)bytesRead;
        buf += (size_t)bytesRead;
    }
    return 0;
}

int write_exactly(int fd, const char* buf, size_t n_bytes) {
    while (n_bytes > 0) {
        ssize_t bytesSent = send(fd, buf, n_bytes, 0);
        if (bytesSent <= 0) {
            perror("send");
            return -1;
        }
        assert((size_t)bytesSent <= n_bytes);
        n_bytes -= (size_t)bytesSent;
        buf += (size_t)bytesSent;
    }
    return 0;
}