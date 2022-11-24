#include "framework.h"

#include "Util/Packet.h"
#include "Util/RingBuffer.h"
#include "Protocol.h"
#include "Network.h"

#include "Util/MemoryPool.h"

#include "Contents/Contents.h"
#include "PacketProc.h"
#include "Util/Log.h"

//#include "Contents.h"

//#define _PROFILE
#include "Util/Profile.h"

#pragma comment(lib, "Ws2_32.lib")

static SOCKET g_listenSocket;
static Identity g_clientID = 1;

std::unordered_map<SOCKET, Session*>	g_umapSession;

MemoryPool<Packet> g_packetPool(1);

//DEBUG var
DWORD g_sendCnt = 0;
DWORD g_TPS = 0;

bool InitNetwork()
{
	WSADATA wsadata;
	if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0)
	{
		return false;
	}

	g_listenSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (g_listenSocket == INVALID_SOCKET)
	{
		return false;
	}

	SOCKADDR_IN serverAddr;
	ZeroMemory(&serverAddr, sizeof(SOCKADDR_IN));
	serverAddr.sin_addr.s_addr = htonl(ADDR_ANY);
	serverAddr.sin_port = htons(dfNETWORK_PORT);
	serverAddr.sin_family = AF_INET;

	if (bind(g_listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) != 0)
	{
		return false;
	}

	if (listen(g_listenSocket, SOMAXCONN_HINT(500)) != 0)
	{
		return false;
	}

	int opt_val = 1;

	setsockopt(g_listenSocket, IPPROTO_TCP, TCP_NODELAY, (const char*)&opt_val, sizeof(opt_val));
	
	u_long mode = 1;
	if (ioctlsocket(g_listenSocket, FIONBIO, &mode) != 0)
	{
		return false;
	}

	linger opt;
	opt.l_onoff = 1;
	opt.l_linger = 0;
	setsockopt(g_listenSocket, SOL_SOCKET, SO_LINGER, (const char*)&opt, sizeof(opt));

	return true;
}

void ShutdownNetwork()
{
	closesocket(g_listenSocket);

	WSACleanup();
}

void ProcessNetwork()
{
	PROFILE();

	Session* pSession;
	fd_set readSet;
	fd_set writeSet;
	size_t setSize = 0;
	SOCKET arySockets[FD_SETSIZE];

	std::unordered_map<SOCKET, Session*>::iterator iter = g_umapSession.begin();

	FD_ZERO(&readSet);
	FD_ZERO(&writeSet);

	FD_SET(g_listenSocket, &readSet);

	arySockets[0] = g_listenSocket;
	setSize++;
	
	for (; iter != g_umapSession.end();)
	{
		pSession = iter->second;

		++iter;

		arySockets[setSize] = pSession->_socket;

		setSize++;

		FD_SET(pSession->_socket, &readSet);
		if (pSession->_sendBuf.GetUseSize() > 0)
		{
			FD_SET(pSession->_socket, &writeSet);
		}

		if (setSize >= FD_SETSIZE)
		{
			ProcessSelect(arySockets, setSize, &readSet, &writeSet);

			FD_ZERO(&readSet);
			FD_ZERO(&writeSet);

			memset(arySockets, INVALID_SOCKET, sizeof(SOCKET) * FD_SETSIZE);
			//FD_SET(g_listenSocket, &readSet);

			//arySockets[0] = g_listenSocket;
			//setSize = 1;
			setSize = 0;
		}
	}
	if (setSize > 0)
	{
		ProcessSelect(arySockets, setSize, &readSet, &writeSet);
	}
}

