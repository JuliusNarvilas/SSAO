#include "MyScene.h"

#include <nclgl/OBJMesh.h>

#include <ncltech\SimpleMeshObject.h>
#include <ncltech\SphereCollisionShape.h>
#include <ncltech\CuboidCollisionShape.h>
#include <ncltech\PhysicsEngine.h>
#include <ncltech\CommonMeshes.h>
#include <ncltech\DistanceConstraint.h>
#include <ncltech\NCLDebug.h>
#include "ncltech\OctreeSpacePartition.h"
#include "ncltech\PositioningState.h"
#include "TardisGameObject.h"
#include "ncltech/PyramidCollisionShape.h"
#include "TestCases.h"


MyScene::MyScene(Window& window) : Scene(window) {
	if (init == true)
		init = InitialiseGL();

	UpdateWorldMatrices(m_RootGameObject, Mat4Physics::IDENTITY);
	m_RenderMode = NormalRenderMode;

	m_Light = new Light(CommonMeshes::Sphere(), false);
	m_Light->m_transform.SetScaling(Vec3Graphics(10.0f, 10.0f, 10.0f));

	//m_ProjectileTex = SOIL_load_OGL_texture(TEXTUREDIR"rocks1.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);
}

MyScene::~MyScene() {
	for (GameObject* go : m_Resources)
		delete go;

	delete m_Light;

	TestCases::ReleaseResources();
}

bool MyScene::InitialiseGL() {
	m_Camera->SetPosition(Vec3Physics(-6.25f, 2.0f, 10.0f));

	//Create Ground
	SimpleMeshObject* ground = new SimpleMeshObject("Ground");
	ground->SetMesh(CommonMeshes::Cube(), false);
	ground->SetLocalTransform(Mat4Physics::Translation(Vec3Physics(0.0, 0.0, 0.0f)) * Mat4Physics::Scale(Vec3Physics(60.0f, 0.5f, 60.0f))); //80m width, 1m height, 80m depth
	ground->SetColour(Vec4Graphics(0.2f, 1.0f, 0.5f, 1.0f));
	this->AddGameObject(ground);
	m_Resources.push_back(ground);

	TestCases::AddPlaneTestcase(this, Vec3Physics(0.0f, 0.0f, -40.0f));

	TestCases::AddWall(this, Vec3Physics(-60.0f, 0.0f, 0.0f), Vec3Physics(0.5f, 15.0f, 60.0f));
	TestCases::AddWall(this, Vec3Physics(60.0f, 0.0f, 0.0f), Vec3Physics(0.5f, 15.0f, 60.0f));
	TestCases::AddWall(this, Vec3Physics(0.0f, 0.0f, 60.0f), Vec3Physics(60.0f, 15.0f, 0.5f));
	TestCases::AddWall(this, Vec3Physics(0.0f, 0.0f, -60.0f), Vec3Physics(60.0f, 15.0f, 0.5f));

	/*
	TardisGameObject* tardis = new TardisGameObject();
	tardis->SetColour(Vec4Graphics(1.0f, 1.0f, 1.0f, 1.0f));
	tardis->Physics()->SetPosition(Vec3Physics(20.0f, 15.0f, 20.0f));
	tardis->Physics()->NeverRest();

	this->AddGameObject(tardis);
	m_Resources.push_back(tardis);
	*/

	return true;
}

//Scene* MyScene::GetNextScene(Window& window) {
//	return new EndScene(window);;
//}

void MyScene::Cleanup() {
	for (GameObject* go : m_Resources)
		go->Ditach();
}

void MyScene::UpdateScene(float sec) {
	sceneUpdateTimer.GetTimedMS();
	std::lock_guard<std::mutex> guard(PhysicsObject::g_ExternalChanges);

	Keyboard* keyboard = Window::GetKeyboard();

	static unsigned int projectileCounter = 0;

	if (keyboard->KeyTriggered(KEYBOARD_M))
		m_RenderMode = (m_RenderMode + 1) % RenderModeMax;

	if (keyboard->KeyTriggered(KEYBOARD_X) || keyboard->KeyTriggered(KEYBOARD_ESCAPE)) {
		m_EndScene = true;
	}

	if ((RenderModeMasks[m_RenderMode] & RenderModeMasks[DebugRenderMode]) != 0) {
		//PhysicsEngine::Instance()->DebugRender();
		//NCLDebug::AddStatusEntry(Vec4Graphics::ONES, "Physics Engine: %s (Press P to toggle)", PhysicsEngine::Instance()->IsPaused() ? "Paused" : "Enabled");
		//NCLDebug::AddStatusEntry(Vec4Graphics::ONES, "--------------------------------");
		//NCLDebug::AddStatusEntry(Vec4Graphics::ONES, "Physics Timestep : %5.2fms (%5.2f FPS)", PhysicsEngine::Instance()->GetUpdateTimestep() * 1000.0f, 1.0f / PhysicsEngine::Instance()->GetUpdateTimestep());
		NCLDebug::AddStatusEntry(Vec4Graphics::ONES, "Graphics Timestep: %5.2fms (%5.2f FPS)", sec * 1000.0f, 1.0f / sec);
		NCLDebug::AddStatusEntry(Vec4Graphics::ONES, "--------------------------------");
		//NCLDebug::AddStatusEntry(Vec4Graphics::ONES, "Physics Update: %5.2fms", PhysicsEngine::g_LastStepTime);
		NCLDebug::AddStatusEntry(Vec4Graphics::ONES, "Scene Update  : %5.2fms", sceneUpdateTimer.GetTimedMS());
	}

	Scene::UpdateScene(sec);
}

void MyScene::RenderScene() {
	if ((RenderModeMasks[m_RenderMode] & RenderModeMasks[NormalRenderMode]) != 0) {
		glClearColor(0.6f, 0.6f, 0.6f, 1.f);
		if ((RenderModeMasks[m_RenderMode] & RenderModeMasks[DebugRenderMode]) == 0)
			//force buffer swap and previous buffer cleanup
			NCLDebug::g_NewData = true;
		Scene::RenderScene();
	} else {
		glClearColor(0.0f, 0.0f, 0.0f, 1.f);
		glBindFramebuffer(GL_FRAMEBUFFER, m_ScreenFBO);
		viewMatrix = m_Camera->BuildViewMatrix();
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		RenderDebug();
	}
}

