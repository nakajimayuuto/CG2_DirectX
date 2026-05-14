#include "Matrix4x4.h"
#include "Transform.h"

using namespace std;

Matrix4x4 Matrix4x4::operator+(const Matrix4x4& m1) {
	Matrix4x4 result;
	for (int row = 0; row < 4; row++) {
		for (int column = 0; column < 4; column++) {
			result.matrix[column][row] = matrix[column][row] + m1.matrix[column][row];
		}
	}

	return result;
}

Matrix4x4 Matrix4x4::operator-(const Matrix4x4& m1) {
	Matrix4x4 result;
	for (int row = 0; row < 4; row++) {
		for (int column = 0; column < 4; column++) {
			result.matrix[column][row] = matrix[column][row] - m1.matrix[column][row];
		}
	}

	return result;
}

Matrix4x4 Matrix4x4::operator*(float scalar) {
	Matrix4x4 result;
	for (int row = 0; row < 4; row++) {
		for (int column = 0; column < 4; column++) {
			result.matrix[column][row] = matrix[column][row] * scalar;
		}
	}

	return result;
}

Matrix4x4 Matrix4x4::operator*(const Matrix4x4& m1) {
	Matrix4x4 result;
	for (int row = 0; row < 4; row++) {
		for (int column = 0; column < 4; column++) {
			result.matrix[row][column] = (matrix[row][0] * m1.matrix[0][column]) + (matrix[row][1] * m1.matrix[1][column]) + (matrix[row][2] * m1.matrix[2][column]) + (matrix[row][3] * m1.matrix[3][column]);
		}
	}

	return result;
}

Matrix4x4& Matrix4x4::operator+=(const Matrix4x4& m1) {
	for (int row = 0; row < 4; row++) {
		for (int column = 0; column < 4; column++) {
			matrix[column][row] += m1.matrix[column][row];
		}
	}

	return *this;
}

Matrix4x4& Matrix4x4::operator-=(const Matrix4x4& m1) {
	for (int row = 0; row < 4; row++) {
		for (int column = 0; column < 4; column++) {
			matrix[column][row] -= m1.matrix[column][row];
		}
	}

	return *this;
}

Matrix4x4& Matrix4x4::operator*=(float scalar) {
	for (int row = 0; row < 4; row++) {
		for (int column = 0; column < 4; column++) {
			matrix[column][row] *= scalar;
		}
	}

	return *this;
}

Matrix4x4& Matrix4x4::operator*=(const Matrix4x4& m1) {
	for (int row = 0; row < 4; row++) {
		for (int column = 0; column < 4; column++) {
			matrix[row][column] = (matrix[row][0] * m1.matrix[0][column]) + (matrix[row][1] * m1.matrix[1][column]) + (matrix[row][2] * m1.matrix[2][column]) + (matrix[row][3] * m1.matrix[3][column]);
		}
	}

	return *this;
}