void ProcessSelect(const SOCKET* arySockets, size_t arySize, fd_set* readSet, fd_set* writeSet)
{
	timeval time;
	time.tv_sec = 0;
	time.tv_usec = 0;
	int selectRet = select(0, readSet, writeSet, nullptr, &time);
	int errCode;

	if (selectRet == SOCKET_ERROR)
	{
		errCode = WSAGetLastError();
		wprintf(L"select error! code : %d\n", errCode);
	}
	else if (selectRet > 0)
	{
		for (unsigned int idx = 0; idx < arySize; idx++)
		{
			if (arySockets[idx] == INVALID_SOCKET)
			{
				continue;
			}

			if (FD_ISSET(arySockets[idx], writeSet))
			{
				SendProcess(arySockets[idx]);
			}
			if (FD_ISSET(arySockets[idx], readSet))
			{
				if (arySockets[idx] == g_listenSocket)
				{
					AcceptClient();
				}
				else
				{
					RecvMessage(arySockets[idx]);
				}
			}
		}
	}
}

void AcceptClient()
{
	SOCKADDR_IN clientAddr;
	int addrSize = sizeof(clientAddr);
	SOCKET clientSocket = accept(g_listenSocket, (sockaddr*)&clientAddr, &addrSize);

	int errCode;

	if (clientSocket == INVALID_SOCKET)
	{
		errCode = WSAGetLastError();
		wprintf(L"accept error()! code : %d\n", errCode);
		return;
	}

	Session* pSession = CreateSession(clientSocket, clientAddr);
	CreateCharacter(pSession);
	_LOG(dfLOG_LEVEL_DEBUG, L"Connect Session: SessionID: %d", pSession->_id);
}

void RecvMessage(const SOCKET& sessionSocket)
{
	PROFILE();
	Session* pSession = FindSession(sessionSocket);
	if (pSession == nullptr)
	{
		return;
	}

	int errCode;
	int recvRet = recv(pSession->_socket, pSession->_recvBuf.GetRearBufferPtr(), pSession->_recvBuf.DirectEnqueueSize(), 0);
	if (recvRet == 0)
	{
		DisconnectClient(sessionSocket);
		wprintf(L"%d클라이언트 접속 종료!\n", sessionSocket);
	}
	else if (recvRet == SOCKET_ERROR)
	{
		errCode = WSAGetLastError();
		if (errCode != WSAECONNRESET)
		{
			wprintf(L"recv error! code: %d clientID: %d\n", errCode, sessionSocket);
		}

		_LOG(dfLOG_LEVEL_DEBUG ,L"[%d] Disconnect!!", pSession->_id);
		DisconnectClient(sessionSocket);

		return;
	}
	else
	{
		pSession->_recvBuf.MoveRear(recvRet);
		while (1)
		{
			//함수화
			if (pSession->_recvBuf.GetUseSize() < sizeof(st_PACKET_HEADER))
			{
				break;
			}
			st_PACKET_HEADER header;
			pSession->_recvBuf.Peek((char*)&header, sizeof(header));
			if ((unsigned int)pSession->_recvBuf.GetUseSize() < sizeof(st_PACKET_HEADER) + header.bySize)
			{
				break;
			}

			if (header.byCode != dfPACKET_CODE)
			{
				wprintf(L"[%d]Client packet code error!\n", sessionSocket);
				DisconnectClient(sessionSocket);
				return;
			}

			pSession->_recvBuf.MoveFront(sizeof(header));

			Packet* packet = g_packetPool.Alloc();
			packet->Clear();
			pSession->_recvBuf.Dequeue(packet->GetBufferPtr(), header.bySize);
			packet->MoveWritePos(header.bySize);

			if (!PacketProcess(pSession, header.byType, packet))
			{
				wprintf(L"[%d]Client packet type error! type: %d\n", sessionSocket, header.byType);
				DisconnectClient(sessionSocket);
			}

			g_packetPool.Free(packet);
		}
	}

}

