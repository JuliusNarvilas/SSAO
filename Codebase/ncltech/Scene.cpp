#include "Scene.h"
#include "CommonMeshes.h"
#include "NCLDebug.h"
#include <algorithm>
#include <mutex>
#include "../Helpers/RNG.h"
#include "../Helpers/interpolation.h"

unsigned int Scene::RenderMode = 0;

const GLuint SHADOW_WIDTH = 640, SHADOW_HEIGHT = 640;


Scene::Scene(Window& window) : OGLRenderer(window) {
	CommonMeshes::InitializeMeshes();

	m_DebugShader = new Shader(SHADERDIR"debugVertex.glsl", SHADERDIR"debugFragment.glsl");
	if (!m_DebugShader->IsOperational())
		return;

	m_ShadowCubeShader = new Shader(SHADERDIR"ShadowCasting/DepthCubemapVert.glsl", SHADERDIR"ShadowCasting/DepthCubemapFrag.glsl", SHADERDIR"ShadowCasting/DepthCubemapGeo.glsl");
	assert(m_ShadowCubeShader->IsOperational());

	m_GeometryShader = new Shader(SHADERDIR"DeferredGeometryVert.glsl", SHADERDIR"DeferredGeometryFrag.glsl");
	assert(m_GeometryShader->IsOperational());

	m_LightShader = new Shader(SHADERDIR"DeferredLightVert.glsl", SHADERDIR"DeferredLightFrag.glsl");
	assert(m_LightShader->IsOperational());

	m_CombineShader = new Shader(SHADERDIR"DeferredCombineVert.glsl", SHADERDIR"DeferredCombineFrag.glsl");
	assert(m_CombineShader->IsOperational());

	m_SSAOShader = new Shader(SHADERDIR"SSAOVert.glsl", SHADERDIR"SSAOFrag.glsl");
	assert(m_SSAOShader->IsOperational());

	m_SSAOBlurShader = new Shader(SHADERDIR"SSAOBlurVert.glsl", SHADERDIR"SSAOBlurFrag.glsl");
	assert(m_SSAOBlurShader->IsOperational());

	m_Camera = new Camera();
	m_RootGameObject = new GameObject();
	m_RootGameObject->m_Scene = this;

	m_AmbientColour = Vec3Physics(0.2f, 0.2f, 0.2f);
	m_SpecularIntensity = 128.0f;

	m_ScreenTexWidth = width;
	m_ScreenTexHeight = height;

	//Generate our Framebuffers
	glGenFramebuffers(s_FBOCount, m_FBOs);
	glGenTextures(s_TextureCount, m_Textures);

	BuildGeometryPassFBO();
	BuildLightPassFBO();
	BuildFinalFBO();

	BuildShadowFBO();
	BuildSSAOFBO();
	BuildSSAOBlurFBO();

	SetCurrentShader(m_SSAOShader);
	glUniform3fv(glGetUniformLocation(currentShader->GetProgram(), "samples"), m_HemisphereSampleSize, (float*)m_HemisphereSamples);

	NCLDebug::LoadShaders();

	init = true;
	m_EndScene = false;

	m_Light = new Light(CommonMeshes::Sphere(), false);

	m_ScreenQuad = Mesh::GenerateQuad();

	m_OrthoProj = Mat4Graphics::Orthographic(-1, 1, 1, -1, -1, 1);
	projMatrix = Mat4Graphics::Perspective(0.01f, 1000.0f, float(m_ScreenTexWidth) / float(m_ScreenTexHeight), 45.0f);

	wglSwapIntervalEXT(0);
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

	if (m_ShadowCubeShader)
	{
		delete m_ShadowCubeShader;
		m_ShadowCubeShader = NULL;
	}

	if (m_GeometryShader)
	{
		delete m_GeometryShader;
		m_GeometryShader = NULL;
	}

	if (m_Light)
	{
		delete m_Light;
		m_Light = NULL;
	}

	if (m_ScreenQuad)
	{
		delete m_ScreenQuad;
		m_ScreenQuad = NULL;
	}

	glDeleteTextures(s_TextureCount, m_Textures);
	glDeleteFramebuffers(s_FBOCount, m_FBOs);

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
	/*if (m_ScreenTexWidth != width || m_ScreenTexHeight != height)
	{
		BuildGeometryPassFBO();
	}*/

	bool enableSSAO = true;
	bool enableSSAOBlur = false;
	bool enableShadowCasting = false;
	bool enableShadows = false;
	//////////////////////////////////////////////////////////////
	//Shadow casting pass

	if (enableShadowCasting)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_ShadowFBO);
		glClear(GL_DEPTH_BUFFER_BIT);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);

		RenderLightMaps();
	}

	//////////////////////////////////////////////////////////////
	//Geometry pass

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glDisable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glBindFramebuffer(GL_FRAMEBUFFER, m_GeometryPassFBO);
	glViewport(0, 0, m_ScreenTexWidth, m_ScreenTexHeight);
	glClearColor(0.0f, 0.0f, 0.0f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	SetCurrentShader(m_GeometryShader);
	viewMatrix = m_Camera->BuildViewMatrix();
	UpdateShaderMatrices();

	UpdateWorldMatrices(m_RootGameObject, Mat4Physics::IDENTITY);
	
	DrawAllNodes();

	Mat4Graphics fakeLightTransform = Mat4Graphics::Translation(m_Light->position) * Mat4Graphics::Scale(Vec3Graphics(0.5, 0.5, 0.5));
	glUniformMatrix4fv(glGetUniformLocation(currentShader->GetProgram(), "modelMatrix"), 1, false, (float*)&fakeLightTransform);
	glUniform3fv(glGetUniformLocation(currentShader->GetProgram(), "lightPos"), 1, (float*)&m_Light->position);
	
	m_Light->mesh->Draw();

	//////////////////////////////////////////////////////////////////////////////////
	//SSAO pass

	glBindFramebuffer(GL_FRAMEBUFFER, m_SSAOFBO);

	if (enableSSAO)
	{
		glDisable(GL_CULL_FACE);
		glDisable(GL_BLEND);

		//dont really need to clear as the whole thing is going to be rendered on top with no blending
		//glClearColor(0.0f, 0.0f, 0.0f, 1.0f);//???
		//glClear(GL_COLOR_BUFFER_BIT);

		SetCurrentShader(m_SSAOShader);
		UpdateShaderMatrices();

		glUniform2f(glGetUniformLocation(currentShader->GetProgram(), "pixelSize"), 1.0f / m_ScreenTexWidth, 1.0f / m_ScreenTexHeight);
		glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "kernelSize"), m_HemisphereSampleSize);
		glUniformMatrix4fv(glGetUniformLocation(currentShader->GetProgram(), "orthoProjMatrix"), 1, false, (float*)&m_OrthoProj);

		glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "depthTex"), 2);
		glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "normalTex"), 3);
		glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "noiseTex"), 4);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, m_GeometryDepthTex);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, m_GeometryNormalTex);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, m_SSAONoiseTex);

		m_ScreenQuad->Draw();
	}
	else
	{
		glClearColor(1.0, 1.0, 1.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);
	}
	/////////////////////////////////////////////////////////////////////////////////////
	//Blur pass
	if (enableSSAOBlur)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_SSAOBlurFBO);
		/*glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);*/

		SetCurrentShader(m_SSAOBlurShader);
		//UpdateShaderMatrices();

		glUniform2f(glGetUniformLocation(currentShader->GetProgram(), "pixelSize"), 1.0f / m_ScreenTexWidth, 1.0f / m_ScreenTexHeight);
		glUniformMatrix4fv(glGetUniformLocation(currentShader->GetProgram(), "orthoProjMatrix"), 1, false, (float*)&m_OrthoProj);

		glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "ssaoTex"), 2);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, m_LightDiffuseTex);

		m_ScreenQuad->Draw();
	}
	else
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_SSAOFBO);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_SSAOBlurFBO);

		glBlitFramebuffer(0, 0, m_ScreenTexWidth, m_ScreenTexHeight, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	}
	////////////////////////////////////////////////////////////////////////////////////
	//Light pass
	if (enableShadows)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_LightPassFBO);
		glClearColor(0.0f, 0.0f, 0.0f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT);
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		glDisable(GL_DEPTH_TEST);

		SetCurrentShader(m_LightShader);
		UpdateShaderMatrices();
		glUniform2f(glGetUniformLocation(currentShader->GetProgram(), "pixelSize"), 1.0f / m_ScreenTexWidth, 1.0f / m_ScreenTexHeight);
		Mat4Graphics lightTransform = Mat4Graphics::Translation(m_Light->position) * Mat4Graphics::Scale(Vec3Graphics(m_Light->scale, m_Light->scale, m_Light->scale));

		//glUniformMatrix4fv(glGetUniformLocation(currentShader->GetProgram(), "depthMap"), 1, false, ???);
		glUniformMatrix4fv(glGetUniformLocation(currentShader->GetProgram(), "modelMatrix"), 1, false, (float*)&lightTransform);
		glUniform3fv(glGetUniformLocation(currentShader->GetProgram(), "lightPos"), 1, (float*)&m_Light->position);
		glUniform1f(glGetUniformLocation(currentShader->GetProgram(), "lightRadius"), m_Light->scale);
		glUniform3fv(glGetUniformLocation(currentShader->GetProgram(), "cameraPos"), 1, (float*)&m_Camera->GetPosition());
		glUniform3fv(glGetUniformLocation(currentShader->GetProgram(), "lightColour"), 1, (float*)&Vec3Graphics::ONES);

		glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "depthTex"), 2);
		glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "normTex"), 3);
		glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "depthMap"), 4);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, m_GeometryDepthTex);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, m_GeometryNormalTex);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_LightDepthCubeTex);

		m_Light->mesh->Draw();
	}
	else
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_LightPassFBO);
		glClearColor(0.3f, 0.3f, 0.3f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT);
	}
	/////////////////////////////////////////////////////////////////
	//Combine pass

	glBindFramebuffer(GL_FRAMEBUFFER, m_FinalFBO);
	glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);

	SetCurrentShader(m_CombineShader);
	glUniformMatrix4fv(glGetUniformLocation(currentShader->GetProgram(), "orthoProjMatrix"), 1, false, (float*)&m_OrthoProj);

	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "diffuseTex"), 2);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "emissiveTex"), 3);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "specularTex"), 4);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "ssaoTex"), 5);
	
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_GeometryColourTex);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, m_LightDiffuseTex);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, m_LightSpecularTex);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, m_SSAOTintTex);

	m_ScreenQuad->Draw();

	//Finally draw all debug data to FBO (this fbo has anti-aliasing and the screen buffer does not)
	RenderDebug();
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



