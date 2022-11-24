#pragma once
class FrameManager
{
public:
	bool Initialize();

	bool Update();

	void UpdateCount() { _update_count++; }
	void RenderCount() { _render_count++; }

	int GetUpdateFrame() const { return _update_frame; }
	int GetRenderFrame() const { return _render_frame; }

	int GetInfficient() const { return _insufficient_time; }

private:
	DWORD _prev_time;
	int _insufficient_time;
	
	DWORD _timer;
	int _update_count;
	int _render_count;

	int _update_frame;
	int _render_frame;

private:
	static constexpr DWORD FRAME_TIME = 20;

private:
	FrameManager();
	~FrameManager();

private:
	static FrameManager s_instance;

public:
	static FrameManager* GetInstance()
	{
		return &s_instance;
	}
};

