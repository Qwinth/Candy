#include <iostream>
#include <fstream>
#include <string.h>
#include "strlib.hpp"

#ifdef _WIN32
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <ws2tcpip.h>
#define GETSOCKETERRNO() (to_string(WSAGetLastError()))

#elif __linux__
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
#define GETSOCKETERRNO() (to_string(errno))
#endif
using namespace std;

class SSocket {
    int af;
    struct recvdata {
        char* value;
        unsigned long long length;
    };
    struct address {
        string ip;
        int port;
    };
    struct sockaddr_in sock, client, my_addr;
public:
#ifdef _WIN32
    WSADATA wsa;
    SOCKET s;
#elif __linux__
    int s;
#endif

    SSocket(int _af, int type) {
        af = _af;
#ifdef _WIN32
        WSAStartup(MAKEWORD(2, 2), &wsa);
#endif
        if ((s = socket(af, type, 0)) == INVALID_SOCKET)
        {   
            throw GETSOCKETERRNO();
        }



    }
#ifdef _WIN32
    SSocket(SOCKET ss) {
        s = ss;
        WSAStartup(MAKEWORD(2, 2), &wsa);
    }
#elif __linux__
    SSocket(int ss) {
        s = ss;
    }
#endif


    void sconnect(string ip, int port) {
        if (ip != "") {
            if (ip.length() > 3) {
                if (!isdigit(ip[0]) && !isdigit(ip[1]) && !isdigit(ip[2])) {
                    ip = sgethostbyname(ip);
                }
                sock.sin_addr.s_addr = inet_addr(ip.c_str());
            }   
        }
        else {
            sock.sin_addr.s_addr = INADDR_ANY;
        }
        sock.sin_family = af;
        sock.sin_port = htons(port);
        if (connect(s, (struct sockaddr*)&sock, sizeof(sock)) == SOCKET_ERROR) {
            throw GETSOCKETERRNO();
        }
    }

    void sbind(string ip, int port) {
        if (ip != "") {
            if (!isdigit(ip[0]) && !isdigit(ip[1]) && !isdigit(ip[2])) {
                
                ip = sgethostbyname(ip);
            }
            sock.sin_addr.s_addr = inet_addr(ip.c_str());
        }
        else {
            sock.sin_addr.s_addr = INADDR_ANY;
        }
        sock.sin_family = af;
        sock.sin_port = htons(port);

        if (bind(s, (struct sockaddr*)&sock, sizeof(sock)) == SOCKET_ERROR) {
            //cout << "binding error" << endl;
            throw GETSOCKETERRNO();
        }

    }

    string sgethostbyname(string name) {
        struct hostent* remoteHost;
        struct in_addr addr;
        remoteHost = gethostbyname(name.c_str());
        addr.s_addr = *(u_long*)remoteHost->h_addr_list[0];
        return string(inet_ntoa(addr));
    }


    address sgetsockname() {
        memset(&my_addr, 0, sizeof(my_addr));
        int addrlen = sizeof(my_addr);
#ifdef _WIN32
        if (getsockname(s, (struct sockaddr*)&my_addr, &addrlen) == SOCKET_ERROR) {
            throw GETSOCKETERRNO();
        }
        
        
#elif __linux__
        socklen_t len = sizeof(my_addr);
        if (getsockname(s, (struct sockaddr*)&my_addr, &len) == SOCKET_ERROR){
            throw GETSOCKETERRNO();
        }
#endif
        address addr;
        addr.ip = inet_ntoa(my_addr.sin_addr);
        addr.port = my_addr.sin_port;
        return addr;
    }

    void ssetsockopt(int level, int optname, int optval) {
#ifdef _WIN32
        setsockopt(s, level, optname, (char *)&optval, sizeof((char *)optval));
#elif __linux__
        setsockopt(s, level, optname, &optval, sizeof(int));
#endif
    }



    void slisten(int clients) {
        if (listen(s, clients) == SOCKET_ERROR) {
            //cout << "listening error" << endl;
            throw GETSOCKETERRNO();
        }
    }

    SSocket saccept() {
        int c = sizeof(struct sockaddr_in);
#ifdef _WIN32
        auto new_socket = accept(s, (struct sockaddr*)&client, &c);
#elif __linux__
        auto new_socket = accept(s, (struct sockaddr*)&client, (socklen_t*)&c);
#endif
        if (new_socket == INVALID_SOCKET) {
            throw format("%s", GETSOCKETERRNO());
        }

        return SSocket(new_socket);
    }

    void ssend(string data) {
#ifdef _WIN32

        send(s, data.c_str(), data.length(), 0);
#elif __linux__
        send(s, data.c_str(), data.length(), MSG_NOSIGNAL);
#endif
    }

    void ssend(const char * data) {
#ifdef _WIN32
        send(s, data, strlen(data), 0);
#elif __linux__
        send(s, data, strlen(data), MSG_NOSIGNAL);
#endif
    }

    void ssend_file(ifstream& file) {
        char buffer[65536];
        while (file.tellg() != -1) {
            file.read(buffer, 65536);
#ifdef _WIN32
        send(s, buffer, file.gcount(), 0);
#elif __linux__
        send(s, buffer, file.gcount(), MSG_NOSIGNAL);
#endif
        memset(buffer, 0, sizeof(buffer));
        }
    }

    string srecv(int length) {
        if (length > 32768){
            cout << "Error: srecv max value 32768" << endl;
            exit(EXIT_FAILURE);
        }
        char buffer[32768];
        recv(s, buffer, length, 0);
        string recv_data = buffer;
        return recv_data;

    }

    recvdata srecv_char(int length) {
        if (length > 32768) {
            cout << "Error: srecv_char max value 32768" << endl;
            exit(EXIT_FAILURE);
        }
        recvdata data;
        char buffer[32768];
        data.length = recv(s, buffer, length, 0);
        data.value = buffer;
        return data;

    }

    void sclose() {
#ifdef _WIN32
        closesocket(s);
        WSACleanup();
#elif __linux__
        close(s);
#endif
    }
};