Matrix4x4 Matrix4x4::Inverse() {
	float determinat = {
		(matrix[0][0] * matrix[1][1] * matrix[2][2] * matrix[3][3])//
		+ (matrix[0][0] * matrix[1][2] * matrix[2][3] * matrix[3][1])
		+ (matrix[0][0] * matrix[1][3] * matrix[2][1] * matrix[3][2])

		- (matrix[0][0] * matrix[1][3] * matrix[2][2] * matrix[3][1])//
		- (matrix[0][0] * matrix[1][2] * matrix[2][1] * matrix[3][3])
		- (matrix[0][0] * matrix[1][1] * matrix[2][3] * matrix[3][2])

		- (matrix[0][1] * matrix[1][0] * matrix[2][2] * matrix[3][3])//
		- (matrix[0][2] * matrix[1][0] * matrix[2][3] * matrix[3][1])
		- (matrix[0][3] * matrix[1][0] * matrix[2][1] * matrix[3][2])

		+ (matrix[0][3] * matrix[1][0] * matrix[2][2] * matrix[3][1])//
		+ (matrix[0][2] * matrix[1][0] * matrix[2][1] * matrix[3][3])
		+ (matrix[0][1] * matrix[1][0] * matrix[2][3] * matrix[3][2])

		+ (matrix[0][1] * matrix[1][2] * matrix[2][0] * matrix[3][3])//
		+ (matrix[0][2] * matrix[1][3] * matrix[2][0] * matrix[3][1])
		+ (matrix[0][3] * matrix[1][1] * matrix[2][0] * matrix[3][2])

		- (matrix[0][3] * matrix[1][2] * matrix[2][0] * matrix[3][1])//
		- (matrix[0][2] * matrix[1][1] * matrix[2][0] * matrix[3][3])
		- (matrix[0][1] * matrix[1][3] * matrix[2][0] * matrix[3][2])

		- (matrix[0][1] * matrix[1][2] * matrix[2][3] * matrix[3][0])//
		- (matrix[0][2] * matrix[1][3] * matrix[2][1] * matrix[3][0])
		- (matrix[0][3] * matrix[1][1] * matrix[2][2] * matrix[3][0])

		+ (matrix[0][3] * matrix[1][2] * matrix[2][1] * matrix[3][0])//
		+ (matrix[0][2] * matrix[1][1] * matrix[2][3] * matrix[3][0])
		+ (matrix[0][1] * matrix[1][3] * matrix[2][2] * matrix[3][0])
	};

	Matrix4x4 result;
	result.matrix[0][0] = {
		(matrix[1][1] * matrix[2][2] * matrix[3][3])
		+ (matrix[1][2] * matrix[2][3] * matrix[3][1])
		+ (matrix[1][3] * matrix[2][1] * matrix[3][2])

		- (matrix[1][3] * matrix[2][2] * matrix[3][1])
		- (matrix[1][2] * matrix[2][1] * matrix[3][3])
		- (matrix[1][1] * matrix[2][3] * matrix[3][2])
	};

	result.matrix[0][1] = {
		-(matrix[0][1] * matrix[2][2] * matrix[3][3])
		- (matrix[0][2] * matrix[2][3] * matrix[3][1])
		- (matrix[0][3] * matrix[2][1] * matrix[3][2])

		+ (matrix[0][3] * matrix[2][2] * matrix[3][1])
		+ (matrix[0][2] * matrix[2][1] * matrix[3][3])
		+ (matrix[0][1] * matrix[2][3] * matrix[3][2])
	};

	result.matrix[0][2] = {
		(matrix[0][1] * matrix[1][2] * matrix[3][3])
		+ (matrix[0][2] * matrix[1][3] * matrix[3][1])
		+ (matrix[0][3] * matrix[1][1] * matrix[3][2])

		- (matrix[0][3] * matrix[1][2] * matrix[3][1])
		- (matrix[0][2] * matrix[1][1] * matrix[3][3])
		- (matrix[0][1] * matrix[1][3] * matrix[3][2])
	};

	result.matrix[0][3] = {
		-(matrix[0][1] * matrix[1][2] * matrix[2][3])
		- (matrix[0][2] * matrix[1][3] * matrix[2][1])
		- (matrix[0][3] * matrix[1][1] * matrix[2][2])

		+ (matrix[0][3] * matrix[1][2] * matrix[2][1])
		+ (matrix[0][2] * matrix[1][1] * matrix[2][3])
		+ (matrix[0][1] * matrix[1][3] * matrix[2][2])
	};

	result.matrix[1][0] = {
		-(matrix[1][0] * matrix[2][2] * matrix[3][3])
		- (matrix[1][2] * matrix[2][3] * matrix[3][0])
		- (matrix[1][3] * matrix[2][0] * matrix[3][2])

		+ (matrix[1][3] * matrix[2][2] * matrix[3][0])
		+ (matrix[1][2] * matrix[2][0] * matrix[3][3])
		+ (matrix[1][0] * matrix[2][3] * matrix[3][2])
	};

	result.matrix[1][1] = {
		(matrix[0][0] * matrix[2][2] * matrix[3][3])
		+ (matrix[0][2] * matrix[2][3] * matrix[3][0])
		+ (matrix[0][3] * matrix[2][0] * matrix[3][2])

		- (matrix[0][3] * matrix[2][2] * matrix[3][0])
		- (matrix[0][2] * matrix[2][0] * matrix[3][3])
		- (matrix[0][0] * matrix[2][3] * matrix[3][2])
	};

	result.matrix[1][2] = {
		-(matrix[0][0] * matrix[1][2] * matrix[3][3])
		- (matrix[0][2] * matrix[1][3] * matrix[3][0])
		- (matrix[0][3] * matrix[1][0] * matrix[3][2])

		+ (matrix[0][3] * matrix[1][2] * matrix[3][0])
		+ (matrix[0][2] * matrix[1][0] * matrix[3][3])
		+ (matrix[0][0] * matrix[1][3] * matrix[3][2])
	};

	result.matrix[1][3] = {
		(matrix[0][0] * matrix[1][2] * matrix[2][3])
		+ (matrix[0][2] * matrix[1][3] * matrix[2][0])
		+ (matrix[0][3] * matrix[1][0] * matrix[2][2])

		- (matrix[0][3] * matrix[1][2] * matrix[2][0])
		- (matrix[0][2] * matrix[1][0] * matrix[2][3])
		- (matrix[0][0] * matrix[1][3] * matrix[2][2])
	};

	result.matrix[2][0] = {
		(matrix[1][0] * matrix[2][1] * matrix[3][3])
		+ (matrix[1][1] * matrix[2][3] * matrix[3][0])
		+ (matrix[1][3] * matrix[2][0] * matrix[3][1])

		- (matrix[1][3] * matrix[2][1] * matrix[3][0])
		- (matrix[1][1] * matrix[2][0] * matrix[3][3])
		- (matrix[1][0] * matrix[2][3] * matrix[3][1])
	};

	result.matrix[2][1] = {
		-(matrix[0][0] * matrix[2][1] * matrix[3][3])
		- (matrix[0][1] * matrix[2][3] * matrix[3][0])
		- (matrix[0][3] * matrix[2][0] * matrix[3][1])

		+ (matrix[0][3] * matrix[2][1] * matrix[3][0])
		+ (matrix[0][1] * matrix[2][0] * matrix[3][3])
		+ (matrix[0][0] * matrix[2][3] * matrix[3][1])
	};

	result.matrix[2][2] = {
		(matrix[0][0] * matrix[1][1] * matrix[3][3])
		+ (matrix[0][1] * matrix[1][3] * matrix[3][0])
		+ (matrix[0][3] * matrix[1][0] * matrix[3][1])

		- (matrix[0][3] * matrix[1][1] * matrix[3][0])
		- (matrix[0][1] * matrix[1][0] * matrix[3][3])
		- (matrix[0][0] * matrix[1][3] * matrix[3][1])
	};

	result.matrix[2][3] = {
		-(matrix[0][0] * matrix[1][1] * matrix[2][3])
		- (matrix[0][1] * matrix[1][3] * matrix[2][0])
		- (matrix[0][3] * matrix[1][0] * matrix[2][1])

		+ (matrix[0][3] * matrix[1][1] * matrix[2][0])
		+ (matrix[0][1] * matrix[1][0] * matrix[2][3])
		+ (matrix[0][0] * matrix[1][3] * matrix[2][1])
	};

	result.matrix[3][0] = {
		-(matrix[1][0] * matrix[2][1] * matrix[3][2])
		- (matrix[1][1] * matrix[2][2] * matrix[3][0])
		- (matrix[1][2] * matrix[2][0] * matrix[3][1])

		+ (matrix[1][2] * matrix[2][1] * matrix[3][0])
		+ (matrix[1][1] * matrix[2][0] * matrix[3][2])
		+ (matrix[1][0] * matrix[2][2] * matrix[3][1])
	};

	result.matrix[3][1] = {
		(matrix[0][0] * matrix[2][1] * matrix[3][2])
		+ (matrix[0][1] * matrix[2][2] * matrix[3][0])
		+ (matrix[0][2] * matrix[2][0] * matrix[3][1])

		- (matrix[0][2] * matrix[2][1] * matrix[3][0])
		- (matrix[0][1] * matrix[2][0] * matrix[3][2])
		- (matrix[0][0] * matrix[2][2] * matrix[3][1])
	};

	result.matrix[3][2] = {
		-(matrix[0][0] * matrix[1][1] * matrix[3][2])
		- (matrix[0][1] * matrix[1][2] * matrix[3][0])
		- (matrix[0][2] * matrix[1][0] * matrix[3][1])

		+ (matrix[0][2] * matrix[1][1] * matrix[3][0])
		+ (matrix[0][1] * matrix[1][0] * matrix[3][2])
		+ (matrix[0][0] * matrix[1][2] * matrix[3][1])
	};

	result.matrix[3][3] = {
		(matrix[0][0] * matrix[1][1] * matrix[2][2])
		+ (matrix[0][1] * matrix[1][2] * matrix[2][0])
		+ (matrix[0][2] * matrix[1][0] * matrix[2][1])

		- (matrix[0][2] * matrix[1][1] * matrix[2][0])
		- (matrix[0][1] * matrix[1][0] * matrix[2][2])
		- (matrix[0][0] * matrix[1][2] * matrix[2][1])
	};

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			result.matrix[i][j] = result.matrix[i][j] / determinat;
		}
	}

	return result;
}

