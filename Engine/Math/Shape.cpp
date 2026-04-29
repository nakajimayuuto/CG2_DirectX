#include "Shape.h"

OBB& OBB::operator=(const Matrix4x4& matrix){
    for (uint32_t i = 0; i < 3; i++) {
        orientations[i].x = matrix.matrix[i][0];
        orientations[i].y = matrix.matrix[i][1];
        orientations[i].z = matrix.matrix[i][2];
    }

    return *this;
}
