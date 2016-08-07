#include "CDBSocket.h"
#include "../CServer.h"

SOCKET CDBSocket::g_pDBSocket = INVALID_SOCKET;

bool CDBSocket::Connect(WORD wPort)
{
	CDBSocket::g_pDBSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (CDBSocket::g_pDBSocket <= INVALID_SOCKET) {
		printf("Error creating socket.\n");
		return false;
	}

	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(sockaddr_in));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(wPort);

    if (connect(CDBSocket::g_pDBSocket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) <= SOCKET_ERROR) {
    	printf("Failed to connect to DB Server.\n");
    	return false;
    }
    
	printf("Connected to DBServer.\n");

	pthread_t t;
	pthread_create(&t, NULL, &CDBSocket::ProcessDB, NULL);

	return true;
}

bool CDBSocket::Close()
{
	return true;
}

PVOID CDBSocket::ProcessDB(PVOID param)
{
	while (true)
	{
		Packet *packet = new Packet;
		memset(packet, 0, sizeof(Packet));
		//Sleep?

		int nLen = recv(CDBSocket::g_pDBSocket, packet, MAX_PACKET_LENGTH + (packet->data-(char*)packet), 0);
		if (nLen <= 0 || packet->wSize <=0) {
			printf("DBServer disconnected.\n");
			break;
		}

		if (nLen > MAX_PACKET_LENGTH || packet->wSize > MAX_PACKET_LENGTH) continue;

		DebugRawPacket(packet);

		pthread_t t;
		pthread_create(&t, NULL, &CDBSocket::Process, (PVOID)packet);
	}
}

PVOID CDBSocket::Process(PVOID param)
{
	Packet* packet = (Packet*)param;

	switch (packet->byType)
	{
		case D2S_LOGIN:
		{
			BYTE byAnswer=0;
			int nClientID=0;
			char *p = CSocket::ReadPacket(packet->data, "bd", &byAnswer, &nClientID);

			CClient *pClient = CServer::FindClient(nClientID);
			if (!pClient) break;

			if (byAnswer == LA_SAMEUSER) {
				int nClientExID=0;
				CSocket::ReadPacket(p, "d", &nClientExID);

				auto pClientEx = CServer::FindClient(nClientExID);
				if (pClientEx)
					pClientEx->Write(S2C_CLOSE, "b", CC_SAMEUSER);
			}

			pClient->Write(S2C_ANS_LOGIN, "b", byAnswer);
			printf("S2C_ANS_LOGIN sent.\n");

			break;
		}

		case D2S_SEC_LOGIN:
		{
			int nClientID=0;
			BYTE byAnswer=0;
			CSocket::ReadPacket(packet->data, "bd", &byAnswer, &nClientID);

			CClient *pClient = CServer::FindClient(nClientID);
			if (pClient)
				pClient->Write(S2C_SECOND_LOGIN, "bb", SL_RESULT_MSG, byAnswer);

			break;
		}

		case D2S_PLAYER_INFO:
		{
			int nClientID=0;
			char *p = CSocket::ReadPacket(packet->data, "d", &nClientID);

			CClient *pClient = CServer::FindClient(nClientID);
			if (!pClient) break;

			BYTE byAuth=0;
			int nExpTime=0;
			BYTE byUnknwon=0;

			pClient->Write(S2C_PLAYERINFO, "bbdm", byAuth, byUnknwon, nExpTime, 
				p, ((char*)packet + packet->wSize) - p);
			printf("S2C_PLAYERINFO sent.\n");

			break;
		}

		case D2S_ANS_NEWPLAYER:
		{
			int nClientID=0;
			char *p = CSocket::ReadPacket(packet->data, "d", &nClientID);

			CClient *pClient = CServer::FindClient(nClientID);
			if (!pClient) break;

			pClient->Write(S2C_ANS_NEWPLAYER, "m", p, ((char*)packet + packet->wSize) - p);
			break;
		}

		case D2S_LOADPLAYER:
		{
			int nClientID;
			char* p= CSocket::ReadPacket(packet->data, "d", &nClientID);

			CClient *pClient = CServer::FindClient(nClientID);
			if (!pClient) break;

			D2S_LOADPLAYER_DESC desc;
			memset(&desc, 0, sizeof(D2S_LOADPLAYER_DESC));

			CSocket::ReadPacket(p, "ddsbbbwwwwwwwIwwwddddbb", 
				&desc.nAID, 
				&desc.nPID,
				&desc.szName,
				&desc.byClass,
				&desc.byJob,
				&desc.byLevel,
				&desc.wStats[STAT_STR], 
				&desc.wStats[STAT_HTH],
				&desc.wStats[STAT_INT],
				&desc.wStats[STAT_WIS], 
				&desc.wStats[STAT_AGI],
				&desc.wCurHP, 
				&desc.wCurMP, 
				&desc.n64Exp, 
				&desc.wPUPoint, 
				&desc.wSUPoint, 
				&desc.wContribute, 
				&desc.nAnger, 
				&desc.nX, 
				&desc.nY, 
				&desc.nZ,
				&desc.byFace,
				&desc.byHair);

			auto pPlayer = new CPlayer(nClientID, desc);
			pClient->SetPlayer(pPlayer);

			pPlayer->SendProperty();

			WORD wTime=1200;

			pClient->Write(S2C_ANS_LOAD, "wdd", wTime, pPlayer->GetX(), pPlayer->GetY());
			printf("S2C_ANS_LOAD sent.\n");

			break;
		}
	}

	delete packet;
	return NULL;
}

bool CDBSocket::Write(BYTE byType, ...)
{
	if (CDBSocket::g_pDBSocket == INVALID_SOCKET)
		return false;

	Packet packet;
	memset(&packet, 0, sizeof(Packet));

	packet.byType = byType;

	va_list va;
	va_start(va, byType);

	char* end = CSocket::WriteV(packet.data, va);

	va_end(va);

	packet.wSize = end - (char*)&packet;
	send(CDBSocket::g_pDBSocket, (char*)&packet, packet.wSize, 0);

	return true;
}

void CDBSocket::DebugRawPacket(Packet *packet)
{
	printf("Incoming D2S packet: [%u]\n", (BYTE)packet->byType);
	for (int i = 0; i < packet->wSize; i++)
		printf("%u ", (BYTE)((char*)packet)[i]);
	printf("\n");
}

