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

	virtual void	Cleanup() {}
	void			Present();
protected:
	void	BuildScreenFBO();

	void	BuildLightFBO();

	void	RenderLightMaps();

	void	PresentScreenFBO();

	void	UpdateWorldMatrices(GameObject* node, const Mat4Graphics& parentWM);

	void	BuildNodeLists(GameObject* node);
	void	SortNodeLists();
	void	ClearNodeLists();
	void	DrawNodes();

	void	DrawAllNodes(GameObject* n);
	void	DrawAllNodes();

	void	DrawTransparentNodes();

	void	DrawNode(GameObject* n);

	bool				m_EndScene;
	Camera*				m_Camera;
	Shader*				m_DebugShader;
	Shader*				m_ShadowDepthCubemapShader;
	Shader*				m_SceneShader;
	Light*				m_Light;

	GameObject*			m_RootGameObject;

	Frustum				m_FrameFrustum;
	vector<FrustrumSortingObject> m_TransparentNodeList;
	vector<FrustrumSortingObject> m_NodeList;

	GLuint	m_ScreenTexWidth, m_ScreenTexHeight;
	GLuint  m_ScreenDTex, m_ScreenCTex;
	
	union {
		struct {
			GLuint m_ScreenFBO;
			GLuint m_DepthMapFBO;
		};
		GLuint m_FBOs[2];
	};

	GLuint m_DepthCubemap;

	Vec3Physics m_AmbientColour;
	float   m_SpecularIntensity;
};