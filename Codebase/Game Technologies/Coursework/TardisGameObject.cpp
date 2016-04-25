#include "TardisGameObject.h"

#include "nclgl\OBJMesh.h"
#include "ncltech\PositioningState.h"
#include "Helpers\RNG.h"

TardisGameObject::TardisGameObject(void) : SimpleMeshObject("Tardis") {
	SetMesh(new OBJMesh(MESHDIR"TARDIS\\TARDIS.obj"), true);
	SetLocalTransform(Mat4Physics::Translation(Vec3Physics(0.0f, -3.0f, 0.0f)));
	IsBoundingSphere(false);
	SetBoundingHalfVolume(Vec3Physics(1.0f, 2.0f, 1.0f));
	m_Alpha = 0.0f;
	m_Accumulator = 0.0f;
	m_MaxAlpha = RNG32::Rand(1.2f, 4.0f);
	m_Period = RNG32::Rand(10.0f, 16.0f);
}


TardisGameObject::~TardisGameObject(void) {
}
