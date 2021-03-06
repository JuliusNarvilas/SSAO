#pragma once
/*
Class:OGLRenderer
Author:Rich Davison	<richard.davison4@newcastle.ac.uk>
Description:Abstract base class for the graphics tutorials. Creates an OpenGL
3.2 CORE PROFILE rendering context. Each lesson will create a renderer that
inherits from this class - so all context creation is handled automatically,
but students still get to see HOW such a context is created.

-_-_-_-_-_-_-_,------,
_-_-_-_-_-_-_-|   /\_/\   NYANYANYAN
-_-_-_-_-_-_-~|__( ^ .^) /
_-_-_-_-_-_-_-""  ""

*/
#include "Common.h"

#include <string>
#include <fstream>
#include <vector>

#include "GL/glew.h"
#include "GL/wglew.h"

#include "../SOIL2/src/SOIL2/SOIL2.h"

#include "Math/nclglMath.h"
#include "Window.h"
//#include "light.h"

#include "Shader.h"		//Students make this file...
#include "Mesh.h"		//And this one...

using std::vector;

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glew32.lib")
#ifdef _DEBUG
#pragma comment(lib, "SOIL2-debug.lib")
#else
#pragma comment(lib, "SOIL2.lib")
#endif

#ifdef _DEBUG
#define GL_BREAKPOINT glUniform4uiv(0,0,0);//Invalid, but triggers gdebugger ;)
#else
#define GL_BREAKPOINT //
#endif

//#define OPENGL_DEBUGGING

static const float biasValues[16] = {
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 0.5, 0.0,
	0.5, 0.5, 0.5, 1.0
};
static const Mat4Graphics biasMatrix(biasValues);

enum DebugDrawMode {
	DEBUGDRAW_ORTHO,
	DEBUGDRAW_PERSPECTIVE
};

struct DebugDrawData {
	vector<Vec3Graphics> lines;
	vector<Vec3Graphics> colours;

	GLuint array;
	GLuint buffers[2];

	DebugDrawData();
	void Draw();

	~DebugDrawData() {
		glDeleteVertexArrays(1, &array);
		glDeleteBuffers(2,buffers);
	}

	inline void Clear() {
		lines.clear();
		colours.clear();
	}

	inline void AddLine(const Vec3Graphics& from,const Vec3Graphics& to,const Vec3Graphics& fromColour,const Vec3Graphics& toColour) {
		lines.push_back(from);
		lines.push_back(to);

		colours.push_back(fromColour);
		colours.push_back(toColour);
	}
};


class Shader;

class OGLRenderer	{
 public:
	friend class Window;
	OGLRenderer(Window& parent);
	virtual ~OGLRenderer(void);

	virtual void	RenderScene()		= 0;
	virtual void	UpdateScene(float msec);
	void			SwapBuffers();

	bool			HasInitialised() const;

	static void		DrawDebugLine(DebugDrawMode mode, const Vec3Graphics& from,const Vec3Graphics& to,const Vec3Graphics& fromColour = Vec3Graphics(1,1,1),const Vec3Graphics& toColour = Vec3Graphics(1,1,1));
	static void		DrawDebugBox(DebugDrawMode mode, const Vec3Graphics& at,const Vec3Graphics& scale,const Vec3Graphics& colour = Vec3Graphics(1,1,1));
	static void		DrawDebugCross(DebugDrawMode mode, const Vec3Graphics& at,const Vec3Graphics& scale,const Vec3Graphics& colour = Vec3Graphics(1,1,1));
	static void		DrawDebugCircle(DebugDrawMode mode, const Vec3Graphics& at,const float radius,const Vec3Graphics& colour = Vec3Graphics(1,1,1));
	static void		DrawDebugMatrix(const Mat3Physics& m, const Vec3Physics& position);

	void			SetAsDebugDrawingRenderer() {
		debugDrawingRenderer = this;
	}

	Shader*			GetCurrentShader() const {
		return currentShader;
	}

 protected:
	virtual void	Resize(int x, int y);
	void			UpdateShaderMatrices();
	void			SetCurrentShader(Shader* s);

	void			SetTextureRepeating(GLuint target, bool state);

	//void			SetShaderLight(const Light &l);

	void			DrawDebugPerspective(Mat4Graphics* matrix = 0);
	void			DrawDebugOrtho(Mat4Graphics* matrix = 0);

	Shader* currentShader;


	Mat4Graphics projMatrix;		//Projection matrix
	Mat4Graphics modelMatrix;	//Model matrix. NOT MODELVIEW
	Mat4Graphics viewMatrix;		//View matrix
	Mat4Graphics textureMatrix;	//Texture matrix

	int		width;			//Render area width (not quite the same as window width)
	int		height;			//Render area height (not quite the same as window height)
	bool	init;			//Did the renderer initialise properly?

	HDC		deviceContext;	//...Device context?
	HGLRC	renderContext;	//Permanent Rendering Context

	static DebugDrawData* orthoDebugData;
	static DebugDrawData* perspectiveDebugData;

	static OGLRenderer*	  debugDrawingRenderer;
	static Shader*		  debugDrawShader;

#ifdef _DEBUG
	static void CALLBACK DebugCallback(GLuint source, GLuint type,GLuint id, GLuint severity,
	                                   int length, const char* message, void* userParam);
#endif

	static bool	drawnDebugOrtho;
	static bool	drawnDebugPerspective;

};