Matrix4x4 Matrix4x4::GetInverse(Matrix4x4 matrix){
	float determinat = {
		(matrix.matrix[0][0] * matrix.matrix[1][1] * matrix.matrix[2][2] * matrix.matrix[3][3])//
		+ (matrix.matrix[0][0] * matrix.matrix[1][2] * matrix.matrix[2][3] * matrix.matrix[3][1])
		+ (matrix.matrix[0][0] * matrix.matrix[1][3] * matrix.matrix[2][1] * matrix.matrix[3][2])

		- (matrix.matrix[0][0] * matrix.matrix[1][3] * matrix.matrix[2][2] * matrix.matrix[3][1])//
		- (matrix.matrix[0][0] * matrix.matrix[1][2] * matrix.matrix[2][1] * matrix.matrix[3][3])
		- (matrix.matrix[0][0] * matrix.matrix[1][1] * matrix.matrix[2][3] * matrix.matrix[3][2])

		- (matrix.matrix[0][1] * matrix.matrix[1][0] * matrix.matrix[2][2] * matrix.matrix[3][3])//
		- (matrix.matrix[0][2] * matrix.matrix[1][0] * matrix.matrix[2][3] * matrix.matrix[3][1])
		- (matrix.matrix[0][3] * matrix.matrix[1][0] * matrix.matrix[2][1] * matrix.matrix[3][2])

		+ (matrix.matrix[0][3] * matrix.matrix[1][0] * matrix.matrix[2][2] * matrix.matrix[3][1])//
		+ (matrix.matrix[0][2] * matrix.matrix[1][0] * matrix.matrix[2][1] * matrix.matrix[3][3])
		+ (matrix.matrix[0][1] * matrix.matrix[1][0] * matrix.matrix[2][3] * matrix.matrix[3][2])
			
		+ (matrix.matrix[0][1] * matrix.matrix[1][2] * matrix.matrix[2][0] * matrix.matrix[3][3])//
		+ (matrix.matrix[0][2] * matrix.matrix[1][3] * matrix.matrix[2][0] * matrix.matrix[3][1])
		+ (matrix.matrix[0][3] * matrix.matrix[1][1] * matrix.matrix[2][0] * matrix.matrix[3][2])
			
		- (matrix.matrix[0][3] * matrix.matrix[1][2] * matrix.matrix[2][0] * matrix.matrix[3][1])//
		- (matrix.matrix[0][2] * matrix.matrix[1][1] * matrix.matrix[2][0] * matrix.matrix[3][3])
		- (matrix.matrix[0][1] * matrix.matrix[1][3] * matrix.matrix[2][0] * matrix.matrix[3][2])
				
		- (matrix.matrix[0][1] * matrix.matrix[1][2] * matrix.matrix[2][3] * matrix.matrix[3][0])//
		- (matrix.matrix[0][2] * matrix.matrix[1][3] * matrix.matrix[2][1] * matrix.matrix[3][0])
		- (matrix.matrix[0][3] * matrix.matrix[1][1] * matrix.matrix[2][2] * matrix.matrix[3][0])
			
		+ (matrix.matrix[0][3] * matrix.matrix[1][2] * matrix.matrix[2][1] * matrix.matrix[3][0])//
		+ (matrix.matrix[0][2] * matrix.matrix[1][1] * matrix.matrix[2][3] * matrix.matrix[3][0])
		+ (matrix.matrix[0][1] * matrix.matrix[1][3] * matrix.matrix[2][2] * matrix.matrix[3][0])
	};

	Matrix4x4 result;
	result.matrix[0][0] = {
		  (matrix.matrix[1][1] * matrix.matrix[2][2] * matrix.matrix[3][3])
		+ (matrix.matrix[1][2] * matrix.matrix[2][3] * matrix.matrix[3][1])
		+ (matrix.matrix[1][3] * matrix.matrix[2][1] * matrix.matrix[3][2])

		- (matrix.matrix[1][3] * matrix.matrix[2][2] * matrix.matrix[3][1])
		- (matrix.matrix[1][2] * matrix.matrix[2][1] * matrix.matrix[3][3])
		- (matrix.matrix[1][1] * matrix.matrix[2][3] * matrix.matrix[3][2])
	};	  

	result.matrix[0][1] = {		 
		- (matrix.matrix[0][1] * matrix.matrix[2][2] * matrix.matrix[3][3])
		- (matrix.matrix[0][2] * matrix.matrix[2][3] * matrix.matrix[3][1])
		- (matrix.matrix[0][3] * matrix.matrix[2][1] * matrix.matrix[3][2])
		   
		+ (matrix.matrix[0][3] * matrix.matrix[2][2] * matrix.matrix[3][1])
		+ (matrix.matrix[0][2] * matrix.matrix[2][1] * matrix.matrix[3][3])
		+ (matrix.matrix[0][1] * matrix.matrix[2][3] * matrix.matrix[3][2])
	};	   
		   
	result.matrix[0][2] = {	
		  (matrix.matrix[0][1] * matrix.matrix[1][2] * matrix.matrix[3][3])
		+ (matrix.matrix[0][2] * matrix.matrix[1][3] * matrix.matrix[3][1])
		+ (matrix.matrix[0][3] * matrix.matrix[1][1] * matrix.matrix[3][2])
		  
		- (matrix.matrix[0][3] * matrix.matrix[1][2] * matrix.matrix[3][1])
		- (matrix.matrix[0][2] * matrix.matrix[1][1] * matrix.matrix[3][3])
		- (matrix.matrix[0][1] * matrix.matrix[1][3] * matrix.matrix[3][2])
	};

	result.matrix[0][3] = {
		- (matrix.matrix[0][1] * matrix.matrix[1][2] * matrix.matrix[2][3])
		- (matrix.matrix[0][2] * matrix.matrix[1][3] * matrix.matrix[2][1])
		- (matrix.matrix[0][3] * matrix.matrix[1][1] * matrix.matrix[2][2])

		+ (matrix.matrix[0][3] * matrix.matrix[1][2] * matrix.matrix[2][1])
		+ (matrix.matrix[0][2] * matrix.matrix[1][1] * matrix.matrix[2][3])
		+ (matrix.matrix[0][1] * matrix.matrix[1][3] * matrix.matrix[2][2])
	};	 

	result.matrix[1][0] = {
		- (matrix.matrix[1][0] * matrix.matrix[2][2] * matrix.matrix[3][3])
		- (matrix.matrix[1][2] * matrix.matrix[2][3] * matrix.matrix[3][0])
		- (matrix.matrix[1][3] * matrix.matrix[2][0] * matrix.matrix[3][2])

		+ (matrix.matrix[1][3] * matrix.matrix[2][2] * matrix.matrix[3][0])
		+ (matrix.matrix[1][2] * matrix.matrix[2][0] * matrix.matrix[3][3])
		+ (matrix.matrix[1][0] * matrix.matrix[2][3] * matrix.matrix[3][2])
	};	  

	result.matrix[1][1] = {
		  (matrix.matrix[0][0] * matrix.matrix[2][2] * matrix.matrix[3][3])
		+ (matrix.matrix[0][2] * matrix.matrix[2][3] * matrix.matrix[3][0])
		+ (matrix.matrix[0][3] * matrix.matrix[2][0] * matrix.matrix[3][2])

		- (matrix.matrix[0][3] * matrix.matrix[2][2] * matrix.matrix[3][0])
		- (matrix.matrix[0][2] * matrix.matrix[2][0] * matrix.matrix[3][3])
		- (matrix.matrix[0][0] * matrix.matrix[2][3] * matrix.matrix[3][2])
	};	

	result.matrix[1][2] = {
		- (matrix.matrix[0][0] * matrix.matrix[1][2] * matrix.matrix[3][3])
		- (matrix.matrix[0][2] * matrix.matrix[1][3] * matrix.matrix[3][0])
		- (matrix.matrix[0][3] * matrix.matrix[1][0] * matrix.matrix[3][2])

		+ (matrix.matrix[0][3] * matrix.matrix[1][2] * matrix.matrix[3][0])
		+ (matrix.matrix[0][2] * matrix.matrix[1][0] * matrix.matrix[3][3])
		+ (matrix.matrix[0][0] * matrix.matrix[1][3] * matrix.matrix[3][2])
	};

	result.matrix[1][3] = {
		  (matrix.matrix[0][0] * matrix.matrix[1][2] * matrix.matrix[2][3])
		+ (matrix.matrix[0][2] * matrix.matrix[1][3] * matrix.matrix[2][0])
		+ (matrix.matrix[0][3] * matrix.matrix[1][0] * matrix.matrix[2][2])

		- (matrix.matrix[0][3] * matrix.matrix[1][2] * matrix.matrix[2][0])
		- (matrix.matrix[0][2] * matrix.matrix[1][0] * matrix.matrix[2][3])
		- (matrix.matrix[0][0] * matrix.matrix[1][3] * matrix.matrix[2][2])
	};

	result.matrix[2][0] = {
		  (matrix.matrix[1][0] * matrix.matrix[2][1] * matrix.matrix[3][3])
		+ (matrix.matrix[1][1] * matrix.matrix[2][3] * matrix.matrix[3][0])
		+ (matrix.matrix[1][3] * matrix.matrix[2][0] * matrix.matrix[3][1])

		- (matrix.matrix[1][3] * matrix.matrix[2][1] * matrix.matrix[3][0])
		- (matrix.matrix[1][1] * matrix.matrix[2][0] * matrix.matrix[3][3])
		- (matrix.matrix[1][0] * matrix.matrix[2][3] * matrix.matrix[3][1])
	};

	result.matrix[2][1] = {
		- (matrix.matrix[0][0] * matrix.matrix[2][1] * matrix.matrix[3][3])
		- (matrix.matrix[0][1] * matrix.matrix[2][3] * matrix.matrix[3][0])
		- (matrix.matrix[0][3] * matrix.matrix[2][0] * matrix.matrix[3][1])

		+ (matrix.matrix[0][3] * matrix.matrix[2][1] * matrix.matrix[3][0])
		+ (matrix.matrix[0][1] * matrix.matrix[2][0] * matrix.matrix[3][3])
		+ (matrix.matrix[0][0] * matrix.matrix[2][3] * matrix.matrix[3][1])
	};

	result.matrix[2][2] = {
		  (matrix.matrix[0][0] * matrix.matrix[1][1] * matrix.matrix[3][3])
		+ (matrix.matrix[0][1] * matrix.matrix[1][3] * matrix.matrix[3][0])
		+ (matrix.matrix[0][3] * matrix.matrix[1][0] * matrix.matrix[3][1])

		- (matrix.matrix[0][3] * matrix.matrix[1][1] * matrix.matrix[3][0])
		- (matrix.matrix[0][1] * matrix.matrix[1][0] * matrix.matrix[3][3])
		- (matrix.matrix[0][0] * matrix.matrix[1][3] * matrix.matrix[3][1])
	};

	result.matrix[2][3] = {
		- (matrix.matrix[0][0] * matrix.matrix[1][1] * matrix.matrix[2][3])
		- (matrix.matrix[0][1] * matrix.matrix[1][3] * matrix.matrix[2][0])
		- (matrix.matrix[0][3] * matrix.matrix[1][0] * matrix.matrix[2][1])

		+ (matrix.matrix[0][3] * matrix.matrix[1][1] * matrix.matrix[2][0])
		+ (matrix.matrix[0][1] * matrix.matrix[1][0] * matrix.matrix[2][3])
		+ (matrix.matrix[0][0] * matrix.matrix[1][3] * matrix.matrix[2][1])
	};

	result.matrix[3][0] = {
		- (matrix.matrix[1][0] * matrix.matrix[2][1] * matrix.matrix[3][2])
		- (matrix.matrix[1][1] * matrix.matrix[2][2] * matrix.matrix[3][0])
		- (matrix.matrix[1][2] * matrix.matrix[2][0] * matrix.matrix[3][1])

		+ (matrix.matrix[1][2] * matrix.matrix[2][1] * matrix.matrix[3][0])
		+ (matrix.matrix[1][1] * matrix.matrix[2][0] * matrix.matrix[3][2])
		+ (matrix.matrix[1][0] * matrix.matrix[2][2] * matrix.matrix[3][1])
	};

	result.matrix[3][1] = {
		  (matrix.matrix[0][0] * matrix.matrix[2][1] * matrix.matrix[3][2])
		+ (matrix.matrix[0][1] * matrix.matrix[2][2] * matrix.matrix[3][0])
		+ (matrix.matrix[0][2] * matrix.matrix[2][0] * matrix.matrix[3][1])

		- (matrix.matrix[0][2] * matrix.matrix[2][1] * matrix.matrix[3][0])
		- (matrix.matrix[0][1] * matrix.matrix[2][0] * matrix.matrix[3][2])
		- (matrix.matrix[0][0] * matrix.matrix[2][2] * matrix.matrix[3][1])
	};

	result.matrix[3][2] = {
		- (matrix.matrix[0][0] * matrix.matrix[1][1] * matrix.matrix[3][2])
		- (matrix.matrix[0][1] * matrix.matrix[1][2] * matrix.matrix[3][0])
		- (matrix.matrix[0][2] * matrix.matrix[1][0] * matrix.matrix[3][1])

		+ (matrix.matrix[0][2] * matrix.matrix[1][1] * matrix.matrix[3][0])
		+ (matrix.matrix[0][1] * matrix.matrix[1][0] * matrix.matrix[3][2])
		+ (matrix.matrix[0][0] * matrix.matrix[1][2] * matrix.matrix[3][1])
	};

	result.matrix[3][3] = {
		  (matrix.matrix[0][0] * matrix.matrix[1][1] * matrix.matrix[2][2])
		+ (matrix.matrix[0][1] * matrix.matrix[1][2] * matrix.matrix[2][0])
		+ (matrix.matrix[0][2] * matrix.matrix[1][0] * matrix.matrix[2][1])

		- (matrix.matrix[0][2] * matrix.matrix[1][1] * matrix.matrix[2][0])
		- (matrix.matrix[0][1] * matrix.matrix[1][0] * matrix.matrix[2][2])
		- (matrix.matrix[0][0] * matrix.matrix[1][2] * matrix.matrix[2][1])
	};

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			result.matrix[i][j] = result.matrix[i][j] / determinat;
		}
	}

	return result;
}

