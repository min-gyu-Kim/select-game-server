#include "framework.h"

#include "Protocol.h"
#include "Util/Packet.h"
#include "Util/RingBuffer.h"
#include "Network.h"
#include "Contents/Contents.h"
#include "PacketProc.h"

#include "Util/Log.h"
#include "Contents/Contents.h"
#include "Util/Define.h"

#include "Util/Profile.h"

void netPacketProc_MoveStart(Session* pSession, Packet* pPacket)
{
	PROFILE();
	BYTE direction;
	unsigned short ushX, ushY;

	*pPacket >> direction >> ushX >> ushY;

	_LOG(dfLOG_LEVEL_DEBUG, L"# MOVESTART > SessionID: %d / Direction: %d / X: %d / Y: %d", pSession->_id, direction, ushX, ushY);
	Character* pCharacter = FindCharacter(pSession->_id);
	if (pCharacter == nullptr)
	{
		_LOG(dfLOG_LEVEL_ERROR, L"# MOVESTART > SessionID: %d Character Not Found!", pSession->_id);
		return;
	}

	if (abs(pCharacter->_x - ushX) > dfERROR_RANGE || abs(pCharacter->_y - ushY) > dfERROR_RANGE)
	{
		_LOG(dfLOG_LEVEL_WARNING, L"# SYNC > SessionID: %d, server(%d, %d) client(%d, %d) curdir: %d dir: %d", pSession->_id, pCharacter->_x, pCharacter->_y, ushX, ushY, pCharacter->_action, direction);
		MakePacketSync(pPacket, pSession->_id, pCharacter->_x, pCharacter->_y);
		//SendPacket_SectorAround(pPacket, pSession, true);
		SendUnicast(pSession, pPacket);

		ushX = pCharacter->_x;
		ushY = pCharacter->_y;		
	}

	pCharacter->_action = direction;
	pCharacter->_moveDirection = direction;

	switch (direction)
	{
	case dfPACKET_MOVE_DIR_RR:
	case dfPACKET_MOVE_DIR_RU:
	case dfPACKET_MOVE_DIR_RD:
		pCharacter->_direction = dfPACKET_MOVE_DIR_RR;
		break;
	case dfPACKET_MOVE_DIR_LL:
	case dfPACKET_MOVE_DIR_LU:
	case dfPACKET_MOVE_DIR_LD:
		pCharacter->_direction = dfPACKET_MOVE_DIR_LL;
		break;
	}

	pCharacter->_x = ushX;
	pCharacter->_y = ushY;

	if (SectorUpdateCharacter(pCharacter))
	{
		CharacterSectorUpdatePacket(pCharacter);
	}

	MakePacketMoveStart(pPacket, pSession->_id, direction, ushX, ushY);
	SendPacket_SectorAround(pPacket, pSession);
}

void netPacketProc_MoveStop(Session* pSession, Packet* pPacket)
{	
	PROFILE();
	BYTE direction;
	unsigned short ushX, ushY;

	*pPacket >> direction >> ushX >> ushY;

	_LOG(dfLOG_LEVEL_DEBUG, L"# MOVESTOP > SessionID: %d / Direction: %d / X: %d / Y: %d", pSession->_id, direction, ushX, ushY);

	Character* pCharacter = FindCharacter(pSession->_id);
	if (pCharacter == nullptr)
	{
		_LOG(dfLOG_LEVEL_ERROR, L"# MOVESTOP > SessionID: %d Character Not Found!", pSession->_id);
		return;
	}

	pCharacter->_action = dfACTION_STAND;
	//Ãß°¡
	pCharacter->_x = ushX;
	pCharacter->_y = ushY;

	if (SectorUpdateCharacter(pCharacter))
	{
		CharacterSectorUpdatePacket(pCharacter);
	}

	MakePacketMoveStop(pPacket, pSession->_id, direction, ushX, ushY);

	//SendBroadcaset(pSession, pPacket);
	SendPacket_SectorAround(pPacket, pSession);
}