void Scene::RenderLightMaps()
{
	// 1. first render to depth cubemap

	SetCurrentShader(m_ShadowCubeShader);
	//camera view and proj are not valid
	//UpdateShaderMatrices();

	float aspect = float(SHADOW_WIDTH) / float(SHADOW_HEIGHT);
	Mat4Graphics shadowProj = Mat4Graphics::Perspective(0.1f, m_Light->scale, aspect, 90.0f);

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

	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_CUBE_MAP, m_LightDepthCubeTex);

	//Send uniforms !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	DrawAllNodes();
}









////////////////////////////////////////////////////////////////////////////////////
//Build FBOs
////////////////////////////////////////////////////////////////////////////////////

void Scene::BuildGeometryPassFBO() {

	/////////////////////////////////////////////////////////////////////

	glBindFramebuffer(GL_FRAMEBUFFER, m_GeometryPassFBO);

	// - Normal color buffer
	glBindTexture(GL_TEXTURE_2D, m_GeometryNormalTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, m_ScreenTexWidth, m_ScreenTexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_GeometryNormalTex, 0);

	// - Color + Specular color buffer
	glBindTexture(GL_TEXTURE_2D, m_GeometryColourTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_ScreenTexWidth, m_ScreenTexHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_GeometryColourTex, 0);

	// Then also add render buffer object as depth buffer and check for completeness.

	//Generate our Scene Depth Texture
	glBindTexture(GL_TEXTURE_2D, m_GeometryDepthTex); //glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_ScreenDTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, m_ScreenTexWidth, m_ScreenTexHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_GeometryDepthTex, 0);
	//glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 2, GL_DEPTH_COMPONENT32, width, height, GL_FALSE);

	// - Tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
	GLuint attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(3, attachments);

	/////////////////////////////////////////////////////////////////////////

	//Generate our Scene Depth Texture
	/*if (!m_ScreenDTex) glGenTextures(1, &m_ScreenDTex);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_ScreenDTex);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 2, GL_DEPTH_COMPONENT32, width, height, GL_FALSE);*/

	//Generate our Scene Colour Texture
	//if (!m_ScreenCTex) glGenTextures(1, &m_ScreenCTex);
	//glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_ScreenCTex);
	////glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB,)
	//glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 2, GL_RGBA8, width, height, GL_FALSE);

	
	/*glBindFramebuffer(GL_FRAMEBUFFER, m_ScreenFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, m_ScreenDTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, m_ScreenCTex, 0);*/

	//Validate our framebuffer
	GLuint status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		//GL_FRAMEBUFFER_INC
		cout << "Error: Unable to create screen framebuffer object.";
		assert(false);
	}

}

