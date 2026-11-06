#include "Quaternion.h"

Quaternion Quaternion::Multiply(const Quaternion& lhs, const Quaternion& rhs) {
	Vector3 result = { 0.0f,0.0f,0.0f };
	result = lhs.GetVector3().Cross(rhs.GetVector3()) + (rhs.GetVector3() * lhs.w) + (lhs.GetVector3() * rhs.w);

	return
		Quaternion(
			result.x,
			result.y,
			result.z,
			lhs.w * rhs.w - lhs.GetVector3().Dot(rhs.GetVector3())
		);
	// return Quaternion();
}

Quaternion Quaternion::operator+(Quaternion r1){
	Quaternion result = {
		x + r1.x,
		y + r1.y,
		z + r1.z,
		w + r1.w,
	};
	return result;
}

Quaternion Quaternion::operator-() {
	return Quaternion(-x,-y,-z,-w);
}

float Quaternion::Dot(const Quaternion& q1)const {
	return (x * q1.x) + (y * q1.y) + (z * q1.z) + (w * q1.w);
}

Quaternion Quaternion::operator*(Quaternion r1) {
	Vector3 result = { 0.0f,0.0f,0.0f };
	result = GetVector3().Cross(r1.GetVector3()) + (GetVector3() * r1.w) + (r1.GetVector3() * w);

	return
		Quaternion(
			result.x,
			result.y,
			result.z,
			w * r1.w - GetVector3().Dot(r1.GetVector3())
		);
}

Quaternion Quaternion::operator*(float scalar) {
	return Quaternion({x * scalar, y * scalar ,z *scalar ,w * scalar});
}

Quaternion Quaternion::operator*=(Quaternion r1) {
	Vector3 result = { 0.0f,0.0f,0.0f };
	result = GetVector3().Cross(r1.GetVector3()) + (GetVector3() * r1.w) + (r1.GetVector3() * w);

	x = result.x;
	y = result.y;
	z = result.z;
	w = w * r1.w - GetVector3().Dot(r1.GetVector3());

	return Quaternion(x, y, z, w);
}

Quaternion Quaternion::Conjugation() const{
	return {-GetVector3().x,-GetVector3().y, -GetVector3().z,w };
}

Quaternion Quaternion::Normalize() const{
	Quaternion result;
	result.w = w / Norm();
	result.x = x / Norm();
	result.y = y / Norm();
	result.z = z / Norm();

	return result;
}

Quaternion Quaternion::Inverse() const {
	Quaternion result;
	result.w = Conjugation().w / pow(Norm(), 2.0f); 
	result.x = Conjugation().x / pow(Norm(), 2.0f); 
	result.y = Conjugation().y / pow(Norm(), 2.0f); 
	result.z = Conjugation().z / pow(Norm(), 2.0f); 

	return result;
}

Quaternion Quaternion::MakeRotateAxisAngleQuaternion(const Vector3& axis, float angle){
	Quaternion rotateQuaternion;
	rotateQuaternion.SetVector3(static_cast<Vector3>(axis) * sin(angle / 2.0f));
	rotateQuaternion.w = cos(angle / 2.0f);

	return rotateQuaternion;
}

Vector3 Quaternion::RotateVector(const Vector3& vector){
	Quaternion vectorQuaternion;
	vectorQuaternion.SetVector3(vector);
	vectorQuaternion.w = 0.0f;

	Quaternion rotateQuaternion;
	rotateQuaternion = {x,y,z,w};
	rotateQuaternion = rotateQuaternion.Normalize();

	vectorQuaternion = rotateQuaternion * vectorQuaternion * rotateQuaternion.Conjugation();
	return vectorQuaternion.GetVector3();
}

Quaternion Quaternion::GetDirectionToDirection(const Vector3& from, const Vector3& to){
	const float tolerance = 1.0e-6f;
	float dot = Vector3::GetDot(from,to);
	Vector3 axis = Vector3(from.y,-from.x,0.0f).Normalize();
	if (dot <= -1.0f + tolerance) {
		return Quaternion(axis.x,axis.y,axis.z,0.0f);
	}

	Vector3 cross = Vector3::GetCross(from,to);
	Quaternion rotation{
		cross.x,cross.y,cross.z,1.0f + dot
	};

	return rotation.Normalize();
}
