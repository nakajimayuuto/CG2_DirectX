#include "Shape.h"
#include "Matrix4x4.h"

OBB& OBB::operator=(const Matrix4x4& matrix){
    for (uint32_t i = 0; i < 3; i++) {
        orientations[i].x = matrix.matrix[i][0];
        orientations[i].y = matrix.matrix[i][1];
        orientations[i].z = matrix.matrix[i][2];
    }

    return *this;
}

void Plane::SetPlane(const Vector3& p0, const Vector3& p1, const Vector3& p2) {
	Vector3 v1 = static_cast<Vector3>(p1) - p0;
	Vector3 v2 = static_cast<Vector3>(p2) - p0;

	normal = v1.Cross(v2).Normalize();

	distance = -normal.Dot(p0);
}
