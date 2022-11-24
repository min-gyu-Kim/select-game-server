#pragma once

typedef unsigned int Identity;

typedef struct stSession
{
	//stSession() : _sendBuf(20000), _recvBuf(20000) {}

	SOCKET		_socket;
	SOCKADDR_IN _addr;
	Identity	_id;
	RingBuffer	_recvBuf;
	RingBuffer	_sendBuf;
	DWORD		_lastRecvTime;
} Session;

bool InitNetwork();
void ShutdownNetwork();

void ProcessNetwork();
void ProcessSelect(const SOCKET* arySockets, size_t arySize, fd_set* readSet, fd_set* writeSet);

void AcceptClient();
void RecvMessage(const SOCKET& sessionSocket);
void SendProcess(const SOCKET& sessionSocket);
void DisconnectClient(const SOCKET& sessionSocket);

bool PacketProcess(Session* pSession, unsigned char packetType, Packet* pPacket);

Session* FindSession(const SOCKET& sessionSocket);
Session* CreateSession(const SOCKET& sessionSocket, const SOCKADDR_IN& sessionAddr);

void SendBroadcaset(Session* pSession, Packet* pPacket);
void SendUnicast(Session* pSession, Packet* pPacket);