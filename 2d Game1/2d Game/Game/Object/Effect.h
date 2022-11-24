#pragma once

class Player;

class Effect :
	public BaseObject
{
public:
	Effect();
	~Effect();

	virtual void Update() override;
	virtual void Render(BYTE* screen, size_t width, size_t height, size_t pitch) override;

	void SetPlayer(Player* player, Player* attacker) { _player = player; _attacker = attacker; _sprite_now = _sprite_start; }
	bool IsRun() const { return _player != nullptr; }

private:
	bool _isRun = false;
	Player* _player;
	Player* _attacker;
};