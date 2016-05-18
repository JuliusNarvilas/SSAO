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

	m_Light->scale = 10;
	//m_Light->position = Vec3Graphics(1.3f, 4.0f, 2.0f);
	//SSAO test
	m_Light->position = Vec3Graphics(-3.0f, 4.0f, -1.0f);
}

MyScene::~MyScene() {
	for (GameObject* go : m_Resources)
		delete go;

	TestCases::ReleaseResources();
}

bool MyScene::InitialiseGL() {
	m_Camera->SetPosition(Vec3Physics(-1.5f, 3.0f, 3.0f));
	//model 
	m_Camera->SetPitch(-35.0f);
	m_Camera->SetYaw(-20.0f);
	//wall
	//m_Camera->SetPitch(-20.0f);
	//m_Camera->SetYaw(-60.0f);

	//shadow smoothing
	/*m_Camera->SetPosition(Vec3Physics(-1.3f, 1.0f, 0.8f));
	m_Camera->SetPitch(-20.0f);
	m_Camera->SetYaw(-15.0f);
	*/

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

	auto checkerboardTex = SOIL_load_OGL_texture(TEXTUREDIR"checkerboard.tga", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);
	SimpleMeshObject* test = new SimpleMeshObject("Test");
	//Mesh* testMesh = ModelLoader::LoadMGL(MESHDIR"teapot.mgl");
	Mesh* testMesh = ModelLoader::LoadMGL(MESHDIR"dragon.mgl");
	//Mesh* testMesh = ModelLoader::LoadMGL(MESHDIR"buddha.mgl");
	testMesh->SetTexture(checkerboardTex);
	test->SetMesh(testMesh, true);
	//test->SetLocalTransform(Mat4Physics::Translation(Vec3Physics(0.0, 0.1, 0.0f)) * Mat4Physics::Scale(Vec3Physics(1.0f, 1.0f, 1.0f)));
	test->SetLocalTransform(Mat4Physics::Translation(Vec3Physics(0.0, 1.0, 0.0f)) * Mat4Physics::Scale(Vec3Physics(2.0f, 2.0f, 2.0f)) * Mat4Physics::RotationY(-100.0f));
	//test->SetLocalTransform(Mat4Physics::Translation(Vec3Physics(0.0, 0.5, 0.0f)) * Mat4Physics::Scale(Vec3Physics(1.0f, 1.0f, 1.0f)) * Mat4Physics::RotationY(-160.0f));
	test->SetColour(Vec4Graphics(0.2f, 1.0f, 0.5f, 1.0f));
	this->AddGameObject(test);

	/*for (int i = 1; i < 100; ++i)
	{
		SimpleMeshObject* test = new SimpleMeshObject("Test");
		test->SetMesh(testMesh, false);
		test->SetLocalTransform(Mat4Physics::Translation(Vec3Physics(-0.5 * i, 0.5, 0.0f)) * Mat4Physics::Scale(Vec3Physics(1.0f, 1.0f, 1.0f)));
		this->AddGameObject(test);
		m_Resources.push_back(test);
	}*/

	return true;
}


void MyScene::UpdateScene(float sec) {
	Keyboard* keyboard = Window::GetKeyboard();

	if (keyboard->KeyTriggered(KEYBOARD_X) || keyboard->KeyTriggered(KEYBOARD_ESCAPE)) {
		m_EndScene = true;
	}

	NCLDebug::AddStatusEntry(Vec4Graphics::ONES, "Graphics Timestep : %5.2fms (%5.2f FPS)", sec * 1000.0f, 1.0f / sec);
	Scene::UpdateScene(sec);
}
