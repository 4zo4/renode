//
// Copyright (c) 2010-2024 Antmicro
//
// This file is licensed under the MIT License.
// Full license text is available in 'licenses/MIT.txt'.
//

#include <thread>
#include <chrono>
#include "socket_channel.h"
#include "src/renode_log.h"

#define TIME_TO_MS(min, sec, ms) (((min) * 60000UL) + ((sec) * 1000UL) + (ms))
#define MAX_WAIT_TIME TIME_TO_MS(2,0,0)

SocketCommunicationChannel::SocketCommunicationChannel()
{
    ASocket::SettingsFlag dontLog = ASocket::NO_FLAGS;
    mainSocket.reset(new CTCPClient(NULL, dontLog));
    senderSocket.reset(new CTCPClient(NULL, dontLog));
}

void SocketCommunicationChannel::connect(int receiverPort, int senderPort, const char* address)
{
    mainSocket->Connect(address, std::to_string(receiverPort));
    senderSocket->Connect(address, std::to_string(senderPort));
#if defined(ENABLE_WAIT_LIMIT) && (ENABLE_WAIT_LIMIT == 1)
    mainSocket->SetRxBlocking(false);
    int count = 0;
    while (count++ < MAX_WAIT_TIME) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        if (handshakeValid())
            break;
    }
    mainSocket->SetRxBlocking(true);
#else
    handshakeValid();
#endif
    if (isConnected()) {
        sendMain(Protocol(handshake, 0, 0, noPeripheralIndex));
        std::cout << "<- Handshake acked" << std::endl;
    } else {
        std::cout << "Connection failed" << std::endl;
    }
}

void SocketCommunicationChannel::disconnect()
{
    connected = false;
}

bool SocketCommunicationChannel::isConnected()
{
    return connected;
}

bool SocketCommunicationChannel::handshakeValid()
{
    bool done = false;
    Protocol* received = receive();
    if (received->actionId == handshake) {
        connected = true;
        done = true;
        std::cout << "-> Handshake received" << std::endl;
    } else if (received->actionId == invalidAction && received->peripheralIndex < 0) {
        done = true;
    }
    delete received;
    return done;
}

void SocketCommunicationChannel::log(int logLevel, const char* data)
{
    sendSender(Protocol(logMessage, strlen(data), logLevel, noPeripheralIndex));
    senderSocket->Send(data, strlen(data));
}

Protocol* SocketCommunicationChannel::receive()
{
    Protocol* message = new Protocol;

    int bytesRead = mainSocket->Receive((char *)message, sizeof(Protocol));

    if (bytesRead < 0) {
        connected = false;
        message->actionId = invalidAction;
        message->peripheralIndex = bytesRead;
        std::cout << "Connection " <<
            (bytesRead == -1 ? "error" : (bytesRead == -2 ? "shut down" : "closed")) << std::endl;
    } else if (bytesRead < (int)sizeof(Protocol)) {
        message->actionId = invalidAction;
        message->peripheralIndex = 0;
    }
    return message;
}

void SocketCommunicationChannel::sendMain(const Protocol message)
{
    try {
        LOG_ARGS("TX", message.actionId, message.addr, message.value, message.peripheralIndex);
        mainSocket->Send((char *)&message, sizeof(struct Protocol));
    }
    catch(const char* msg) {
        connected = false;
        throw msg;
    }
}

void SocketCommunicationChannel::sendSender(const Protocol message)
{
    try {
        LOG_ARGS("TX", message.actionId, message.addr, message.value, message.peripheralIndex);
        senderSocket->Send((char *)&message, sizeof(struct Protocol));
    }
    catch(const char* msg) {
        connected = false;
        throw msg;
    }
}

