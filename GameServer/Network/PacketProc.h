#pragma once

void netPacketProc_MoveStart(Session* pSession, Packet* pPacket);
void netPacketProc_MoveStop(Session* pSession, Packet* pPacket);
void netPacketProc_Attack1(Session* pSession, Packet* pPacket);
void netPacketProc_Attack2(Session* pSession, Packet* pPacket);
void netPacketProc_Attack3(Session* pSession, Packet* pPacket);
void netPacketProc_Echo(Session* pSession, Packet* pPacket);

void MakePacketMoveStart(Packet* pPacket, Identity sessionID, BYTE direction, short x, short y);
void MakePacketMoveStop(Packet* pPacket, Identity sessionID, BYTE direction, short x, short y);
void MakePacketDamage(Packet* pPacket, Identity attackID, Identity damageID, BYTE damageHP);
void MakePacketAttack1(Packet* pPacket, Identity sessionID, BYTE direction, short x, short y);
void MakePacketAttack2(Packet* pPacket, Identity sessionID, BYTE direction, short x, short y);
void MakePacketAttack3(Packet* pPacket, Identity sessionID, BYTE direction, short x, short y);

void SendCreateMyCharacter(Packet* pPacket, Character* pCharacter);
void MakePacketCreateMyCharacter(Packet* pPacket, Identity sessionID, BYTE direction, short x, short y, BYTE hp);
void MakePacketCreateOtherCharacter(Packet* pPacket, Identity sessionID, BYTE direction, short x, short y, BYTE hp);
void MakePacketDeleteCharacter(Packet* pPacket, Identity sessionID);

void MakePacketSync(Packet* pPacket, Identity sessionID, short x, short y);
void MakePacketEcho(Packet* pPacket, DWORD time);

//	4	-	ID
//	1	-	Direction
//	2	-	X
//	2	-	Y
//	1	-	HP