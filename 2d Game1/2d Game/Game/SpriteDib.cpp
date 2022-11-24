#include "framework.h"
#include "SpriteDib.h"
#include "../Camera.h"

SpriteDib SpriteDib::s_instance;

SpriteDib::SpriteDib()
{
}

SpriteDib::~SpriteDib()
{
}

bool SpriteDib::Initiailze(size_t max_sprite_count, DWORD color_key)
{
	_max_sprite = max_sprite_count;
	_sprites = new _SPRITE[max_sprite_count];
	memset(_sprites, 0, sizeof(_SPRITE) * max_sprite_count);

	_color_key = color_key;

	return true;
}

void SpriteDib::LoadSprite(const wchar_t* sprite_path, unsigned int sprite_index, const POINT& center_position)
{
	if (_sprites == nullptr || sprite_index >= _max_sprite || _sprites[sprite_index]._buffer != nullptr)
	{
		//error
		return;
	}

	FILE* sprite_file;
	errno_t error_code = _wfopen_s(&sprite_file, sprite_path, L"rb");
	if (error_code != 0)
	{
		//error
		return;
	}
			
	BITMAPFILEHEADER bitmap_header;
	BITMAPINFOHEADER bitmap_info_header;

	fread(&bitmap_header, sizeof(BITMAPFILEHEADER), 1, sprite_file);
	fread(&bitmap_info_header, sizeof(BITMAPINFOHEADER), 1, sprite_file);

	_SPRITE* sprite = &_sprites[sprite_index];
	sprite->_center_position = center_position;
	sprite->_width = bitmap_info_header.biWidth;
	sprite->_height = bitmap_info_header.biHeight;
	sprite->_pitch = (sprite->_width * sizeof(DWORD) + 3) & ~3;
	size_t buffer_size = sprite->_pitch * sprite->_height;
	sprite->_buffer = new BYTE[buffer_size];

	BYTE* tmp_buffer = new BYTE[buffer_size];

	fread(tmp_buffer, buffer_size, 1, sprite_file);
	for (int iHeight = 0; iHeight < sprite->_height; iHeight++)
	{
		memcpy(sprite->_buffer + (sprite->_pitch * iHeight), tmp_buffer + (sprite->_pitch * (sprite->_height - iHeight - 1)), sprite->_pitch);
	}

	delete[] tmp_buffer;

	fclose(sprite_file);
}

void SpriteDib::ReleaseSprite(unsigned int sprite_index)
{
	if (_sprites == nullptr || sprite_index >= _max_sprite || _sprites[sprite_index]._buffer == nullptr)
	{
		//error
		return;
	}

	delete[] _sprites[sprite_index]._buffer;
	memset(&_sprites[sprite_index], 0, sizeof(_SPRITE));
}

void SpriteDib::ReleaseAll()
{
	for (int index = 0; index < _max_sprite; index++)
	{
		ReleaseSprite(index);
	}
}

void SpriteDib::DrawSprite(unsigned int sprite_index, const POINT& position, BYTE* back_buffer, size_t width, size_t height, size_t pitch, int len)
{
	if (sprite_index >= _max_sprite || back_buffer == nullptr)
	{
		//error
		return;
	}

	const _SPRITE* sprite = &_sprites[sprite_index];

	if (len < 0) len = 0;
	int sprite_width = sprite->_width * len / 100;
	int sprite_height = sprite->_height;

	DWORD* screen = (DWORD*)back_buffer;
	DWORD* sprite_buffer = (DWORD*)sprite->_buffer;

	POINT cameraPos = Camera::GetInstance()->GetPosition();

	int draw_x = position.x - sprite->_center_position.x - cameraPos.x;
	int draw_y = position.y - sprite->_center_position.y - cameraPos.y;

	if (0 > draw_y)
	{
		sprite_height = sprite_height - (-draw_y);
		sprite_buffer = (DWORD*)(sprite->_buffer + sprite->_pitch * (-draw_y));

		draw_y = 0;
	}

	if (draw_y + sprite->_height >= height)
	{
		sprite_height -= ((draw_y + sprite->_height) - height);
	}

	if (0 > draw_x)
	{
		sprite_width = sprite_width - (-draw_x);
		sprite_buffer += (-draw_x);

		draw_x = 0;
	}

	if (width <= draw_x + sprite->_width)
	{
		sprite_width -= ((draw_x + sprite->_width) - width);
	}

	if (sprite_width <= 0 || sprite_height <= 0)
	{
		return;
	}

	screen = (DWORD*)((BYTE*)(screen + draw_x) + (draw_y * pitch));

	BYTE* screen_origin = (BYTE*)screen;
	BYTE* sprite_origin = (BYTE*)sprite_buffer;

	for (int y = 0; y < sprite_height; y++)
	{
		for (int x = 0; x < sprite_width; x++)
		{
			if (_color_key != (*sprite_buffer & 0x00ffffff))
			{
				*screen = *sprite_buffer;
			}

			screen++;
			sprite_buffer++;
		}

		screen_origin += pitch;
		sprite_origin += sprite->_pitch;

		screen = (DWORD*)screen_origin;
		sprite_buffer = (DWORD*)sprite_origin;
	}
}

