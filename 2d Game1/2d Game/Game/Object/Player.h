#pragma once

class Player : public BaseObject
{
public:
	Player(bool is_player, unsigned char direction);
	~Player();

private:
	bool _is_player;
	char _hp;

	DWORD _prev_action;
	DWORD _cur_action;
	DWORD _direction;

public:
	void Update() override;
	void Render(BYTE* screen, size_t width, size_t height, size_t pitch) override;

	void ActionProc();

	bool IsPlayer() const { return _is_player; }
	char GetHP() const { return _hp; }
	void SetHP(char hp) { _hp = hp; }
	DWORD GetDirection() const { return _direction; }
	void SetDirection(DWORD dir) { _direction = dir; }

	DWORD GetCurAction() const { return _cur_action; }

private:
	void SetAction();
	void SetActionStand();
	void SetActionMove();

private:
	void SendMove();
	void SendStop();
	void SendAttack1(unsigned char byDir);
	void SendAttack2(unsigned char byDir);
	void SendAttack3(unsigned char byDir);
};

class RingBuffer;
extern RingBuffer* g_sendBuf;
extern bool g_isNet;
extern RingBuffer* g_recvBuf;
extern SOCKET g_sock;
