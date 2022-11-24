#pragma once

class ScreenDib
{
public:
	bool Initialize(size_t width, size_t height, int color_bit);
	void Release();
	void Flip(HWND hWnd);

	inline BYTE* GetBuffer() const { return _buffer; }
	inline size_t GetWidth() const { return _width; }
	inline size_t GetHeight() const { return _height; }
	inline size_t GetPitch() const { return _pitch; }
	inline size_t GetBufferSize() const { return _buffer_size; }

private:
	BYTE* _buffer;
	BITMAPINFOHEADER _bitmap_info_header;
	size_t _width;
	size_t _height;
	size_t _pitch;
	size_t _buffer_size;

private:
	ScreenDib();
	~ScreenDib();

private:
	static ScreenDib s_instance;

public:
	static ScreenDib* GetInstance()
	{
		return &s_instance;
	}
};

