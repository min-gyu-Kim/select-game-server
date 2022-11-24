#pragma once

void NetProcCreateMyCharacter(const char* buffer);
void NetProcCreateOtherCharacter(const char* buffer);
void NetProcDeleteCharacter(const char* buffer);
void NetProcMoveStart(const char* buffer);
void NetProcMoveStop(const char* buffer);
void NetProcAttack1(const char* buffer);
void NetProcAttack2(const char* buffer);
void NetProcAttack3(const char* buffer);
void NetProcDamage(const char* buffer);

class Player;
extern List<Player*> g_playerList;
extern List<Effect*> g_effectList;
extern Player* g_myPlayer;

void SetEffect(Player* attackerPlayer, Player* attacker);

void NetSetMoveStart(PacketHeader* outHeader, stPACKET_CS_MOVE_START* outPayload, unsigned char byDirection, unsigned short x, unsigned short y);
void NetSetMoveStop(PacketHeader* outHeader, stPACKET_CS_MOVE_STOP* outPayload, unsigned byDirection, unsigned short x, unsigned short y);
void NetSetAttack1(PacketHeader* outHeader, stPACKET_CS_ATTACK1* outPayload, unsigned byDirection, unsigned short x, unsigned short y);
void NetSetAttack2(PacketHeader* outHeader, stPACKET_CS_ATTACK2* outPayload, unsigned byDirection, unsigned short x, unsigned short y);
void NetSetAttack3(PacketHeader* outHeader, stPACKET_CS_ATTACK3* outPayload, unsigned byDirection, unsigned short x, unsigned short y);