#include "QuaternionIntrinsics.h"

const QuaternionIntrinsics QuaternionIntrinsics::EMPTY = QuaternionIntrinsics(_mm_setzero_ps());
const QuaternionIntrinsics QuaternionIntrinsics::IDENTITY = QuaternionIntrinsics(_mm_set_ps(1.0f, 0, 0, 0));


Matrix4Intrinsics QuaternionIntrinsics::ToMatrix4() const {
	__m128 multiplier2 = _mm_set1_ps(2.0f);
	__m128Converter squaredTimes2;
	squaredTimes2.mmvalue = _mm_mul_ps(_mm_mul_ps(mmvalue, mmvalue), multiplier2);
	__m128Converter offset1Times2;
	offset1Times2.mmvalue = _mm_mul_ps(_mm_mul_ps(mmvalue, _mm_set_ps(z, y, x, w)), multiplier2);
	__m128Converter offset2Times2;
	offset2Times2.mmvalue = _mm_mul_ps(_mm_mul_ps(mmvalue, _mm_set_ps(y, x, w, z)), multiplier2);

	Matrix4Intrinsics mat;

	mat.values[0] = 1 - squaredTimes2.y - squaredTimes2.z;
	mat.values[1] = offset1Times2.y + offset1Times2.w;
	mat.values[2] = offset2Times2.x - offset2Times2.y;

	mat.values[4] = offset1Times2.y - offset1Times2.w;
	mat.values[5] = 1 - squaredTimes2.x - squaredTimes2.z;
	mat.values[6] = offset1Times2.z + offset1Times2.x;

	mat.values[8] = offset2Times2.x + offset2Times2.y;
	mat.values[9] = offset1Times2.z - offset1Times2.x;
	mat.values[10] = 1 - squaredTimes2.x - squaredTimes2.y;

	return mat;
}

Matrix3Intrinsics QuaternionIntrinsics::ToMatrix3() const {
	Matrix3Intrinsics mat;

	float yy = y*y;
	float zz = z*z;
	float xy = x*y;
	float zw = z*w;
	float xz = x*z;
	float yw = y*w;
	float xx = x*x;
	float yz = y*z;
	float xw = x*w;

	mat.values[0] = 1 - 2 * yy - 2 * zz;
	mat.values[1] = 2 * xy + 2 * zw;
	mat.values[2] = 2 * xz - 2 * yw;

	mat.values[3] = 2 * xy - 2 * zw;
	mat.values[4] = 1 - 2 * xx - 2 * zz;
	mat.values[5] = 2 * yz + 2 * xw;

	mat.values[6] = 2 * xz + 2 * yw;
	mat.values[7] = 2 * yz - 2 * xw;
	mat.values[8] = 1 - 2 * xx - 2 * yy;

	return mat;
}

QuaternionIntrinsics QuaternionIntrinsics::EulerAnglesToQuaternion(float pitch, float yaw, float roll)	{
	float y2 = DegToRad(yaw / 2.0f);
	float p2 = DegToRad(pitch / 2.0f);
	float r2 = DegToRad(roll / 2.0f);

	float cosy   = cos(y2);
	float cosp   = cos(p2);
	float cosr   = cos(r2);

	float siny   = sin(y2);
	float sinp   = sin(p2);
	float sinr   = sin(r2);

	QuaternionIntrinsics q;

	q.x = cosr * sinp * cosy + sinr * cosp * siny;
	q.y = cosr * cosp * siny - sinr * sinp * cosy;
	q.z = sinr * cosp * cosy - cosr * sinp * siny;
	q.w = cosr * cosp * cosy + sinr * sinp * siny;

	return q;
};

QuaternionIntrinsics QuaternionIntrinsics::AxisAngleToQuaterion(const Vector3Intrinsics& vector, float degrees)	{
	float theta = DegToRad(degrees);
	float result = sin(theta * 0.5f);

	return QuaternionIntrinsics((float)(vector.x * result), (float)(vector.y * result), (float)(vector.z * result), (float)cos(theta * 0.5f));
}

void QuaternionIntrinsics::GenerateW()	{
	w = 1.0f - (x*x)-(y*y)-(z*z);
	if(w < 0.0f)
		w = 0.0f;
	else
		w = - sqrtf(w);
}

QuaternionIntrinsics QuaternionIntrinsics::FromMatrix(const Matrix4Intrinsics& m)	{
	QuaternionIntrinsics q;

	q.w = sqrt(std::max(0.0f, (1.0f + m.values[0] + m.values[5] + m.values[10]))) * 0.5f;
	q.x = sqrt(std::max(0.0f, (1.0f + m.values[0] - m.values[5] - m.values[10]))) * 0.5f;
	q.y = sqrt(std::max(0.0f, (1.0f - m.values[0] + m.values[5] - m.values[10]))) * 0.5f;
	q.z = sqrt(std::max(0.0f, (1.0f - m.values[0] - m.values[5] + m.values[10]))) * 0.5f;

	q.x = (float)_copysign(q.x, m.values[9] - m.values[6]);
	q.y = (float)_copysign(q.y, m.values[2] - m.values[8]);
	q.z = (float)_copysign(q.z, m.values[4] - m.values[1]);

	return q;
}

QuaternionIntrinsics QuaternionIntrinsics::Interpolate(const QuaternionIntrinsics& pStart, const QuaternionIntrinsics& pEnd, float pFactor)
{
	// calc cosine theta
	float cosom = pStart.x * pEnd.x + pStart.y * pEnd.y + pStart.z * pEnd.z + pStart.w * pEnd.w;

	// adjust signs (if necessary)
	QuaternionIntrinsics end = pEnd;
	if (cosom < 0.0f)
	{
		cosom = -cosom;
		end.x = -end.x;   // Reverse all signs
		end.y = -end.y;
		end.z = -end.z;
		end.w = -end.w;
	}

	// Calculate coefficients
	float sclp, sclq;
	if ((1.0f - cosom) > 0.0001f) // 0.0001 -> some epsillon
	{
		// Standard case (slerp)
		float omega, sinom;
		omega = acos(cosom); // extract theta from dot product's cos theta
		sinom = sin(omega);
		sclp = sin((1.0f - pFactor) * omega) / sinom;
		sclq = sin(pFactor * omega) / sinom;
	}
	else
	{
		// Very close, do linear interp (because it's faster)
		sclp = 1.0f - pFactor;
		sclq = pFactor;
	}

	QuaternionIntrinsics out;
	out.x = sclp * pStart.x + sclq * end.x;
	out.y = sclp * pStart.y + sclq * end.y;
	out.z = sclp * pStart.z + sclq * end.z;
	out.w = sclp * pStart.w + sclq * end.w;

	return out;
}