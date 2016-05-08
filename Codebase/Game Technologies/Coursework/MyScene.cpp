#include "MyScene.h"

#include <nclgl/OBJMesh.h>

#include <ncltech\SimpleMeshObject.h>
#include <ncltech\CommonMeshes.h>
#include <ncltech\NCLDebug.h>
#include "TardisGameObject.h"
#include "TestCases.h"
#include "nclgl\ModelLoader.h"


MyScene::MyScene(Window& window) : Scene(window) {
	if (init == true)
		init = InitialiseGL();

	UpdateWorldMatrices(m_RootGameObject, Mat4Physics::IDENTITY);
	RenderMode = NormalAndDebugRenderMode;

	//m_ProjectileTex = SOIL_load_OGL_texture(TEXTUREDIR"rocks1.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);

	m_Light->scale = 10;
	m_Light->position = Vec3Graphics(2, 3, 2);
}

MyScene::~MyScene() {
	for (GameObject* go : m_Resources)
		delete go;

	TestCases::ReleaseResources();
}

bool MyScene::InitialiseGL() {
	m_Camera->SetPosition(Vec3Physics(-1.5f, 3.0f, 3.0f));
	m_Camera->SetPitch(-35.0f);
	m_Camera->SetYaw(-20.0f);

	//Create Ground
	SimpleMeshObject* ground = new SimpleMeshObject("Ground");
	ground->SetMesh(CommonMeshes::Cube(), false);
	ground->SetLocalTransform(Mat4Physics::Translation(Vec3Physics(0.0, 0.0, 0.0f)) * Mat4Physics::Scale(Vec3Physics(3.0f, 0.1f, 3.0f))); //80m width, 1m height, 80m depth
	ground->SetColour(Vec4Graphics(0.2f, 1.0f, 0.5f, 1.0f));
	this->AddGameObject(ground);
	m_Resources.push_back(ground);

	//TestCases::AddPlaneTestcase(this, Vec3Physics(0.0f, 0.0f, -40.0f));

	TestCases::AddWall(this, Vec3Physics(1.5f, 0.5f, 0.0f), Vec3Physics(0.1f, 2.0f, 3.0f));
	TestCases::AddWall(this, Vec3Physics(0.0f, 0.5f, -1.5f), Vec3Physics(3.0f, 2.0f, 0.1f));
	//TestCases::AddWall(this, Vec3Physics::ZEROS, Vec3Physics(10.0f, 7.0f, 10.0f));


	/*SimpleMeshObject* test = new SimpleMeshObject("Test");
	test->SetMesh(new OBJMesh(MESHDIR"dragon.obj"), true);
	test->SetLocalTransform(Mat4Physics::Translation(Vec3Physics(0.0, 0.0, 0.0f)) * Mat4Physics::Scale(Vec3Physics(20.0f, 20.0f, 20.0f)));
	test->SetColour(Vec4Graphics(0.2f, 1.0f, 0.5f, 1.0f));
	this->AddGameObject(test);
	m_Resources.push_back(test);*/


	SimpleMeshObject* test = new SimpleMeshObject("Test");
	//Mesh* testMesh = ModelLoader::LoadMGL(MESHDIR"dragon2.mgl");
	Mesh* testMesh = ModelLoader::LoadMGL(MESHDIR"buddha4.mgl");
	test->SetMesh(testMesh, true);
	test->SetLocalTransform(Mat4Physics::Translation(Vec3Physics(0.0, 0.5, 0.0f)) * Mat4Physics::Scale(Vec3Physics(1.0f, 1.0f, 1.0f)) * Mat4Physics::RotationY(-90.0f));
	test->SetColour(Vec4Graphics(0.2f, 1.0f, 0.5f, 1.0f));
	this->AddGameObject(test);
	m_Resources.push_back(test);

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


void MyScene::UpdateScene(float sec) {
	sceneUpdateTimer.GetTimedMS();

	Keyboard* keyboard = Window::GetKeyboard();

	if (keyboard->KeyTriggered(KEYBOARD_M))
		RenderMode = (RenderMode + 1) % RenderModeMax;

	if (keyboard->KeyTriggered(KEYBOARD_X) || keyboard->KeyTriggered(KEYBOARD_ESCAPE)) {
		m_EndScene = true;
	}

	if ((RenderModeMasks[RenderMode] & RenderModeMasks[DebugRenderMode]) != 0) {
		//PhysicsEngine::Instance()->DebugRender();
		//NCLDebug::AddStatusEntry(Vec4Graphics::ONES, "Physics Engine: %s (Press P to toggle)", PhysicsEngine::Instance()->IsPaused() ? "Paused" : "Enabled");
		//NCLDebug::AddStatusEntry(Vec4Graphics::ONES, "--------------------------------");
		//NCLDebug::AddStatusEntry(Vec4Graphics::ONES, "Physics Timestep : %5.2fms (%5.2f FPS)", PhysicsEngine::Instance()->GetUpdateTimestep() * 1000.0f, 1.0f / PhysicsEngine::Instance()->GetUpdateTimestep());
		NCLDebug::AddStatusEntry(Vec4Graphics::ONES, "Graphics Timestep : %5.2fms (%5.2f FPS)", sec * 1000.0f, 1.0f / sec);
		//NCLDebug::AddStatusEntry(Vec4Graphics::ONES, "--------------------------------");
		//NCLDebug::AddStatusEntry(Vec4Graphics::ONES, "Physics Update: %5.2fms", PhysicsEngine::g_LastStepTime);
		NCLDebug::AddStatusEntry(Vec4Graphics::ONES, "Scene Update      : %5.2fms", sceneUpdateTimer.GetTimedMS());
	}

	Scene::UpdateScene(sec);
}

void MyScene::RenderScene() {
	sceneRenderTimer.GetTimedMS();
	if ((RenderModeMasks[RenderMode] & RenderModeMasks[NormalRenderMode]) != 0) {
		glClearColor(0.6f, 0.6f, 0.6f, 1.f);
		if ((RenderModeMasks[RenderMode] & RenderModeMasks[DebugRenderMode]) == 0)
			//force buffer swap and previous buffer cleanup
			NCLDebug::g_NewData = true;
		Scene::RenderScene();
	} else {
		glClearColor(0.0f, 0.0f, 0.0f, 1.f);
		glBindFramebuffer(GL_FRAMEBUFFER, m_GeometryPassFBO);
		viewMatrix = m_Camera->BuildViewMatrix();
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		RenderDebug();
	}
	if ((RenderModeMasks[RenderMode] & RenderModeMasks[DebugRenderMode]) != 0) {
		NCLDebug::AddStatusEntry(Vec4Graphics::ONES, "Scene Render      : %5.2fms", sceneRenderTimer.GetTimedMS());
	}
}