void SpriteDib::DrawSprite_BK(unsigned int sprite_index, const POINT& position, BYTE* back_buffer, size_t width, size_t height, size_t pitch, int len)
{
	if (sprite_index >= _max_sprite || back_buffer == nullptr)
	{
		//error
		return;
	}

	const _SPRITE* sprite = &_sprites[sprite_index];

	if (len < 0) len = 0;
	int sprite_width = sprite->_width * len / 100;
	int sprite_height = sprite->_height;

	DWORD* screen = (DWORD*)back_buffer;
	DWORD* sprite_buffer = (DWORD*)sprite->_buffer;

	int draw_x = position.x - sprite->_center_position.x;
	int draw_y = position.y - sprite->_center_position.y;

	if (0 > draw_y)
	{
		sprite_height = sprite_height - (-draw_y);
		sprite_buffer = (DWORD*)(sprite->_buffer + sprite->_pitch * (-draw_y));

		draw_y = 0;
	}

	if (draw_y + sprite->_height >= height)
	{
		sprite_height -= ((draw_y + sprite->_height) - height);
	}

	if (0 > draw_x)
	{
		sprite_width = sprite_width - (-draw_x);
		sprite_buffer += (-draw_x);

		draw_x = 0;
	}

	if (width <= draw_x + sprite->_width)
	{
		sprite_width -= ((draw_x + sprite->_width) - width);
	}

	if (sprite_width <= 0 || sprite_height <= 0)
	{
		return;
	}

	screen = (DWORD*)((BYTE*)(screen + draw_x) + (draw_y * pitch));

	BYTE* screen_origin = (BYTE*)screen;
	BYTE* sprite_origin = (BYTE*)sprite_buffer;

	for (int y = 0; y < sprite_height; y++)
	{
		for (int x = 0; x < sprite_width; x++)
		{
			*screen = *sprite_buffer;

			screen++;
			sprite_buffer++;
		}

		screen_origin += pitch;
		sprite_origin += sprite->_pitch;

		screen = (DWORD*)screen_origin;
		sprite_buffer = (DWORD*)sprite_origin;
	}
}

void SpriteDib::DrawSprite_redtone(unsigned int sprite_index, const POINT& position, BYTE* back_buffer, size_t width, size_t height, size_t pitch)
{
	if (sprite_index >= _max_sprite || back_buffer == nullptr)
	{
		//error
		return;
	}

	const _SPRITE* sprite = &_sprites[sprite_index];

	int sprite_width = sprite->_width * 100 / 100;
	int sprite_height = sprite->_height;

	DWORD* screen = (DWORD*)back_buffer;
	DWORD* sprite_buffer = (DWORD*)sprite->_buffer;

	POINT cameraPos = Camera::GetInstance()->GetPosition();

	int draw_x = position.x - sprite->_center_position.x - cameraPos.x;
	int draw_y = position.y - sprite->_center_position.y - cameraPos.y;

	if (0 > draw_y)
	{
		sprite_height = sprite_height - (-draw_y);
		sprite_buffer = (DWORD*)(sprite->_buffer + sprite->_pitch * (-draw_y));

		draw_y = 0;
	}

	if (draw_y + sprite->_height >= height)
	{
		sprite_height -= ((draw_y + sprite->_height) - height);
	}

	if (0 > draw_x)
	{
		sprite_width = sprite_width - (-draw_x);
		sprite_buffer += (-draw_x);

		draw_x = 0;
	}

	if (width <= draw_x + sprite->_width)
	{
		sprite_width -= ((draw_x + sprite->_width) - width);
	}

	if (sprite_width <= 0 || sprite_height <= 0)
	{
		return;
	}

	screen = (DWORD*)((BYTE*)(screen + draw_x) + (draw_y * pitch));

	BYTE* screen_origin = (BYTE*)screen;
	BYTE* sprite_origin = (BYTE*)sprite_buffer;

	for (int y = 0; y < sprite_height; y++)
	{
		for (int x = 0; x < sprite_width; x++)
		{
			if (_color_key != (*sprite_buffer & 0x00ffffff))
			{
				BYTE red = (*sprite_buffer & 0x00ff0000) >> 16;
				BYTE green = ((*sprite_buffer & 0x0000ff00) >> 8) / 2;
				BYTE blue = (*sprite_buffer & 0x000000ff) / 2;

				DWORD rgb = RGB(blue, green, red);
				*screen = rgb;
			}

			screen++;
			sprite_buffer++;
		}

		screen_origin += pitch;
		sprite_origin += sprite->_pitch;

		screen = (DWORD*)screen_origin;
		sprite_buffer = (DWORD*)sprite_origin;
	}
}

