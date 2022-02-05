#ifndef TCPINTERFACE_H
#define TCPINTERFACE_H


#include <iostream>
#include <string>


typedef enum : uint8_t {
    PACKET_CLOSE = 0x00,
    PACKET_OPEN = 0x01,
    PACKET_DATA = 0x02,
    PACKET_BROADCAST = 0x03,
    PACKET_LOGIN = 0x04,
    PACKET_REGIST = 0x05
} PACKET_TYPE_t;

typedef struct {
    PACKET_TYPE_t type;
    uint32_t size;
    uint8_t *data;
} PACKET_t;

PACKET_t copyPacket(PACKET_t &packet);

#ifdef _WIN32
#pragma comment( lib, "wsock32.lib" )

#include <WinSock2.h>

#else

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#endif

class TCPHandlerInterface {
protected:
#ifdef _WIN32
    SOCKET handler;
#else
    int handler;
#endif
    bool alive = true;

public:
    TCPHandlerInterface();

#ifdef _WIN32

    TCPHandlerInterface(SOCKET);

#else
    TCPHandlerInterface(int);
#endif

    int read(uint8_t *, int);

    int write(uint8_t *, int);

    void sendPacket(PACKET_t &);

    PACKET_t recvPacket();

    bool isAlive();

    ~TCPHandlerInterface();
};

class TCPClient : public TCPHandlerInterface {
private:
    sockaddr_in addr;

public:
    TCPClient(std::string, int);

    ~TCPClient();
};


class TCPServer {
private:
#ifdef _WIN32
    SOCKET handler;
#else
    int handler;
#endif
    sockaddr_in addr;

public:
    TCPServer(int, int);

    TCPHandlerInterface *acceptConn();

    ~TCPServer();
};

#endif // TCPINTERFACE_H
