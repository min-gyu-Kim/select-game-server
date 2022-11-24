#pragma once

class Player;

#define dfCAMERA_END_X 6400 - 640
#define dfCAMERA_END_Y 6400 - 480

class Camera
{
public:
	void Initialize(Player* player);
	void Update();

	POINT GetPosition() const { return _position; }
private:
	POINT _position;
	Player* _player;

private:
	Camera();
	~Camera();

private:
	static Camera s_instance;

public:
	static Camera* GetInstance()
	{
		return &s_instance;
	}
};

