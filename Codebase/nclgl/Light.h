#pragma once
#include "Mesh.h"
#include "Math\nclglMath.h"

class Light
{
public:
	Light(Mesh* mesh, bool needsCleanup = false);
	~Light();

	Mat4Graphics m_transform;

private:

	Mesh* m_Mesh;
	bool m_Cleanup;
};

