#include "CommonMeshes.h"
#include <nclgl\OBJMesh.h>

Mesh* CommonMeshes::m_pPlane	= NULL;
Mesh* CommonMeshes::m_pCube		= NULL;
Mesh* CommonMeshes::m_pSphere	= NULL;
Mesh* CommonMeshes::m_pPyramid	= NULL;

GLuint CommonMeshes::m_CheckerboardTex = 0;

void CommonMeshes::InitializeMeshes()
{
	if (m_pPlane == NULL)
	{
		m_CheckerboardTex = SOIL_load_OGL_texture(TEXTUREDIR"checkerboard.tga", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);
		glBindTexture(GL_TEXTURE_2D, m_CheckerboardTex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		m_pPlane = Mesh::GenerateQuadTexCoordCol(Vec2Physics(1.f, 1.f), Vec2Physics(0.0f, 1.0f), Vec4Physics(1.0f, 1.0f, 1.0f, 1.0f));
		m_pCube = new OBJMesh(MESHDIR"cube.obj");
		m_pSphere = new OBJMesh(MESHDIR"sphere.obj");
		m_pPyramid= new OBJMesh(MESHDIR"pyramid.obj");

		m_pPlane->SetTexture(m_CheckerboardTex);
		m_pCube->SetTexture(m_CheckerboardTex);
		m_pSphere->SetTexture(m_CheckerboardTex);
		m_pPyramid->SetTexture(m_CheckerboardTex);
	}
}

void CommonMeshes::ReleaseMeshes()
{
	if (m_pPlane != NULL)
	{
		glDeleteTextures(1, &m_CheckerboardTex);
		delete m_pPlane;
		delete m_pCube;
		delete m_pSphere;
	}

	m_pPlane = NULL;
}