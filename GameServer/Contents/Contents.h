#pragma once

#define dfSECTOR_MAX_X 32
#define dfSECTOR_MAX_Y 32
#define dfSECTOR_SIZE_X 200
#define dfSECTOR_SIZE_Y 200

typedef struct st_SECTOR_POS
{
	int iX;
	int iY;
} SectorPos;

typedef struct st_SECTOR_AROUND
{
	int iCount;
	SectorPos Around[9];
} SectorAround;

typedef struct st_CHARACTER
{
	Session* _pSession;
	Identity _id;

	DWORD	_action;
	BYTE	_direction;
	BYTE	_moveDirection;

	unsigned short	_x;
	unsigned short	_y;

	SectorPos		_curSectorPos;
	SectorPos		_oldSectorPos;

	char			_hp;
} Character;

#ifdef __WINDBG__
extern HDC g_memDC;
extern HBITMAP g_bitmap;
extern HPEN	g_whitePen;
#endif

Character* FindCharacter(const Identity& sessionIdentity);
Character* CreateCharacter(Session* pSession);
void DisconnectCharacter(const Identity& sessionIdentity);

bool CheckMoveCharacter(short x, short y);
Character* CheckCollisionAttack1(Character* pCharacter);
Character* CheckCollisionAttack2(Character* pCharacter);
Character* CheckCollisionAttack3(Character* pCharacter);

void Update();

void SectorAddCharacter(Character* pCharacter);
void SectorRemoveCharacter(Character* pCharacter);
void SectorGetAround(int iSectorX, int iSectorY, SectorAround* pSectorAround);
void SectorGetUpdateAround(Character* pCharacter, SectorAround* pRemoveSector, SectorAround* pAddSector);
bool SectorUpdateCharacter(Character* pCharacter);

void CharacterSectorUpdatePacket(Character* pCharacter);

void SendPacket_SectorOne(int iSectorX, int iSectorY, Packet* pPacket, Session* pSession);
void SendPacket_SectorAround(Packet* pPacket, Session* pSession, bool sendMe = false);