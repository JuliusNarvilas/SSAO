#pragma once

#include <nclgl\Mesh.h>
#include <ncltech\Scene.h>
#include <ncltech\SimpleMeshObject.h>
#include <nclgl\GameTimer.h>
#include <sstream>

enum RenderMode : unsigned int {
	NormalRenderMode,
	DebugRenderMode,
	NormalAndDebugRenderMode,
	RenderModeMax
};
static const unsigned int RenderModeMasks[RenderModeMax] = {
	1,
	2,
	3
};

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
	void	Cleanup() override;
	//Scene* GetNextScene(Window& window) override;

 protected:
	unsigned int	m_RenderMode;
	GameTimer		sceneUpdateTimer;
	std::vector<GameObject*> m_Resources;
};