void SpriteDib::DrawSprite_color(unsigned int sprite_index, const POINT& position, BYTE* back_buffer, size_t width, size_t height, size_t pitch, DWORD color)
{
	if (sprite_index >= _max_sprite || back_buffer == nullptr)
	{
		//error
		return;
	}

	const _SPRITE* sprite = &_sprites[sprite_index];

	int sprite_width = sprite->_width;
	int sprite_height = sprite->_height;

	DWORD* screen = (DWORD*)back_buffer;
	DWORD* sprite_buffer = (DWORD*)sprite->_buffer;

	POINT cameraPos = Camera::GetInstance()->GetPosition();

	int draw_x = position.x - sprite->_center_position.x - cameraPos.x;
	int draw_y = position.y - sprite->_center_position.y - cameraPos.y;

	if (0 > draw_y)
	{
		sprite_height = sprite_height - (-draw_y);
		sprite_buffer = (DWORD*)(sprite->_buffer + sprite->_pitch * (-draw_y));

		draw_y = 0;
	}

	if (draw_y + sprite->_height >= height)
	{
		sprite_height -= ((draw_y + sprite->_height) - height);
	}

	if (0 > draw_x)
	{
		sprite_width = sprite_width - (-draw_x);
		sprite_buffer += (-draw_x);

		draw_x = 0;
	}

	if (width <= draw_x + sprite->_width)
	{
		sprite_width -= ((draw_x + sprite->_width) - width);
	}

	if (sprite_width <= 0 || sprite_height <= 0)
	{
		return;
	}

	screen = (DWORD*)((BYTE*)(screen + draw_x) + (draw_y * pitch));

	BYTE* screen_origin = (BYTE*)screen;
	BYTE* sprite_origin = (BYTE*)sprite_buffer;

	for (int y = 0; y < sprite_height; y++)
	{
		for (int x = 0; x < sprite_width; x++)
		{
			if (_color_key != (*sprite_buffer & 0x00ffffff))
			{
				*screen = color;
			}

			screen++;
			sprite_buffer++;
		}

		screen_origin += pitch;
		sprite_origin += sprite->_pitch;

		screen = (DWORD*)screen_origin;
		sprite_buffer = (DWORD*)sprite_origin;
	}
}

void SpriteDib::DrawSprite_red(unsigned int sprite_index, const POINT& position, BYTE* back_buffer, size_t width, size_t height, size_t pitch)
{
	if (sprite_index >= _max_sprite || back_buffer == nullptr)
	{
		//error
		return;
	}

	DWORD color = RGB(0, 255, 0);

	POINT cameraPos = Camera::GetInstance()->GetPosition();
	POINT pt = position;
	pt.x = position.x - 2;
	DrawSprite_color(sprite_index, pt, back_buffer, width, height, pitch, color);
	pt.x = position.x + 2;
	DrawSprite_color(sprite_index, pt, back_buffer, width, height, pitch, color);
	pt.x = position.x;
	pt.y = position.y - 2;
	DrawSprite_color(sprite_index, pt, back_buffer, width, height, pitch, color);
	pt.y = position.y + 2;
	DrawSprite_color(sprite_index, pt, back_buffer, width, height, pitch, color);

	const _SPRITE* sprite = &_sprites[sprite_index];

	int sprite_width = sprite->_width;
	int sprite_height = sprite->_height;

	DWORD* screen = (DWORD*)back_buffer;
	DWORD* sprite_buffer = (DWORD*)sprite->_buffer;

	int draw_x = position.x - sprite->_center_position.x - cameraPos.x;
	int draw_y = position.y - sprite->_center_position.y - cameraPos.y;

	if (0 > draw_y)
	{
		sprite_height = sprite_height - (-draw_y);
		sprite_buffer = (DWORD*)(sprite->_buffer + sprite->_pitch * (-draw_y));

		draw_y = 0;
	}

	if (draw_y + sprite->_height >= height)
	{
		sprite_height -= ((draw_y + sprite->_height) - height);
	}

	if (0 > draw_x)
	{
		sprite_width = sprite_width - (-draw_x);
		sprite_buffer += (-draw_x);

		draw_x = 0;
	}

	if (width <= draw_x + sprite->_width)
	{
		sprite_width -= ((draw_x + sprite->_width) - width);
	}

	if (sprite_width <= 0 || sprite_height <= 0)
	{
		return;
	}

	screen = (DWORD*)((BYTE*)(screen + draw_x) + (draw_y * pitch));

	BYTE* screen_origin = (BYTE*)screen;
	BYTE* sprite_origin = (BYTE*)sprite_buffer;

	for (int y = 0; y < sprite_height; y++)
	{
		for (int x = 0; x < sprite_width; x++)
		{
			if (_color_key != (*sprite_buffer & 0x00ffffff))
			{								
				*screen = *sprite_buffer;
			}

			screen++;
			sprite_buffer++;
		}

		screen_origin += pitch;
		sprite_origin += sprite->_pitch;

		screen = (DWORD*)screen_origin;
		sprite_buffer = (DWORD*)sprite_origin;
	}
}
