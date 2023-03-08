#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <vector>
#include <algorithm>
#include <mutex>
#include <atomic>
#include <future>
#include <sstream>
#include <stdexcept>
#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

constexpr int MAX_THREADS = 256;
constexpr int TIMEOUT_MS = 100;

std::mutex mutex;

std::atomic_int num_open_ports(0);

std::vector<int> open_ports;

// Scans a single port and adds it to the list of open ports if it's open
void scan_port(const std::string& ip_addr, int port)
{
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    InetPton(AF_INET, ip_addr.c_str(), &server.sin_addr);

    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        return;
    }

    u_long iMode = 1;
    ioctlsocket(sock, FIONBIO, &iMode);

    int result = connect(sock, reinterpret_cast<sockaddr*>(&server), sizeof(server));
    if (result == SOCKET_ERROR) {
        int error = WSAGetLastError();
        if (error != WSAEWOULDBLOCK && error != WSAECONNREFUSED) {
            closesocket(sock);
            return;
        }
        fd_set set;
        FD_ZERO(&set);
        FD_SET(sock, &set);
        timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = TIMEOUT_MS * 1000;
        result = select(0, NULL, &set, NULL, &timeout);
        if (result <= 0) {
            closesocket(sock);
            return;
        }
    }

    std::lock_guard<std::mutex> lock(mutex);
    ++num_open_ports;
    open_ports.push_back(port);

    closesocket(sock);
}

int main()
{
    std::string ip_addr;
    std::cout << "Enter IP address: ";
    std::cin >> ip_addr;

    std::vector<std::future<void>> tasks;

    for (int port = 1; port <= 65535; ++port) {
        tasks.push_back(std::async(std::launch::async, scan_port, ip_addr, port));
        if (tasks.size() >= MAX_THREADS) {
            for (auto& task : tasks) {
                task.get();
            }
            tasks.clear();
        }
    }

    for (auto& task : tasks) {
        task.get();
    }

    if (num_open_ports == 0) {
        std::cout << "No open ports found." << std::endl;
    }
    else {
        std::sort(open_ports.begin(), open_ports.end());
        std::stringstream ss;
        for (int port : open_ports) {
            ss << port << ", ";
        }
        std::string result = ss.str();
        result = result.substr(0, result.size() - 2);
        std::cout << "Open ports: " << result << std::endl;
    }

    return 0;
}