void netPacketProc_Attack1(Session* pSession, Packet* pPacket)
{
	PROFILE();
	BYTE direction;
	unsigned short ushX, ushY;

	*pPacket >> direction >> ushX >> ushY;

	_LOG(dfLOG_LEVEL_DEBUG, L"# ATTACK1 > SessionID: %d / Direction: %d / X: %d / Y: %d", pSession->_id, direction, ushX, ushY);
	Character* pCharacter = FindCharacter(pSession->_id);
	if (pCharacter == nullptr)
	{
		_LOG(dfLOG_LEVEL_ERROR, L"# MOVESTOP > SessionID: %d Character Not Found!", pSession->_id);
		return;
	}

	MakePacketAttack1(pPacket, pCharacter->_id, pCharacter->_direction, pCharacter->_x, pCharacter->_y);
	SendPacket_SectorAround(pPacket, pSession);

	Character* damageCharacter = CheckCollisionAttack1(pCharacter);
	if (damageCharacter)
	{
		damageCharacter->_hp -= dfATTACK1_DAMAGE;
		//DUMMY
		damageCharacter->_hp = max(damageCharacter->_hp, 0);

		MakePacketDamage(pPacket, pCharacter->_id, damageCharacter->_id, damageCharacter->_hp);
		//SendBroadcaset(nullptr, pPacket);
		SendPacket_SectorAround(pPacket, damageCharacter->_pSession, true);
	}
}

void netPacketProc_Attack2(Session* pSession, Packet* pPacket)
{
	PROFILE();
	BYTE direction;
	unsigned short ushX, ushY;

	*pPacket >> direction >> ushX >> ushY;

	_LOG(dfLOG_LEVEL_DEBUG, L"# ATTACK2 > SessionID: %d / Direction: %d / X: %d / Y: %d", pSession->_id, direction, ushX, ushY);
	Character* pCharacter = FindCharacter(pSession->_id);
	if (pCharacter == nullptr)
	{
		_LOG(dfLOG_LEVEL_ERROR, L"# ATTACK2 > SessionID: %d Character Not Found!", pSession->_id);
		return;
	}

	MakePacketAttack2(pPacket, pCharacter->_id, pCharacter->_direction, pCharacter->_x, pCharacter->_y);
	//SendBroadcaset(pCharacter->_pSession, pPacket);
	SendPacket_SectorAround(pPacket, pSession);

	Character* damageCharacter = CheckCollisionAttack2(pCharacter);
	if (damageCharacter)
	{
		damageCharacter->_hp -= dfATTACK2_DAMAGE;
		//DUMMY
		damageCharacter->_hp = max(damageCharacter->_hp, 0);

		MakePacketDamage(pPacket, pCharacter->_id, damageCharacter->_id, damageCharacter->_hp);
		//SendBroadcaset(nullptr, pPacket);
		SendPacket_SectorAround(pPacket, damageCharacter->_pSession, true);
	}
}

void netPacketProc_Attack3(Session* pSession, Packet* pPacket)
{
	PROFILE();
	BYTE direction;
	unsigned short ushX, ushY;

	*pPacket >> direction >> ushX >> ushY;

	_LOG(dfLOG_LEVEL_DEBUG, L"# ATTACK3 > SessionID: %d / Direction: %d / X: %d / Y: %d", pSession->_id, direction, ushX, ushY);
	Character* pCharacter = FindCharacter(pSession->_id);
	if (pCharacter == nullptr)
	{
		_LOG(dfLOG_LEVEL_ERROR, L"# ATTACK3 > SessionID: %d Character Not Found!", pSession->_id);
		return;
	}

	MakePacketAttack3(pPacket, pCharacter->_id, pCharacter->_direction, pCharacter->_x, pCharacter->_y);
	//SendBroadcaset(pCharacter->_pSession, pPacket);
	SendPacket_SectorAround(pPacket, pSession);

	Character* damageCharacter = CheckCollisionAttack3(pCharacter);
	if (damageCharacter)
	{
		damageCharacter->_hp -= dfATTACK3_DAMAGE;
		//DUMMY
		damageCharacter->_hp = max(damageCharacter->_hp, 0);
			
		MakePacketDamage(pPacket, pCharacter->_id, damageCharacter->_id, damageCharacter->_hp);
		//SendBroadcaset(nullptr, pPacket);
		SendPacket_SectorAround(pPacket, damageCharacter->_pSession, true);
	}
}

void netPacketProc_Echo(Session* pSession, Packet* pPacket)
{
	DWORD time;
	*pPacket >> time;

	MakePacketEcho(pPacket, time);
	SendUnicast(pSession, pPacket);
}

void MakePacketMoveStart(Packet* pPacket, Identity sessionID, BYTE direction, short x, short y)
{
	PROFILE();
	st_PACKET_HEADER header;
	header.byCode = dfPACKET_CODE;
	header.bySize = 9;
	header.byType = dfPACKET_SC_MOVE_START;
	pPacket->Clear();
	pPacket->PutData((char*)&header, sizeof(header));
	*pPacket << sessionID << direction << x << y;
}

