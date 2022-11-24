#pragma once

typedef struct stPACKET_SC_HEADER
{
	unsigned char byCode;
	unsigned char bySize;
	unsigned char byType;
} PacketHeader;

#pragma pack(push, 1)
typedef struct stPACKET_SC_CREATE_MY_CHARACTER
{
	DWORD dwID;
	unsigned char byDirection;
	unsigned short shX;
	unsigned short shY;
	unsigned char byHP;
};

typedef struct stPACKET_SC_CREATE_OTHER_CHARACTER
{
	DWORD dwID;
	unsigned char byDirection;
	unsigned short shX;
	unsigned short shY;
	unsigned char byHP;
};

typedef struct stPACKET_SC_DELETE_CHARACTER
{
	DWORD dwID;	
};

typedef struct stPACKET_SC_MOVE_START
{
	DWORD dwID;
	unsigned char byDirection;
	unsigned short shX;
	unsigned short shY;
};

typedef struct stPACKET_SC_MOVE_STOP
{
	DWORD dwID;
	unsigned char byDirection;
	unsigned short shX;
	unsigned short shY;
};

typedef struct stPACKET_SC_ATTACK1
{
	DWORD dwID;
	unsigned char byDirection;
	unsigned short shX;
	unsigned short shY;
};

typedef struct stPACKET_SC_ATTACK2
{
	DWORD dwID;
	unsigned char byDirection;
	unsigned short shX;
	unsigned short shY;
};

typedef struct stPACKET_SC_ATTACK3
{
	DWORD dwID;
	unsigned char byDirection;
	unsigned short shX;
	unsigned short shY;
};

typedef struct stPACKET_SC_DAMAGE
{
	DWORD dwAttackerID;
	DWORD dwVictimID;
	unsigned char byVictimHP;
};


typedef struct stPACKET_CS_MOVE_START
{
	unsigned char byDirection;
	unsigned short shX;
	unsigned short shY;
};

typedef struct stPACKET_CS_MOVE_STOP
{
	unsigned char byDirection;
	unsigned short shX;
	unsigned short shY;
};

typedef struct stPACKET_CS_ATTACK1
{
	unsigned char byDirection;
	unsigned short shX;
	unsigned short shY;
};

typedef struct stPACKET_CS_ATTACK2
{
	unsigned char byDirection;
	unsigned short shX;
	unsigned short shY;
};

typedef struct stPACKET_CS_ATTACK3
{
	unsigned char byDirection;
	unsigned short shX;
	unsigned short shY;
};

#pragma pack(pop)