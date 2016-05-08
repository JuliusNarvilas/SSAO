#include "Light.h"



Light::Light(Mesh* mesh, bool needsCleanup)
{
	m_Cleanup = needsCleanup;
	this->mesh = mesh;
	scale = 0;
}


Light::~Light()
{
	if (m_Cleanup)
	{
		delete mesh;
	}
}


void Light::UpdateLight(float msec)
{
	float dt = msec * 0.001f;
	float speed = 3.0f * dt; //1.5m per second


	if (Window::GetKeyboard()->KeyDown(KEYBOARD_UP))
		position += Vec3Graphics(0, 0, -1) * speed;
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_DOWN))
		position -= Vec3Graphics(0, 0, -1) * speed;

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_LEFT))
		position += Vec3Graphics(-1, 0, 0) * speed;
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_RIGHT))
		position -= Vec3Graphics(-1, 0, 0) * speed;
}