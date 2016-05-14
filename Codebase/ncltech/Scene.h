/******************************************************************************
Class: Scene
Implements: OGLRenderer
Author:Pieran Marris <p.marris@newcastle.ac.uk>, Rich Davison <richard.davison4@newcastle.ac.uk> and YOU!
Description: For this module, you are being provided with a basic working
Renderer - to give you more time to work on your physics and AI!

It is basically the Renderer from the Graphics For Games Module as it was
by Tutorial 7 - Scene Management. It will split nodes up into those that are
opaque and transparent, and render accordingly.

The only new bits are the ability to search for Game Object's by their name,
this is not a fast function but does allow some ability to talk between objects in a more
complicated scene.


		(\_/)								-_-_-_-_-_-_-_,------,
		( '_')								_-_-_-_-_-_-_-|   /\_/\   NYANYANYAN
	 /""""""""""""\=========     -----D		-_-_-_-_-_-_-~|__( ^ .^) /
	/"""""""""""""""""""""""\				_-_-_-_-_-_-_-""  ""
....\_@____@____@____@____@_/

*//////////////////////////////////////////////////////////////////////////////
#pragma once

#include <nclgl/OGLRenderer.h>
#include <nclgl/Camera.h>
#include <nclgl/Shader.h>
#include <nclgl/Frustum.h>
#include <nclgl/Light.h>

#include "TSingleton.h"
#include "GameObject.h"


enum RenderMode : unsigned int {
	NormalRenderMode,
	DebugRenderMode,
	NormalAndDebugRenderMode,
	RenderModeMax
};
static const unsigned int RenderModeMasks[RenderModeMax] = {
	1,
	2,
	3
};


struct FrustrumSortingObject
{
	float		camera_distance;
	GameObject* target;

	static bool CompareByCameraDistance(const FrustrumSortingObject& a, const FrustrumSortingObject& b)  {
		return (a.camera_distance < b.camera_distance) ? true : false;
	}

	static bool CompareByCameraDistanceInv(const FrustrumSortingObject& a, const FrustrumSortingObject& b)  {
		return (a.camera_distance > b.camera_distance) ? true : false;
	}
};

class Scene : public OGLRenderer
{
public:

	static unsigned int	RenderMode;

	Scene(Window& window);
	~Scene();

	void AddGameObject(GameObject* game_object);	
	GameObject* FindGameObject(const std::string& name);

	virtual bool InitialiseGL()				{ return true; };
	virtual void RenderScene();
	virtual void RenderDebug();
	virtual void UpdateScene(float dt); //This is msec * 0.001f (e.g time relative to seconds not milliseconds)

	bool GetEndScene() { return m_EndScene; }
	virtual Scene* GetNextScene(Window& window) { return nullptr; }

	void			Present();
protected:
	void	BuildGeometryPassFBO();
	void	BuildLightPassFBO();
	void	BuildFinalFBO();

	void	BuildShadowFBO();

	void	RenderLightMaps();

	void	PresentScreenFBO();

	void	UpdateWorldMatrices(GameObject* node, const Mat4Graphics& parentWM);

	void	DrawAllNodes(GameObject* n);
	void	DrawAllNodes();

	void	DrawNode(GameObject* n);

	bool				m_EndScene;
	Camera*				m_Camera;
	Mat4Graphics		m_OrthoProj;

	Shader*				m_DebugShader;
	Shader*				m_ShadowCubeShader;
	Shader*				m_GeometryShader;
	Shader*				m_LightShader;
	Shader*				m_CombineShader;
	Shader*				m_SSAOShader;
	Shader*				m_SSAOBlurShader;

	Light*				m_Light;

	GameObject*			m_RootGameObject;

	vector<FrustrumSortingObject> m_TransparentNodeList;
	vector<FrustrumSortingObject> m_NodeList;

	GLuint	m_ScreenTexWidth, m_ScreenTexHeight;
	static const int s_TextureCount = 8;
	union
	{
		struct {
			GLuint m_GeometryDepthTex;
			GLuint m_GeometryColourTex;
			GLuint m_GeometryNormalTex;

			GLuint m_LightDiffuseTex;
			GLuint m_LightSpecularTex;

			//reusing m_GeometryNormalTex for the final combine step as it is no longer needed
			//GLuint m_FinalTex;

			GLuint m_LightDepthCubeTex;

			GLuint m_SSAONoiseTex;
			GLuint m_SSAOTintTex;
		};
		GLuint m_Textures[s_TextureCount];
	};
	
	static const int s_FBOCount = 6;
	union {
		struct {
			GLuint m_GeometryPassFBO;
			GLuint m_LightPassFBO;
			GLuint m_ShadowFBO;
			GLuint m_FinalFBO;
			GLuint m_SSAOFBO;
			GLuint m_SSAOBlurFBO;
		};
		GLuint m_FBOs[s_FBOCount];
	};

	Mesh*		m_ScreenQuad;

	Vec3Physics m_AmbientColour;
	float   m_SpecularIntensity;

	void BuildHemisphere();

	const int	m_HemisphereSampleSize = 16;
	Vec3Graphics m_HemisphereSamples[64];


	template<int texSize>
	void BuildSSAONoiseTex()
	{
		unsigned char temp[2 * texSize * texSize];
		std::uniform_int_distribution<int> randomFloats(0, 255); // random floats between 0.0 - 1.0
		std::default_random_engine generator;
		int count = texSize * texSize;
		for (int i = 0; i < count; i++)
		{
			unsigned char* noise = temp + (i * 2);
			noise[0] = randomFloats(generator);
			noise[1] = randomFloats(generator);
		}

		glBindTexture(GL_TEXTURE_2D, m_SSAONoiseTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG8, texSize, texSize, 0, GL_RG, GL_UNSIGNED_BYTE, temp);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	void BuildSSAOFBO();
	void BuildSSAOBlurFBO();
};