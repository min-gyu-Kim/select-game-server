#include "framework.h"
#include "ScreenDib.h"

ScreenDib ScreenDib::s_instance;

ScreenDib::ScreenDib()
{
}

ScreenDib::~ScreenDib()
{
	Release();
}

bool ScreenDib::Initialize(size_t width, size_t height, int color_bit)
{
	if (width <= 0 || height <= 0 || color_bit <= 0)
	{
		return false;
	}

	_width = width;
	_height = height;
	_pitch = (width * (color_bit / 8) + 3) & ~3;
	_buffer_size = _pitch * _height;

	_bitmap_info_header.biBitCount = color_bit;
	_bitmap_info_header.biClrImportant = 0;
	_bitmap_info_header.biClrUsed = 0;
	_bitmap_info_header.biCompression = 0;
	_bitmap_info_header.biHeight = -1 * _height;
	_bitmap_info_header.biPlanes = 1;
	_bitmap_info_header.biSize = sizeof(BITMAPINFOHEADER);
	_bitmap_info_header.biSizeImage = _buffer_size;
	_bitmap_info_header.biWidth = _width;
	_bitmap_info_header.biXPelsPerMeter = 0;
	_bitmap_info_header.biYPelsPerMeter = 0;

	_buffer = new BYTE[_buffer_size];
	memset(_buffer, 0, _buffer_size);

	return true;
}

void ScreenDib::Release()
{
	delete[] _buffer;
}

void ScreenDib::Flip(HWND hWnd)
{
	if (_buffer == nullptr)
	{
		return;
	}

	HDC hdc = GetDC(hWnd);

	StretchDIBits(hdc, 0, 0, _width, _height, 0, 0, _width, _height, _buffer, (BITMAPINFO*)&_bitmap_info_header, DIB_RGB_COLORS, SRCCOPY);

	ReleaseDC(hWnd, hdc);
}
