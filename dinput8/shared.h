#pragma once

class I3DEngine
{
public:
	byte padding[0xE8];
	float framerate;
	byte padding2[0xE8];
	int viewWidth;
	int viewHeight;
};
