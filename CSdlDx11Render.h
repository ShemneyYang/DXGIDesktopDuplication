#pragma once

class CSdlDx11Render
{
public:
	CSdlDx11Render();
	~CSdlDx11Render();

	void init(void);

	bool createTexture(int format,int w, int h);

	bool updateTexture(const char* buffer, int size);

	void render(void);
};

