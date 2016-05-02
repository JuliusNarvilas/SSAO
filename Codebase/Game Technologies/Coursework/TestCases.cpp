#include "TestCases.h"
#include "ncltech/SimpleMeshObject.h"
#include "ncltech/CommonMeshes.h"
#include <sstream>

std::vector<GameObject*> TestCases::m_Resources;

void TestCases::AddSimpleStackedRestingTestcase(Scene* scene, const Vec3Physics& pos) {
	Vec3Physics cubeScale = Vec3Physics(1.0f, 1.0f, 1.0f);
	SimpleMeshObject* cube = new SimpleMeshObject("Cube Testcase1");
	cube->SetMesh(CommonMeshes::Cube(), false);
	cube->SetLocalTransform(Mat4Physics::Scale(cubeScale));
	cube->SetColour(Vec4Graphics(0.2f, 0.5f, 1.0f, 1.0f));
	cube->SetBoundingRadius(1.734f);
	scene->AddGameObject(cube);
	m_Resources.push_back(cube);

	Vec3Physics sphereScale = Vec3Physics(1.0f, 1.0f, 1.0f);
	SimpleMeshObject* sphere = new SimpleMeshObject("Sphere Testcase1");
	sphere->SetMesh(CommonMeshes::Sphere(), false);
	sphere->SetLocalTransform(Mat4Physics::Scale(sphereScale));
	sphere->SetColour(Vec4Graphics(0.2f, 0.5f, 1.0f, 1.0f));
	sphere->SetBoundingRadius(sphereScale.x + 0.001f);
	scene->AddGameObject(sphere);
	m_Resources.push_back(sphere);
}


void TestCases::AddSimpleSwingingTestcase(Scene* scene, const Vec3Physics& pos) {
	SimpleMeshObject* cube = new SimpleMeshObject("Cube");
	cube->SetMesh(CommonMeshes::Cube(), false);
	cube->SetLocalTransform(Mat4Physics::Scale(Vec3Physics(0.5f, 0.5f, 0.5f)));	//1m x 1m x 1m
	cube->SetColour(Vec4Graphics(0.2f, 0.5f, 1.0f, 1.0f));
	cube->SetBoundingRadius(0.867f);

	scene->AddGameObject(cube);
	m_Resources.push_back(cube);

	std::stringstream ss;

	for (size_t i = 0; i < 3; ++i) {
		ss << "Sphere Swinging " << i;
		SimpleMeshObject* sphere = new SimpleMeshObject(ss.str());
		ss.str("");
		sphere->SetMesh(CommonMeshes::Sphere(), false);
		sphere->SetLocalTransform(Mat4Physics::Scale(Vec3Physics(0.5f, 0.5f, 0.5f)));//1m radius
		sphere->SetColour(Vec4Graphics(1.0f, 0.2f, 0.5f, 1.0f));
		sphere->SetBoundingRadius(1.0f + 0.001f);

		scene->AddGameObject(sphere);
		m_Resources.push_back(sphere);
	}
}


void TestCases::AddPlaneTestcase(Scene* scene, const Vec3Physics& pos) {
	Vec3Physics halfVol = Vec3Physics(10.0f, 0.25f, 10.0f);
	float length = halfVol.Length();
	SimpleMeshObject* plane1 = new SimpleMeshObject("Plane1");
	plane1->SetMesh(CommonMeshes::Cube(), false);
	plane1->SetLocalTransform(Mat4Physics::Scale(halfVol)); //80m width, 1m height, 80m depth
	plane1->SetColour(Vec4Graphics(0.8f, 0.5f, 0.5f, 1.0f));
	plane1->SetBoundingRadius(length);
	//scene->AddGameObject(plane1);
	m_Resources.push_back(plane1);

	SimpleMeshObject* plane2 = new SimpleMeshObject("Plane2");
	plane2->SetMesh(CommonMeshes::Cube(), false);
	plane2->SetLocalTransform(Mat4Physics::Scale(halfVol)); //80m width, 1m height, 80m depth
	plane2->SetColour(Vec4Graphics(0.8f, 0.5f, 0.5f, 1.0f));
	plane2->SetBoundingRadius(length);
	//scene->AddGameObject(plane2);
	m_Resources.push_back(plane2);
}


