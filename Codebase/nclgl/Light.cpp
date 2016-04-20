#include "Light.h"



Light::Light(Mesh* mesh, bool needsCleanup)
{
	m_Cleanup = needsCleanup;
	m_Mesh = mesh;
}


Light::~Light()
{
	if (m_Cleanup)
	{
		delete m_Mesh;
	}
}
