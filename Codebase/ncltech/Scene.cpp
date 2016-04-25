#include "Scene.h"
#include "CommonMeshes.h"
#include "NCLDebug.h"
#include <algorithm>
#include <mutex>


constexpr GLuint SHADOW_WIDTH = 1024 * 2, SHADOW_HEIGHT = 1024 * 2;


Scene::Scene(Window& window) : OGLRenderer(window) {
	CommonMeshes::InitializeMeshes();

	m_DebugShader = new Shader(SHADERDIR"debugVertex.glsl", SHADERDIR"debugFragment.glsl");
	if (!m_DebugShader->IsOperational())
		return;

	m_ShadowDepthCubemapShader = new Shader(SHADERDIR"DepthCubemapVert.glsl", SHADERDIR"DepthCubemapFrag.glsl", SHADERDIR"DepthCubemapGeo.glsl");
	assert(m_ShadowDepthCubemapShader->IsOperational());

	m_SceneShader = new Shader(SHADERDIR"TestVert.glsl", SHADERDIR"TestFrag.glsl");
	assert(m_SceneShader->IsOperational());

	m_Camera = new Camera();
	m_RootGameObject = new GameObject();
	m_RootGameObject->m_Scene = this;

	m_AmbientColour = Vec3Physics(0.2f, 0.2f, 0.2f);
	m_SpecularIntensity = 128.0f;

	m_ScreenDTex = NULL;
	m_ScreenCTex = NULL;
	memset(m_FBOs, 0, sizeof(m_FBOs));

	m_DepthCubemap = NULL;

	//Generate our Framebuffers
	glGenFramebuffers(sizeof(m_FBOs) / sizeof(GLuint), m_FBOs);

	BuildScreenFBO();

	BuildLightFBO();

	glClearColor(0.6f, 0.6f, 0.6f, 1.f);

	NCLDebug::LoadShaders();

	init = true;
	m_EndScene = false;

	projMatrix = Mat4Graphics::Perspective(0.01f, 1000.0f, (float)width / (float)height, 45.0f);

	m_Light = new Light(CommonMeshes::Sphere(), false);
}

Scene::~Scene() {
	CommonMeshes::ReleaseMeshes();

	if (m_Camera) {
		delete m_Camera;
		m_Camera = NULL;
	}

	if (m_RootGameObject) {
		delete m_RootGameObject;
		m_RootGameObject = NULL;
	}

	if (m_ShadowDepthCubemapShader)
	{
		delete m_ShadowDepthCubemapShader;
		m_ShadowDepthCubemapShader = NULL;
	}

	if (m_SceneShader)
	{
		delete m_SceneShader;
		m_SceneShader = NULL;
	}

	if (m_Light)
	{
		delete m_Light;
		m_Light = NULL;
	}

	glDeleteTextures(1, &m_ScreenDTex);
	glDeleteTextures(1, &m_ScreenCTex);
	glDeleteTextures(1, &m_DepthCubemap);

	glDeleteFramebuffers(sizeof(m_FBOs) / sizeof(GLuint), m_FBOs);

	NCLDebug::ReleaseShaders();
}


void Scene::AddGameObject(GameObject* game_object) {
	m_RootGameObject->AddChildObject(game_object);
	game_object->SetScene(this);
	if (game_object->GetChildren().size() > 0)
		for (GameObject* child : game_object->GetChildren())
			AddGameObject(child);
}

GameObject* Scene::FindGameObject(const std::string& name) {
	return m_RootGameObject->FindGameObject(name);
}

