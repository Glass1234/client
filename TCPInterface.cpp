#include "TCPInterface.h"

//----------------------------------------------------------------------------------//
TCPHandlerInterface::TCPHandlerInterface() : handler(NULL) {}

#ifdef _WIN32

TCPHandlerInterface::TCPHandlerInterface(SOCKET handler) : handler(handler) {}

#else
TCPHandlerInterface::TCPHandlerInterface(int handler): handler(handler) {}
#endif

int TCPHandlerInterface::read(uint8_t *data, int len) {
    memset(data, 0, len);
    int ret = recv(this->handler, (char *) data, len, 0);
    this->alive = ret < 0 ? false : true;
    return ret;
}

int TCPHandlerInterface::write(uint8_t *data, int len) {
    int ret = send(this->handler, (char *) data, len, 0);
    this->alive = ret < 0 ? false : true;
    return ret;
}

void TCPHandlerInterface::sendPacket(PACKET_t &packet) {
    this->write((uint8_t *) &packet.type, sizeof(PACKET_TYPE_t));
    this->write((uint8_t *) &packet.size, sizeof(uint32_t));
    this->write(packet.data, sizeof(uint8_t) * packet.size);
}

PACKET_t TCPHandlerInterface::recvPacket() {
    PACKET_t newPacket;
    this->read((uint8_t *) &newPacket.type, sizeof(PACKET_TYPE_t));
    this->read((uint8_t *) &newPacket.size, sizeof(uint32_t));
    newPacket.data = new uint8_t[newPacket.size];
    this->read(newPacket.data, sizeof(uint8_t) * newPacket.size);
    return newPacket;
}

bool TCPHandlerInterface::isAlive() {
    return this->alive;
}


TCPHandlerInterface::~TCPHandlerInterface() {
#ifdef _WIN32
    closesocket(this->handler);
#else
    close(this->handler);
#endif
}

PACKET_t copyPacket(PACKET_t &packet) {
    PACKET_t newPacket;
    newPacket.type = packet.type;
    newPacket.size = packet.size;
    newPacket.data = new uint8_t[packet.size];
    memcpy(newPacket.data, packet.data, packet.size);
    return newPacket;
}


//----------------------------------------------------------------------------------//
TCPClient::TCPClient(std::string ip, int port) : TCPHandlerInterface() {
#ifdef _WIN32
    WSADATA WsaData;
    if (WSAStartup(MAKEWORD(2, 2), &WsaData) != NO_ERROR) {
        exit(0);
    };
#endif
    this->addr.sin_family = AF_INET;
    this->addr.sin_port = htons(port);
    this->addr.sin_addr.s_addr = inet_addr(ip.c_str());

    this->handler = socket(AF_INET, SOCK_STREAM, 0);
    if (this->handler < 0) {
        perror("socket");
        exit(1);
    }

    if (connect(this->handler, (sockaddr *) &this->addr, sizeof(this->addr)) < 0) {
        perror("connect");
        exit(2);
    }
}

TCPClient::~TCPClient() {
#ifdef _WIN32
    WSACleanup();
    closesocket(this->handler);
#else
    close(this->handler);
#endif
}


//----------------------------------------------------------------------------------//
TCPServer::TCPServer(int port, int maxconn) {
#ifdef _WIN32
    WSADATA WsaData;
    if (WSAStartup(MAKEWORD(2, 2), &WsaData) != NO_ERROR) {
        exit(0);
    };
#endif
    this->addr.sin_family = AF_INET;
    this->addr.sin_port = htons(port);
    this->addr.sin_addr.s_addr = htonl(INADDR_ANY);

    this->handler = socket(AF_INET, SOCK_STREAM, 0);
    if (this->handler < 0) {
        perror("socket");
        exit(1);
    }

    if (bind(this->handler, (sockaddr *) &this->addr, sizeof(this->addr)) < 0) {
        perror("bind");
        exit(2);
    }

    listen(this->handler, maxconn);
}

TCPHandlerInterface *TCPServer::acceptConn() {
#ifdef _WIN32
    SOCKET handler;
#else
    int handler;
#endif
    handler = accept(this->handler, NULL, NULL);
    if (handler < 0) {
        perror("accept");
        //exit(3);
        return NULL;
    }
    return new TCPHandlerInterface(handler);
}

TCPServer::~TCPServer() {
#ifdef _WIN32
    WSACleanup();
    closesocket(this->handler);
#else
    close(this->handler);
#endif
}
