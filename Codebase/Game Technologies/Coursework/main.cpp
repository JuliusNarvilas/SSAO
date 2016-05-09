#include <cstdio>
#include <nclgl\Window.h>
#include "MyScene.h"
#include <ncltech\NCLDebug.h>
#include <future>
#include "ncltech\CommonMeshes.h"
#include "Helpers\MeasuringTimer.h"

Scene* scene = NULL;

int Quit(bool pause = false, const string& reason = "") {
	if (scene) {
		delete scene;
		scene = NULL;
	}

	Window::Destroy();

	if (pause) {
		std::cout << reason << std::endl;
		system("PAUSE");
	}

	return 0;
}

void GameLoop(Scene* scene) {


	while (Window::GetWindow().UpdateWindow() && !scene->GetEndScene()) {
		//Note that for the sake of simplicity, all calculations with time will be done in seconds (ms * 0.001)
		// this is to simplify physics, as I cant visualise how fast 0.02 meters per millisecond is.
		float dt = Window::GetWindow().GetTimer()->GetTimedMS() * 0.001f;	//How many milliseconds since last update?

		MEASURING_TIMER_LOG_START("frame");

		MEASURING_TIMER_LOG_START("update ");
		scene->UpdateScene(dt);
		MEASURING_TIMER_LOG_END();

		MEASURING_TIMER_LOG_START("render ");
		scene->RenderScene();
		MEASURING_TIMER_LOG_END();

		MEASURING_TIMER_LOG_START("present");
		scene->Present();
		MEASURING_TIMER_LOG_END();

		MEASURING_TIMER_LOG_END();

		if (Window::GetWindow().GetKeyboard()->KeyTriggered(KEYBOARD_R))
		{
			MEASURING_TIMER_PRINT(std::cout);
		}
		MEASURING_TIMER_CLEAR();
	}
}

int main() {
	//Initialise the Window
	if (!Window::Initialise("Game Technologies - Framework Example", 1280, 720, false))
		return Quit(true, "Window failed to initialise!");

	Window::GetWindow().LockMouseToWindow(true);
	Window::GetWindow().ShowOSPointer(false);

	//Initialise the Scene
	scene = new MyScene(Window::GetWindow());
	if (!scene->HasInitialised())
		return Quit(true, "Renderer failed to initialise!");

	while (scene) {
		GameLoop(scene);
		Scene* temp = scene->GetNextScene(Window::GetWindow());
		scene = temp;
	}

	//Cleanup
	return Quit();
}