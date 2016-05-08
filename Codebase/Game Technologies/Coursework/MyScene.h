#pragma once

#include <nclgl\Mesh.h>
#include <ncltech\Scene.h>
#include <ncltech\SimpleMeshObject.h>
#include <nclgl\GameTimer.h>
#include <sstream>
#include <nclgl\Light.h>

enum EndGameStates {
	LoseGameState,
	HighScoreGameState,
	QuitGameState
};

class MyScene : public Scene {
 public:
	MyScene(Window& window);
	~MyScene();

	bool InitialiseGL()	override;
	void RenderScene() override;
	void UpdateScene(float dt)  override;
	//Scene* GetNextScene(Window& window) override;

 protected:
	GameTimer		sceneUpdateTimer;
	GameTimer		sceneRenderTimer;
	std::vector<GameObject*> m_Resources;
};