#pragma once
#include "Mesh.h"
#include "Math\nclglMath.h"

class Light
{
public:
	Light(Mesh* mesh, bool needsCleanup = false);
	~Light();

	Vec3Graphics position;
	float scale;
	Mesh* mesh;

	void UpdateLight(float msec);

private:

	bool m_Cleanup;
};