Matrix4x4 Matrix4x4::Transpose() {
	Matrix4x4 result;

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			result.matrix[i][j] = matrix[j][i];
		}
	}

	return result;
}

Matrix4x4 Matrix4x4::GetTranspose(Matrix4x4 matrix){
	Matrix4x4 result;

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			result.matrix[i][j] = matrix.matrix[j][i];
		}
	}

	return result;
}

Matrix4x4 Matrix4x4::Identity() {
	Matrix4x4 result;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			if (i == j) {
				result.matrix[i][j] = 1.0f;
			} else {
				result.matrix[i][j] = 0.0f;
			}
		}
	}

	return result;
}

Vector3 Matrix4x4::MatrixTransform(const Vector3& vector) {
	Vector3 result;
	result.x = vector.x * matrix[0][0] + vector.y * matrix[1][0] + vector.z * matrix[2][0] + 1.0f * matrix[3][0];
	result.y = vector.x * matrix[0][1] + vector.y * matrix[1][1] + vector.z * matrix[2][1] + 1.0f * matrix[3][1];
	result.z = vector.x * matrix[0][2] + vector.y * matrix[1][2] + vector.z * matrix[2][2] + 1.0f * matrix[3][2];
	float w = vector.x * matrix[0][3] + vector.y * matrix[1][3] + vector.z * matrix[2][3] + 1.0f * matrix[3][3];
	assert(w != 0.0f);
	result.x /= w;
	result.y /= w;
	result.z /= w;
	return result;
}

