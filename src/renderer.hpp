#pragma once

#include <GL/glew.h>

class Renderer {
private:
	GLuint gBuffer;
	GLuint gPosition, gNormal, gAlbedoSpec, gDepth;

	void DeletegBuffer();

public:
	void Init();
	void CreategBuffer();
	void Resize();

	void useGeometryPass();
	void useLightingPass();
};