void TestCases::AddWall(Scene* scene, const Vec3Physics& pos, const Vec3Physics& scale) {
	float length = scale.Length();
	SimpleMeshObject* plane1 = new SimpleMeshObject("Plane1");
	plane1->SetMesh(CommonMeshes::Cube(), false);
	plane1->SetLocalTransform(Mat4Physics::Scale(scale)); //80m width, 1m height, 80m depth
	plane1->SetColour(Vec4Graphics(0.8f, 0.8f, 0.8f, 1.0f));
	plane1->SetBoundingRadius(length);
	plane1->IsBoundingSphere(false);
	plane1->SetBoundingHalfVolume(scale);
	scene->AddGameObject(plane1);
	m_Resources.push_back(plane1);
}

void TestCases::AddCuboidStack(Scene* scene, const Vec3Physics& pos, unsigned int size) {
	Vec3Physics scale = Vec3Physics(1.0f, 1.0f, 1.0f);
	float length = scale.Length();

	for (unsigned int i = 0; i < size; ++i) {
		SimpleMeshObject* box = new SimpleMeshObject("Plane1");
		box->SetMesh(CommonMeshes::Cube(), false);
		box->SetLocalTransform(Mat4Physics::Scale(scale)); //80m width, 1m height, 80m depth
		box->SetColour(Vec4Graphics(0.8f, 0.8f, 0.3f, 1.0f));
		box->SetBoundingRadius(length);
		scene->AddGameObject(box);
		m_Resources.push_back(box);
	}
}


void TestCases::AddSpheresCuboid(Scene* scene, const Vec3Physics& pos, const Vec3Physics& count) {
	Vec3Physics start = pos - (count * 0.5f);
	start.y += count.y;
	Vec3Physics sphereScale = Vec3Physics(1.0f, 1.0f, 1.0f);

	for (unsigned int x = 0; x < count.x; ++x) {
		for (unsigned int y = 0; y < count.y; ++y) {
			for (unsigned int z = 0; z < count.z; ++z) {
				SimpleMeshObject* sphere = new SimpleMeshObject("Sphere");
				sphere->SetMesh(CommonMeshes::Sphere(), false);
				sphere->SetLocalTransform(Mat4Physics::Scale(sphereScale));
				sphere->SetColour(Vec4Graphics(0.2f, 0.8f, 0.5f, 1.0f));
				sphere->SetBoundingRadius(sphereScale.x + 0.001f);
				scene->AddGameObject(sphere);
				m_Resources.push_back(sphere);
			}
		}
	}
}

SimpleMeshObject* GetPyramid(const Vec3Physics& scale) {
	SimpleMeshObject* pyramid = new SimpleMeshObject("Pyramid");
	pyramid->SetMesh(CommonMeshes::Pyramid(), false);
	pyramid->SetLocalTransform(Mat4Physics::Scale(scale));
	pyramid->SetColour(Vec4Graphics(1.0f, 0.2f, 0.5f, 1.0f));
	pyramid->SetBoundingRadius(scale.Length() + 0.001f);
	return pyramid;
}

void TestCases::AddStackedPyramidTestcase(Scene* scene, const Vec3Physics& pos, bool resting) {
	SimpleMeshObject* pyramid1 = GetPyramid(Vec3Physics(1.5f, 1.0f, 1.5f));
	scene->AddGameObject(pyramid1);
	m_Resources.push_back(pyramid1);

	SimpleMeshObject* pyramid2 = GetPyramid(Vec3Physics(0.5f, 1.0f, 0.5f));
	scene->AddGameObject(pyramid2);
	m_Resources.push_back(pyramid2);

	SimpleMeshObject* pyramid3 = GetPyramid(Vec3Physics(0.5f, 1.0f, 0.5f));
	scene->AddGameObject(pyramid3);
	m_Resources.push_back(pyramid3);

	SimpleMeshObject* pyramid4 = GetPyramid(Vec3Physics(0.5f, 1.0f, 0.5f));
	scene->AddGameObject(pyramid4);
	m_Resources.push_back(pyramid4);

	SimpleMeshObject* pyramid5 = GetPyramid(Vec3Physics(0.5f, 1.0f, 0.5f));
	scene->AddGameObject(pyramid5);
	m_Resources.push_back(pyramid5);
}


void TestCases::ReleaseResources() {
	for (GameObject* go : m_Resources)
		delete go;
}