Vector3 Matrix4x4::TransformNomal(const Vector3& vector){
	Vector3 result{
		vector.x * matrix[0][0] + vector.y * matrix[1][0] + vector.z * matrix[2][0],
		vector.x * matrix[0][1] + vector.y * matrix[1][1] + vector.z * matrix[2][1],
		vector.x * matrix[0][2] + vector.y * matrix[1][2] + vector.z * matrix[2][2]};
	return result;
}

Matrix4x4 Matrix4x4::MakeTranslateMatrix(const Vector3& translate) {
	Matrix4x4 result;

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			result.matrix[i][j] = 0.0f;
		}
	}

	result.matrix[0][0] = 1.0f;
	result.matrix[1][1] = 1.0f;
	result.matrix[2][2] = 1.0f;
	result.matrix[3][0] = translate.x;
	result.matrix[3][1] = translate.y;
	result.matrix[3][2] = translate.z;
	result.matrix[3][3] = 1.0f;

	return result;

}

Matrix4x4 Matrix4x4::MakeScaleMatrix(const Vector3& scale) {
	Matrix4x4 result;

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			result.matrix[i][j] = 0.0f;
		}
	}

	result.matrix[0][0] = scale.x;
	result.matrix[1][1] = scale.y;
	result.matrix[2][2] = scale.z;
	result.matrix[3][3] = 1.0f;

	return result;
}