//int frame_idx = 0;
void Scene::RenderScene() {
	//Check to see if the window has been resized
	if (m_ScreenTexWidth != width || m_ScreenTexHeight != height)
	{
		BuildScreenFBO();
	}

	RenderLightMaps();

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	// 2. then render scene as normal with shadow mapping (using depth cubemap)
	glBindFramebuffer(GL_FRAMEBUFFER, m_ScreenFBO);
	glViewport(0, 0, m_ScreenTexWidth, m_ScreenTexHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	SetCurrentShader(m_SceneShader);
	viewMatrix = m_Camera->BuildViewMatrix();
	m_FrameFrustum.FromMatrix(projMatrix * viewMatrix);
	UpdateShaderMatrices();
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_DepthCubemap);

	UpdateWorldMatrices(m_RootGameObject, Mat4Physics::IDENTITY);
	BuildNodeLists(m_RootGameObject);
	SortNodeLists();

	DrawNodes();
	DrawTransparentNodes();

	Mat4Graphics lightTransform = Mat4Graphics::Translation(m_Light->position);// *Mat4Graphics::Scale(Vec3Graphics(m_Light->scale, m_Light->scale, m_Light->scale));

	glUniformMatrix4fv(glGetUniformLocation(currentShader->GetProgram(), "modelMatrix"), 1, false, (float*)&lightTransform);
	glUniform3fv(glGetUniformLocation(currentShader->GetProgram(), "lightPos"), 1, (float*)&m_Light->position);
	glUniform1f(glGetUniformLocation(currentShader->GetProgram(), "far_plane"), m_Light->scale);
	glUniform3fv(glGetUniformLocation(currentShader->GetProgram(), "viewPos"), 1, (float*)&m_Camera->GetPosition());
	m_Light->mesh->Draw();



	/*
	//Setup Default Shader Uniforms
	Vec3Physics camPos = m_Camera->GetPosition();
	Vec3Physics lightPos;
	Vec4Physics lightPosEyeSpace = viewMatrix * Vec4Physics(lightPos.x, lightPos.y, lightPos.z, 1.0f);

	SetCurrentShader(m_SceneShader);
	UpdateShaderMatrices();
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "diffuseTex"), 0);
	glUniform3fv(glGetUniformLocation(currentShader->GetProgram(), "ambientColour"), 1, &m_AmbientColour.x);
	//glUniform3fv(glGetUniformLocation(currentShader->GetProgram(), "invLightDir"), 1, &m_InvLightDirection.x);
	glUniform3fv(glGetUniformLocation(currentShader->GetProgram(), "cameraPos"), 1, &camPos.x);
	glUniform1f(glGetUniformLocation(currentShader->GetProgram(), "specularIntensity"), m_SpecularIntensity);


	//Setup Render FBO/OpenGL States
	glBindFramebuffer(GL_FRAMEBUFFER, m_ScreenFBO);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	//glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glDepthFunc(GL_LEQUAL);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	
	//Finally Render the Light Sections of the scene where the shadow volumes overlapped
	glDepthFunc(GL_LEQUAL);
	//glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	DrawNodes();
	DrawTransparentNodes();
	*/


	//Clear Render List
	ClearNodeLists();


	//Finally draw all debug data to FBO (this fbo has anti-aliasing and the screen buffer does not)
	RenderDebug();
}

void Scene::Present() {
	//Present our Screen
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	PresentScreenFBO();

	//Swap Buffers and get ready to repeat the process
	glUseProgram(0);
	SwapBuffers();
}

void Scene::RenderDebug() {
	NCLDebug::SetDebugDrawData(projMatrix * viewMatrix, m_Camera->GetPosition());
	glDisable(GL_DEPTH_TEST);

	NCLDebug::SortDebugLists();
	NCLDebug::DrawDebugLists();
	NCLDebug::ClearDebugLists();
}

void Scene::UpdateScene(float dt) {
	m_Camera->UpdateCamera(dt * 1000.f);
	m_Light->UpdateLight(dt * 1000.0f);
}


