#ifndef _CDBSOCKET_
#define _CDBSOCKET_

#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include <Protocol/Packet.h>
#include <Protocol/MainProtocol.h>
#include <minwindef.h>
#include <common.h>

#include "../CClient.h"
#include "../CPlayer.h"

class CDBSocket
{
	static SOCKET g_pDBSocket;

public:
	static bool Connect(WORD wPort);
	static bool Close();
	static PVOID ProcessDB(PVOID packet);
	static PVOID Process(PVOID packet);

	static bool Write(BYTE byType, ...);

	static void DebugRawPacket(Packet *packet);
};

#endif