Matrix4x4 Matrix4x4::MakeRotateXMatrix(float radian) {
	Matrix4x4 result;

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			result.matrix[i][j] = 0.0f;
		}
	}

	result.matrix[0][0] = 1.0f;
	result.matrix[1][1] = cos(radian);
	result.matrix[1][2] = sin(radian);
	result.matrix[2][1] = -sin(radian);
	result.matrix[2][2] = cos(radian);
	result.matrix[3][3] = 1.0f;

	return result;
}

Matrix4x4 Matrix4x4::MakeRotateYMatrix(float radian) {
	Matrix4x4 result;

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			result.matrix[i][j] = 0.0f;
		}
	}

	result.matrix[0][0] = cos(radian);
	result.matrix[0][2] = -sin(radian);
	result.matrix[1][1] = 1.0f;
	result.matrix[2][0] = sin(radian);
	result.matrix[2][2] = cos(radian);
	result.matrix[3][3] = 1.0f;

	return result;
}

Matrix4x4 Matrix4x4::MakeRotateZMatrix(float radian) {
	Matrix4x4 result;

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			result.matrix[i][j] = 0.0f;
		}
	}

	result.matrix[0][0] = cos(radian);
	result.matrix[0][1] = sin(radian);
	result.matrix[1][0] = -sin(radian);
	result.matrix[1][1] = cos(radian);
	result.matrix[2][2] = 1.0f;
	result.matrix[3][3] = 1.0f;

	return result;
}

