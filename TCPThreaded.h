#pragma once

#include <iostream>
#include <vector>
#include <queue>
#include <iomanip>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "TCPInterface.h"
#include "log.h"



class TCPThreaded {
private:
    std::mutex send_mtxCond;
    std::mutex recv_mtxCond;
    std::condition_variable send_waitCond;
    std::condition_variable recv_waitCond;

    std::queue<PACKET_t> sendBuffer;
    std::queue<PACKET_t> recvBuffer;

public:
    TCPHandlerInterface *handler;

    TCPThreaded(TCPHandlerInterface *handler) : handler(handler) {
        std::thread(&TCPThreaded::recvThreadHandler, this).detach();
        std::thread(&TCPThreaded::sendThreadHandler, this).detach();
    }

    void push(PACKET_t &packet) {
        this->sendBuffer.push(packet);
        this->send_waitCond.notify_all();
    }

    PACKET_t pop() {
        {
            std::unique_lock<std::mutex> ul(this->recv_mtxCond);
            this->recv_waitCond.wait(ul, [=]() { return this->recvBuffer.size() > 0 || !handler->isAlive(); });
        }
        PACKET_t newPacket = {PACKET_CLOSE, 0, 0};
        if (handler->isAlive()) {
//            LOG_INFO("Pop wakeup");
            newPacket = this->recvBuffer.front();
            this->recvBuffer.pop();
        }
        return newPacket;
    }

    void sendThreadHandler() {
        while (true) {
            {
                std::unique_lock<std::mutex> ul(this->send_mtxCond);
                this->send_waitCond.wait(ul, [=]() { return this->sendBuffer.size() > 0 || !handler->isAlive(); });
            }
            if (!handler->isAlive()) {
                this->recv_waitCond.notify_all();
                break;
            }
//            LOG_INFO("Send thread wakeup");
            if (this->sendBuffer.size() > 0 && handler->isAlive()) {
                PACKET_t newPacket = this->sendBuffer.front();
                this->sendBuffer.pop();
                handler->sendPacket(newPacket);
                delete newPacket.data;
            }
        }
    }

    void recvThreadHandler() {
        while (true) {
            PACKET_t newPacket = handler->recvPacket();
            if (handler->isAlive()) {
                this->recvBuffer.push(newPacket);
                this->recv_waitCond.notify_all();
            } else {
                this->send_waitCond.notify_all();
                break;
            }
        }
    }

    ~TCPThreaded() {
        this->send_waitCond.notify_all();
        this->recv_waitCond.notify_all();
        delete handler;
    }
};