void Scene::BuildLightPassFBO()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_LightPassFBO);

	glBindTexture(GL_TEXTURE_2D, m_LightDiffuseTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, m_ScreenTexWidth, m_ScreenTexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_LightDiffuseTex, 0);

	glBindTexture(GL_TEXTURE_2D, m_LightSpecularTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, m_ScreenTexWidth, m_ScreenTexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_LightSpecularTex, 0);

	GLuint attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachments);

	assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
}

void Scene::BuildShadowFBO()
{
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_LightDepthCubeTex);
	for (GLuint i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT32F, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);

	glBindFramebuffer(GL_FRAMEBUFFER, m_ShadowFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_LightDepthCubeTex, 0);
	//this framebuffer object does not render to a color buffer.
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	GLuint status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	assert(status == GL_FRAMEBUFFER_COMPLETE);
}

void Scene::BuildFinalFBO()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_FinalFBO);
	/*
	glBindTexture(GL_TEXTURE_2D, m_FinalTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, m_ScreenTexWidth, m_ScreenTexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_FinalTex, 0);*/

	glBindTexture(GL_TEXTURE_2D, m_GeometryNormalTex);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_GeometryNormalTex, 0);

	GLuint attachments[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, attachments);

	assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
}

void Scene::BuildSSAOFBO()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_SSAOFBO);

	BuildSSAONoiseTex<4>();
	
	//Reuse Light texture as target to later render to the real target when performing blur
	glBindTexture(GL_TEXTURE_2D, m_LightDiffuseTex);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_LightDiffuseTex, 0);

	GLuint attachments[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, attachments);

	assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

	BuildHemisphere();
}