void SendProcess(const SOCKET& sessionSocket)
{
	PROFILE();

	Session* pSession = FindSession(sessionSocket);
	if (!pSession)
	{
		return;
	}

	int sendRet = send(pSession->_socket, pSession->_sendBuf.GetFrontBufferPtr(), pSession->_sendBuf.DIrectDequeueSize(), 0);
	if (sendRet == SOCKET_ERROR)
	{
		int errCode = WSAGetLastError();
		if (errCode == WSAEWOULDBLOCK)
		{
			wprintf(L"[%d] SOCKET send wouldblock!\n", pSession->_id);
			return;
		}
		else if (errCode != WSAECONNRESET && errCode != WSAECONNABORTED)
		{
			wprintf(L"[%d] send error! code : %d\n", pSession->_id, errCode);
		}
		DisconnectClient(pSession->_id);
		return;
	}

	pSession->_sendBuf.MoveFront(sendRet);
}

void DisconnectClient(const SOCKET& sessionSocket)
{
	Session* pSession = FindSession(sessionSocket);
	if (!pSession)
		return;
	_LOG(dfLOG_LEVEL_DEBUG, L"disconnect! %d", pSession->_id);

	Packet packet;
	MakePacketDeleteCharacter(&packet, pSession->_id);
	//SendBroadcaset(pSession, &packet);
	SendPacket_SectorAround(&packet, pSession);
	DisconnectCharacter(pSession->_id);

	closesocket(pSession->_socket);
	g_umapSession.erase(pSession->_socket);
	delete pSession;
	//SAVE_PROFILE_DATA(L"profile");
}

bool PacketProcess(Session* pSession, unsigned char packetType, Packet* pPacket)
{
	g_TPS++;
	switch (packetType)
	{
	case dfPACKET_CS_MOVE_START:
		netPacketProc_MoveStart(pSession, pPacket);
		break;
	case dfPACKET_CS_MOVE_STOP:
		netPacketProc_MoveStop(pSession, pPacket);
		break;
	case dfPACKET_CS_ATTACK1:
		netPacketProc_Attack1(pSession, pPacket);
		break;
	case dfPACKET_CS_ATTACK2:
		netPacketProc_Attack2(pSession, pPacket);
		break;
	case dfPACKET_CS_ATTACK3:
		netPacketProc_Attack3(pSession, pPacket);
		break;
	case dfPACKET_CS_ECHO:
		netPacketProc_Echo(pSession, pPacket);
		break;
	default:
		return false;
	}
	return true;
}

Session* FindSession(const SOCKET& sessionSocket)
{
	const auto& iter = g_umapSession.find(sessionSocket);
	if (iter == g_umapSession.end())
	{
		return nullptr;
	}
	return iter->second;
}

Session* CreateSession(const SOCKET& sessionSocket, const SOCKADDR_IN& sessionAddr)
{
	Session* pSession = new Session;
	pSession->_addr = sessionAddr;
	pSession->_id = g_clientID++;
	pSession->_socket = sessionSocket;
	g_umapSession.emplace(sessionSocket, pSession);

	return pSession;
}

void SendBroadcaset(Session* pSession, Packet* pPacket)
{
	std::unordered_map<SOCKET, Session*>::iterator iter = g_umapSession.begin();
	for (; iter != g_umapSession.end(); ++iter)
	{
		if(iter->second != pSession)
			SendUnicast(iter->second, pPacket);
	}
}

void SendUnicast(Session* pSession, Packet* pPacket)
{
	PROFILE();
	//_LOG(dfLOG_LEVEL_WARNING, L"%dSession Size: %d, buf_free_Size: %d", pSession->_id, pPacket->GetDataSize(), pSession->_sendBuf.GetFreeSize());
	int size = pSession->_sendBuf.Enqueue((const char*)pPacket->GetBufferPtr(), pPacket->GetDataSize());
	//DEBUG
	g_sendCnt++;
	if (size != pPacket->GetDataSize())
	{
		_LOG(dfLOG_LEVEL_ERROR, L"Session: %d SendError!", pSession->_id);
		DisconnectClient(pSession->_socket);
	}
}
