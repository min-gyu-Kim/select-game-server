#pragma once
class SpriteDib
{
public:
	typedef struct _SPRITE
	{
		BYTE* _buffer;
		size_t _width;
		size_t _height;
		size_t _pitch;

		POINT _center_position;
	};

public:
	bool Initiailze(size_t max_sprite_count, DWORD color_key);

	void LoadSprite(const wchar_t* sprite_path, unsigned int sprite_index, const POINT& center_position);
	void ReleaseSprite(unsigned int sprite_index);
	void ReleaseAll();

	void DrawSprite(unsigned int sprite_index, const POINT& position, BYTE* back_buffer, size_t width, size_t height, size_t pitch, int len = 100);
	void DrawSprite_BK(unsigned int sprite_index, const POINT& position, BYTE* back_buffer, size_t width, size_t height, size_t pitch, int len = 100);
	void DrawSprite_color(unsigned int sprite_index, const POINT& position, BYTE* back_buffer, size_t width, size_t height, size_t pitch, DWORD color);
	void DrawSprite_red(unsigned int sprite_index, const POINT& position, BYTE* back_buffer, size_t width, size_t height, size_t pitch);
	void DrawSprite_redtone(unsigned int sprite_index, const POINT& position, BYTE* back_buffer, size_t width, size_t height, size_t pitch);

private:
	_SPRITE* _sprites;
	int _max_sprite;
	DWORD _color_key;

private:
	SpriteDib();
	~SpriteDib();

private:
	static SpriteDib s_instance;

public:
	static SpriteDib* GetInstance()
	{
		return &s_instance;
	}
};