void MakePacketMoveStop(Packet* pPacket, Identity sessionID, BYTE direction, short x, short y)
{
	st_PACKET_HEADER header;
	header.byCode = dfPACKET_CODE;
	header.bySize = 9;
	header.byType = dfPACKET_SC_MOVE_STOP;
	pPacket->Clear();
	pPacket->PutData((char*)&header, sizeof(header));
	*pPacket << sessionID << direction << x << y;
}

void MakePacketDamage(Packet* pPacket, Identity attackID, Identity damageID, BYTE damageHP)
{
	st_PACKET_HEADER header;
	header.byCode = dfPACKET_CODE;
	header.bySize = 9;
	header.byType = dfPACKET_SC_DAMAGE;
	pPacket->Clear();
	pPacket->PutData((char*)&header, sizeof(header));
	*pPacket << attackID << damageID << damageHP;
}

void MakePacketAttack1(Packet* pPacket, Identity sessionID, BYTE direction, short x, short y)
{
	st_PACKET_HEADER header;
	header.byCode = dfPACKET_CODE;
	header.bySize = 9;
	header.byType = dfPACKET_SC_ATTACK1;
	pPacket->Clear();
	pPacket->PutData((char*)&header, sizeof(header));
	*pPacket << sessionID << direction << x << y;
}

void MakePacketAttack2(Packet* pPacket, Identity sessionID, BYTE direction, short x, short y)
{
	st_PACKET_HEADER header;
	header.byCode = dfPACKET_CODE;
	header.bySize = 9;
	header.byType = dfPACKET_SC_ATTACK2;
	pPacket->Clear();
	pPacket->PutData((char*)&header, sizeof(header));
	*pPacket << sessionID << direction << x << y;
}

void MakePacketAttack3(Packet* pPacket, Identity sessionID, BYTE direction, short x, short y)
{
	st_PACKET_HEADER header;
	header.byCode = dfPACKET_CODE;
	header.bySize = 9;
	header.byType = dfPACKET_SC_ATTACK3;
	pPacket->Clear();
	pPacket->PutData((char*)&header, sizeof(header));
	*pPacket << sessionID << direction << x << y;
}

void SendCreateMyCharacter(Packet* pPacket, Character* pCharacter)
{
	MakePacketCreateMyCharacter(pPacket, pCharacter->_id, pCharacter->_direction, pCharacter->_x, pCharacter->_y, pCharacter->_hp);
		
	SendUnicast(pCharacter->_pSession, pPacket);
}

void MakePacketCreateMyCharacter(Packet* pPacket, Identity sessionID, BYTE direction, short x, short y, BYTE hp)
{
	st_PACKET_HEADER header;
	header.byCode = dfPACKET_CODE;
	header.bySize = 10;
	header.byType = dfPACKET_SC_CREATE_MY_CHARACTER;
	pPacket->Clear();
	pPacket->PutData((char*)&header, sizeof(header));
	*pPacket << sessionID << direction << x << y << hp;
}

void MakePacketCreateOtherCharacter(Packet* pPacket, Identity sessionID, BYTE direction, short x, short y, BYTE hp)
{
	st_PACKET_HEADER header;
	header.byCode = dfPACKET_CODE;
	header.bySize = 10;
	header.byType = dfPACKET_SC_CREATE_OTHER_CHARACTER;
	pPacket->Clear();
	pPacket->PutData((char*)&header, sizeof(header));
	*pPacket << sessionID << direction << x << y << hp;
}

void MakePacketDeleteCharacter(Packet* pPacket, Identity sessionID)
{
	st_PACKET_HEADER header;
	header.byCode = dfPACKET_CODE;
	header.bySize = 4;
	header.byType = dfPACKET_SC_DELETE_CHARACTER;
	pPacket->Clear();
	pPacket->PutData((char*)&header, sizeof(header));
	*pPacket << sessionID;
}

void MakePacketSync(Packet* pPacket, Identity sessionID, short x, short y)
{
	st_PACKET_HEADER header;
	header.byCode = dfPACKET_CODE;
	header.bySize = 8;
	header.byType = dfPACKET_SC_SYNC;
	pPacket->Clear();
	pPacket->PutData((char*)&header, sizeof(header));
	*pPacket << sessionID << x << y;
}

void MakePacketEcho(Packet* pPacket, DWORD time)
{
	st_PACKET_HEADER header;
	header.byCode = dfPACKET_CODE;
	header.bySize = 4;
	header.byType = dfPACKET_SC_ECHO;
	pPacket->Clear();
	pPacket->PutData((char*)&header, sizeof(header));
	*pPacket << time;
}