Matrix4x4 Matrix4x4::MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate) {
	return MakeScaleMatrix(scale) * ((MakeRotateXMatrix(rotate.x) * MakeRotateYMatrix(rotate.y)) * MakeRotateZMatrix(rotate.z)) * MakeTranslateMatrix(translate);
}

Matrix4x4 Matrix4x4::MakeAffineMatrix(const Transform& transform){
	return MakeScaleMatrix(transform.scale) * ((MakeRotateXMatrix(transform.rotate.x) * MakeRotateYMatrix(transform.rotate.y)) * MakeRotateZMatrix(transform.rotate.z)) * MakeTranslateMatrix(transform.translate);
}

Transform Matrix4x4::MatrixToTransform(){
	Transform result;
	result.scale = MatrixToScale();
	result.rotate = MatrixToRotate();
	result.translate = MatrixToTranslate();

	return result;
}

Vector3 Matrix4x4::MatrixToScale(){
	Vector3 result;
	result.x = std::sqrt(std::pow(matrix[0][0], 2.0f) + std::pow(matrix[1][0], 2.0f) + std::pow(matrix[2][0], 2.0f));
	result.y = std::sqrt(std::pow(matrix[0][1], 2.0f) + std::pow(matrix[1][1], 2.0f) + std::pow(matrix[2][1], 2.0f));
	result.z = std::sqrt(std::pow(matrix[0][2], 2.0f) + std::pow(matrix[1][2], 2.0f) + std::pow(matrix[2][2], 2.0f));
	return result;
}

