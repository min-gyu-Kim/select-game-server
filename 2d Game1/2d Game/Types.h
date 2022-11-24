#pragma once

//-----------------------------------------------------------------
// 화면 이동 범위.
//-----------------------------------------------------------------
#define dfRANGE_MOVE_TOP	0
#define dfRANGE_MOVE_LEFT	0
#define dfRANGE_MOVE_RIGHT	6400
#define dfRANGE_MOVE_BOTTOM	6400

#define dfDELAY_STAND		5
#define dfDELAY_MOVE		4
#define dfDELAY_ATTACK1		3
#define dfDELAY_ATTACK2		4
#define dfDELAY_ATTACK3		4
#define dfDELAY_EFFECT		3

#define dfACTION_MOVE_LL	0
#define dfACTION_MOVE_LU	1
#define dfACTION_MOVE_UU	2
#define dfACTION_MOVE_RU	3
#define dfACTION_MOVE_RR	4
#define dfACTION_MOVE_RD	5
#define dfACTION_MOVE_DD	6
#define dfACTION_MOVE_LD	7

#define dfACTION_STAND		8
#define dfACTION_ATTACK1	9
#define dfACTION_ATTACK2	10
#define dfACTION_ATTACK3	11

#define dfDIRECTION_LEFT	1
#define dfDIRECTION_RIGHT	2

enum class eObjectType
{
	PLAYER,
	EFFECT
};

#define dfMAP_WIDTH	100
#define dfMAP_HEIGHT	100