void Scene::BuildSSAOBlurFBO()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_SSAOBlurFBO);

	glBindTexture(GL_TEXTURE_2D, m_SSAOTintTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, m_ScreenTexWidth, m_ScreenTexHeight, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_SSAOTintTex, 0);

	GLuint attachments[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, attachments);

	assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
}


////////////////////////////////////////////////////////////////////////////////////


void Scene::BuildHemisphere()
{
	std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between 0.0 - 1.0
	std::default_random_engine generator;
	for (int i = 0; i < m_HemisphereSampleSize; ++i)
	{
		Vec3Graphics& sample = m_HemisphereSamples[i];
		sample = Vec3Graphics(
			randomFloats(generator) * 2.0f - 1.0f,
			randomFloats(generator) * 2.0f - 1.0f,
			randomFloats(generator)
		);
		sample.Normalize();
		//sample *= randomFloats(generator);
		//distribute the kernel samples closer to the origin
		float scale = float(i) / float(m_HemisphereSampleSize);
		scale = Lerp(0.1f, 1.0f, scale * scale);
		sample *= scale;
	}
}


////////////////////////////////////////////////////////////////////////////////////




void Scene::Present() {
	//Present our Screen
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	PresentScreenFBO();
	SwapBuffers();
}

void Scene::PresentScreenFBO() {
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_FinalFBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	glBlitFramebuffer(0, 0, m_ScreenTexWidth, m_ScreenTexHeight, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

void Scene::UpdateWorldMatrices(GameObject* cNode, const Mat4Graphics& parentWM) {
	cNode->m_WorldTransform = parentWM * cNode->m_LocalTransform;

	for (auto child : cNode->GetChildren())
		UpdateWorldMatrices(child, cNode->m_WorldTransform);
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