Vector3 Matrix4x4::MatrixToRotate(){
	Vector3 result;

	result.y = std::atan2(-matrix[2][0],std::sqrt(std::pow(matrix[0][0], 2.0f)+ std::pow(matrix[1][0], 2.0f)));
	result.x = std::atan2(matrix[2][1],matrix[2][2]);
	result.z = std::atan2(matrix[1][0],matrix[0][0]);

	return result;
}

Vector3 Matrix4x4::MatrixToTranslate(){
	return {matrix[3][0],matrix[3][1],matrix[3][2]};
}

Matrix4x4 Matrix4x4::MakeOrthographicMatrix(Vertex4 vertex4, float zNear, float zFar) {
	Matrix4x4 result;

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			result.matrix[i][j] = 0.0f;
		}
	}

	result.matrix[0][0] = 2.0f / (vertex4.rightBottom.x - vertex4.leftTop.x);
	result.matrix[1][1] = 2.0f / (vertex4.leftTop.y - vertex4.rightBottom.y);
	result.matrix[2][2] = 1.0f / (zFar - zNear);
	result.matrix[3][0] = (vertex4.leftTop.x + vertex4.rightBottom.x) / (vertex4.leftTop.x - vertex4.rightBottom.x);
	result.matrix[3][1] = (vertex4.leftTop.y + vertex4.rightBottom.y) / (vertex4.rightBottom.y - vertex4.leftTop.y);
	result.matrix[3][2] = zNear / (zNear - zFar);
	result.matrix[3][3] = 1.0f;

	return result;
}

Matrix4x4 Matrix4x4::MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip) {
	Matrix4x4 result;

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			result.matrix[i][j] = 0.0f;
		}
	}

	result.matrix[0][0] = (1.0f / aspectRatio) * (1 / tan(fovY / 2.0f));
	result.matrix[1][1] = (1 / tan(fovY / 2.0f));
	result.matrix[2][2] = farClip / (farClip - nearClip);
	result.matrix[2][3] = 1.0f;
	result.matrix[3][2] = -nearClip * farClip / (farClip - nearClip);

	return result;
}

Matrix4x4 Matrix4x4::MakeViewportMatrix(Vector3 leftTop, float width, float height, float minDepth, float maxDepth) {
	Matrix4x4 result;

	assert(minDepth <= maxDepth);

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			result.matrix[i][j] = 0.0f;
		}
	}

	result.matrix[0][0] = width / 2.0f;
	result.matrix[1][1] = -(height / 2.0f);
	result.matrix[2][2] = maxDepth - minDepth;
	result.matrix[3][0] = leftTop.x + (width / 2.0f);
	result.matrix[3][1] = leftTop.y + (height / 2.0f);
	result.matrix[3][2] = minDepth;
	result.matrix[3][3] = 1.0f;

	return result;
}