void Scene::BuildScreenFBO() {
	m_ScreenTexWidth = width;
	m_ScreenTexHeight = height;

	//Generate our Scene Depth Texture
	if (!m_ScreenDTex) glGenTextures(1, &m_ScreenDTex);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_ScreenDTex);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 2, GL_DEPTH_COMPONENT32, width, height, GL_FALSE);

	//Generate our Scene Colour Texture
	if (!m_ScreenCTex) glGenTextures(1, &m_ScreenCTex);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_ScreenCTex);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB,)
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 2, GL_RGBA8, width, height, GL_FALSE);

	
	glBindFramebuffer(GL_FRAMEBUFFER, m_ScreenFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, m_ScreenDTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, m_ScreenCTex, 0);

	//Validate our framebuffer
	GLuint status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE
	    || !m_ScreenDTex
	    || !m_ScreenCTex) {
		//GL_FRAMEBUFFER_INC
		cout << "Error: Unable to create screen framebuffer object.";
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Scene::BuildLightFBO()
{
	glGenTextures(1, &m_DepthCubemap);

	glBindTexture(GL_TEXTURE_CUBE_MAP, m_DepthCubemap);
	for (GLuint i = 0; i < 6; ++i)
	{
		glTexImage2D(
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			0,
			GL_DEPTH_COMPONENT,
			SHADOW_WIDTH,
			SHADOW_HEIGHT,
			0,
			GL_DEPTH_COMPONENT,
			GL_FLOAT,
			NULL
			);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glBindFramebuffer(GL_FRAMEBUFFER, m_DepthMapFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_DepthCubemap, 0);
	//this framebuffer object does not render to a color buffer.
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	GLuint status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	assert(status == GL_FRAMEBUFFER_COMPLETE && m_DepthCubemap);
}

void Scene::RenderLightMaps()
{
	// 1. first render to depth cubemap
	glBindFramebuffer(GL_FRAMEBUFFER, m_DepthMapFBO);
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	SetCurrentShader(m_ShadowDepthCubemapShader);
	//camera view and proj are not valid
	//UpdateShaderMatrices();

	float aspect = (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT;
	float nearPlane = 1.0f;
	float farPlane = m_Light->scale;
	Mat4Graphics shadowProj = Mat4Graphics::Perspective(nearPlane, farPlane, aspect, 90.0f);

	Mat4Graphics shadowTransforms[6];

	Vec3Graphics lightPos = m_Light->position;

	shadowTransforms[0] = shadowProj *
		Mat4Graphics::View(lightPos, lightPos + Vec3Graphics(1.0f, 0.0f, 0.0f), Vec3Graphics(0.0, -1.0, 0.0));

	shadowTransforms[1] = shadowProj *
		Mat4Graphics::View(lightPos, lightPos + Vec3Graphics(-1.0f, 0.0f, 0.0f), Vec3Graphics(0.0, -1.0, 0.0));

	shadowTransforms[2] = shadowProj *
		Mat4Graphics::View(lightPos, lightPos + Vec3Graphics(0.0f, 1.0f, 0.0f), Vec3Graphics(0.0, 0.0, 1.0));

	shadowTransforms[3] = shadowProj *
		Mat4Graphics::View(lightPos, lightPos + Vec3Graphics(0.0f, -1.0f, 0.0f), Vec3Graphics(0.0, 0.0, -1.0));

	shadowTransforms[4] = shadowProj *
		Mat4Graphics::View(lightPos, lightPos + Vec3Graphics(0.0f, 0.0f, 1.0f), Vec3Graphics(0.0, -1.0, 0.0));

	shadowTransforms[5] = shadowProj *
		Mat4Graphics::View(lightPos, lightPos + Vec3Graphics(0.0f, 0.0f, -1.0f), Vec3Graphics(0.0, -1.0, 0.0));

	glUniformMatrix4fv(glGetUniformLocation(currentShader->GetProgram(), "shadowMatrices"), 6, false, (float*)&shadowTransforms);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_DepthCubemap);

	//Send uniforms !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	DrawAllNodes();
}

void Scene::PresentScreenFBO() {
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_ScreenFBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

void Scene::UpdateWorldMatrices(GameObject* cNode, const Mat4Graphics& parentWM) {
	cNode->m_WorldTransform = parentWM * cNode->m_LocalTransform;

	for (auto child : cNode->GetChildren())
		UpdateWorldMatrices(child, cNode->m_WorldTransform);
}

void Scene::BuildNodeLists(GameObject* cNode) {
	Vec3Physics obj_pos = cNode->m_WorldTransform.GetTranslation();

	Vec3Physics direction = obj_pos - m_Camera->GetPosition();

	FrustrumSortingObject fso;
	fso.camera_distance = direction.Dot(direction);
	fso.target			= cNode;

	if (m_FrameFrustum.InsideFrustum(obj_pos, cNode->GetBoundingRadius())) {
		if (cNode->GetColour().w < 1.0f)
			m_TransparentNodeList.push_back(fso);
		else
			m_NodeList.push_back(fso);
	}

	for (auto child : cNode->GetChildren())
		BuildNodeLists(child);
}


void Scene::SortNodeLists() {
	std::sort(m_TransparentNodeList.begin(), m_TransparentNodeList.end(), FrustrumSortingObject::CompareByCameraDistanceInv);
	std::sort(m_NodeList.begin(), m_NodeList.end(), FrustrumSortingObject::CompareByCameraDistance);
}

void Scene::ClearNodeLists() {
	m_TransparentNodeList.clear();
	m_NodeList.clear();
}

void Scene::DrawNodes() {
	for (auto node : m_NodeList)
		DrawNode(node.target);
}

void Scene::DrawTransparentNodes()
{
	for (auto node : m_TransparentNodeList)
	{
		DrawNode(node.target);
	}
}

void Scene::DrawNode(GameObject* n) {
	glUniformMatrix4fv(glGetUniformLocation(currentShader->GetProgram(), "modelMatrix"), 1, false, (float*) & (n->m_WorldTransform * Mat4Physics::Scale(n->GetScale())));
	glUniform3fv(glGetUniformLocation(currentShader->GetProgram(), "lightPos"), 1, (float*)&m_Light->position);
	glUniform3fv(glGetUniformLocation(currentShader->GetProgram(), "viewPos"), 1, (float*)&m_Camera->GetPosition());
	glUniform1f(glGetUniformLocation(currentShader->GetProgram(), "far_plane"), m_Light->scale);
	
	n->OnRenderObject();
}

void Scene::DrawAllNodes()
{
	if (m_RootGameObject)
	{
		DrawAllNodes(m_RootGameObject);
	}
}

void Scene::DrawAllNodes(GameObject* n)
{
	DrawNode(n);
	for (auto child : n->GetChildren())
	{
		DrawAllNodes(child);
	}
}
