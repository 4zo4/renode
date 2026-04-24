//
// Copyright (c) 2010-2024 Antmicro
//
// This file is licensed under the MIT License.
// Full license text is available in 'licenses/MIT.txt'.
//

#include "renode_dpi.h"
#include "communication/socket_channel.h"
#include "renode_log.h"

static SocketCommunicationChannel *socketChannel;

bool renodeDPIReceive(uint32_t* actionId, uint64_t* address, uint64_t* value, int32_t* peripheralIndex)
{
    if(!renodeDPIIsConnected())
    {
        return false;
    }
#if defined(ENABLE_FILTER) && (ENABLE_FILTER == 1)
#define DELETE(p) delete p
    socketChannel->setPeek(true);
    Protocol *p = socketChannel->receive();
    socketChannel->setPeek(false);

    if (p->actionId == invalidAction) {
        delete p;
        return false;
    }
    if ((p->actionId >= readRequestByte && p->actionId <= writeRequestQuadWord) || p->actionId == interrupt)
#else // NO FILTER
#define DELETE(p) do {} while(0)
#endif
    {
        Protocol *message = socketChannel->receive();

        *actionId = message->actionId;
        *address = message->addr;
        *value = message->value;
        *peripheralIndex = message->peripheralIndex;

        DELETE(p);
        delete message;
        LOG_ARGS("V-LOG", *actionId, *address, *value, *peripheralIndex);
        return true;
    }
    DELETE(p);
    return false;
}

void renodeDPIConnect(int receiverPort, int senderPort, const char* address)
{
    if (!socketChannel)
    {
        LOG_FUNC("V-LOG");
        socketChannel = new SocketCommunicationChannel();
        socketChannel->connect(receiverPort, senderPort, address);
    }
}

void renodeDPIDisconnect()
{
    if(socketChannel != NULL)
    {
        LOG_FUNC("V-LOG");
        socketChannel->disconnect();
        delete socketChannel;
        socketChannel = NULL;
    }
}

bool renodeDPIIsConnected()
{
    return socketChannel != NULL && socketChannel->isConnected();
}

bool renodeDPISend(uint32_t actionId, uint64_t address, uint64_t value, int32_t peripheralIndex)
{
    if(!renodeDPIIsConnected())
    {
        return false;
    }
    LOG_FUNC("V-LOG");
    socketChannel->sendMain(Protocol(actionId, address, value, peripheralIndex));
    return true;
}

bool renodeDPISendToAsync(uint32_t actionId, uint64_t address, uint64_t value, int32_t peripheralIndex)
{
    if(!renodeDPIIsConnected())
    {
        return false;
    }
    LOG_FUNC("V-LOG");
    socketChannel->sendSender(Protocol(actionId, address, value, peripheralIndex));
    return true;
}

bool renodeDPILog(int logLevel, const char* data)
{
    if(!renodeDPIIsConnected())
    {
        return false;
    }
    LOG_FUNC("V-LOG");
    socketChannel->log(logLevel, data);
    return true;
}

SocketCommunicationChannel *renodeDPIGetSocketChannel()
{
    return socketChannel;
}
