#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

int main()
{
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0)
    {
        std::cerr << "WSAStartup failed: " << result << std::endl;
        return 1;
    }

    std::string ip_addr;
    std::cout << "Enter IP address: ";
    std::cin >> ip_addr;

    // Create a TCP socket
    SOCKET sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
    if (sock == INVALID_SOCKET)
    {
        std::cerr << "Socket creation failed: " << WSAGetLastError() << std::endl;
        return 1;
    }

    // Prepare sockaddr_in structure
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip_addr.c_str());

    // Scan ports from 1 to 65535
    for (int port = 1; port <= 65535; port++)
    {
        // Set port number
        addr.sin_port = htons(port);

        // Attempt to connect to the remote host
        if (connect(sock, (SOCKADDR*)&addr, sizeof(addr)) != SOCKET_ERROR)
        {
            std::cout << "Port " << port << " is open." << std::endl;
        }

        // Close socket and reset error code
        closesocket(sock);
        WSACleanup();

        result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0)
        {
            std::cerr << "WSAStartup failed: " << result << std::endl;
            return 1;
        }

        sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
        if (sock == INVALID_SOCKET)
        {
            std::cerr << "Socket creation failed: " << WSAGetLastError() << std::endl;
            return 1;
        }
    }

    // Clean up
    closesocket(sock);
    WSACleanup();

    return 0;
}
