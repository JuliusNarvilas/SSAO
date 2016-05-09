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
	void UpdateScene(float dt)  override;

 protected:
	std::vector<GameObject*> m_Resources;
};