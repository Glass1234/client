#include <iostream>
#include <vector>
//#include <queue>
//#include <iomanip>
#include <thread>
#include <mutex>
//#include <thread>
//#include <condition_variable>
#include <regex>
#include "windows.h"

#include "TCPInterface.h"
//#include "log.h"
#include "TCPThreaded.h"

std::string userName;

bool is_valid_string(std::string str) {
    static const std::regex r(R"(\w+)");
    return regex_match(str.c_str(), r);
}

void loginUser(TCPThreaded *threaded) {
    while (true) {
        std::cout << "Login 'y' register 'n' : ";
        char responceUser;
        std::cin >> responceUser;
        if (responceUser == 'y') {
            std::string login;
            std::string password;
            std::cout << "Enter login: ";
            std::cin >> login;
            std::cout << "Enter password: ";
            std::cin >> password;
            if (is_valid_string(login) and is_valid_string(password)) {
                std::string data = login + '\0' + password + '\0';
                PACKET_t newPacket{PACKET_LOGIN, (uint32_t) data.size(), (uint8_t *) data.data()};
                PACKET_t packet = copyPacket(newPacket);
                threaded->push(packet);
                while (true) {
                    PACKET_t pack = threaded->pop();
                    uint8_t res = *pack.data;
                    if (pack.type == PACKET_LOGIN) {
                        std::cout << res << '\n';
                        Sleep(3000);
                        if (res == 1) {
                            userName = login;
                            system("cls");
                            std::cout << "//////////////////\n";
                            std::cout << "///successfully///\n";
                            std::cout << "//////////////////\n";
                            delete pack.data;
                            return;
                        } else {
                            std::cout << "this user was not found\n";
                            delete pack.data;
                            break;
                        }
                    }
                    delete pack.data;
                }
            } else {
                system("cls");
                std::cout << "Unacceptable symbols 0_0" << std::endl;
                continue;
            }
        } else if (responceUser == 'n') {
            std::string login;
            std::string password;
            std::string passwordCheck;
            std::cout << "Enter login: ";
            std::cin >> login;
            std::cout << "Enter password: ";
            std::cin >> password;
            std::cout << "Repeat password: ";
            std::cin >> passwordCheck;
            if (password != passwordCheck) {
                std::cout << "Password mismatch ::((\n";
                continue;
            }
            if (is_valid_string(login) and is_valid_string(password)) {
                std::string data = login + '\0' + password + '\0';
                PACKET_t newPacket{PACKET_REGIST, (uint32_t) data.size(), (uint8_t *) data.data()};
                PACKET_t packet = copyPacket(newPacket);
                threaded->push(packet);
                while (true) {
                    PACKET_t pack = threaded->pop();
                    uint8_t res = *pack.data;
                    if (pack.type == PACKET_REGIST) {
                        if (res == 1) {
                            std::cout << "registration completed successfully 0w0\n";
                            std::cout << "//////////////////\n";
                            std::cout << "///successfully///\n";
                            std::cout << "//////////////////\n";
                            delete pack.data;
                            return;
                        } else {
                            std::cout << "this user already exists :c\n";
                            delete pack.data;
                            break;
                        }
                    }
                    delete pack.data;
                }
            } else {
                system("cls");
                std::cout << "Unacceptable symbols 0_0" << std::endl;
            }

        }
    }
}

void messageLoop(TCPClient *client, TCPThreaded *threaded) {
    Sleep(2000);
    PACKET_t newPacket;
    while (client->isAlive()) {
        newPacket = threaded->pop();
        if (newPacket.type == PACKET_BROADCAST) {
            std::string msg = std::string((char *) newPacket.data);
            std::string name = (char *) (newPacket.data + msg.size() + 1);
            std::cout << "->" << name << ":" << msg << '\n';
        }
        delete newPacket.data;
    }
}

void globalChat(TCPClient *client, TCPThreaded *threaded) {
    Sleep(500);
    system("cls");
    std::cout << "welcome to the global chat" << std::endl;
    Sleep(1000);
    system("cls");
    while (client->isAlive()) {
        std::string data;
        getline(std::cin, data);
        data += '\0' + userName + '\0';
        PACKET_t newPacket{PACKET_DATA, (uint32_t) data.size(), (uint8_t *) data.data()};
        PACKET_t packet = copyPacket(newPacket);
        threaded->push(packet);
    }
}

int main(void) {
    TCPClient client("127.0.0.1", 9999);
    TCPThreaded *threaded = new TCPThreaded(&client);

    loginUser(threaded);
    std::thread th1(globalChat, &client, threaded);
    std::thread th2(messageLoop, &client, threaded);
    th1.detach();
    th2.detach();

    while (client.isAlive()) {
    }

    PACKET_t newPacket{PACKET_CLOSE, 0, nullptr};
    threaded->push(newPacket);
}