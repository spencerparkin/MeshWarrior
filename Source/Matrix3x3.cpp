#include "Matrix3x3.h"
#include <assert.h>

using namespace MeshWarrior;

Matrix3x3::Matrix3x3()
{
	this->SetIdentity();
}

/*virtual*/ Matrix3x3::~Matrix3x3()
{
}

void Matrix3x3::SetCopy(const Matrix3x3& matrix)
{
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			this->ele[i][j] = matrix.ele[i][j];
}

void Matrix3x3::GetCopy(Matrix3x3& matrix) const
{
	matrix.SetCopy(*this);
}

void Matrix3x3::SetIdentity(void)
{
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			this->ele[i][j] = (i == j) ? 1.0 : 0.0;
}

void Matrix3x3::SetRow(int i, const Vector& vector)
{
	assert(0 <= i && i < 3);
	this->ele[i][0] = vector.x;
	this->ele[i][1] = vector.y;
	this->ele[i][2] = vector.z;
}

void Matrix3x3::SetCol(int j, const Vector& vector)
{
	assert(0 <= j && j < 3);
	this->ele[0][j] = vector.x;
	this->ele[1][j] = vector.y;
	this->ele[2][j] = vector.z;
}

Vector Matrix3x3::GetRow(int i) const
{
	assert(0 <= i && i < 3);
	return Vector(this->ele[i][0], this->ele[i][1], this->ele[i][2]);
}

Vector Matrix3x3::GetCol(int j) const
{
	assert(0 <= j && j < 3);
	return Vector(this->ele[0][j], this->ele[1][j], this->ele[2][j]);
}

// TODO: The SVD would give us a best-fit rotation matrix.
//       Here we're just using the Graham-Schmidt process.
//       This, however, is a good way to deal with accumulated
//       round-off error in the calculation of rotation matrices
//       in some cases.
void Matrix3x3::Orthonormalize(void)
{
	Vector xAxis = this->GetCol(0);
	Vector yAxis = this->GetCol(1);
	Vector zAxis = this->GetCol(2);

	xAxis.Normalize();
	yAxis.Reject(yAxis, xAxis);
	zAxis.Cross(xAxis, yAxis);

	this->SetCol(0, xAxis);
	this->SetCol(1, yAxis);
	this->SetCol(2, zAxis);
}

void Matrix3x3::SetFromAxisAngle(const Vector& axis, double angle)
{
	Vector xAxis(1.0, 0.0, 0.0);
	Vector yAxis(0.0, 1.0, 0.0);
	Vector zAxis(0.0, 0.0, 1.0);

	xAxis.Rotate(xAxis, axis, angle);
	yAxis.Rotate(yAxis, axis, angle);
	zAxis.Rotate(zAxis, axis, angle);

	this->SetCol(0, xAxis);
	this->SetCol(1, yAxis);
	this->SetCol(2, zAxis);
}

bool Matrix3x3::GetToAxisAngle(Vector& axis, double& angle) const
{
	Quaternion quat;
	if (!this->GetToQuat(quat))
		return false;

	axis = quat.GetRotationAxis();
	angle = quat.GetRotationAngle();

	return true;
}

void Matrix3x3::SetFromQuat(const Quaternion& quat)
{
	Vector xAxis(1.0, 0.0, 0.0);
	Vector yAxis(0.0, 1.0, 0.0);
	Vector zAxis(0.0, 0.0, 1.0);

	xAxis = quat.RotateVector(xAxis);
	yAxis = quat.RotateVector(yAxis);
	zAxis = quat.RotateVector(zAxis);

	this->SetCol(0, xAxis);
	this->SetCol(1, yAxis);
	this->SetCol(2, zAxis);
}

bool Matrix3x3::GetToQuat(Quaternion& quat) const
{
	// This is Cayley's method taken from "A Survey on the Computation of Quaternions from Rotation Matrices" by Sarabandi & Thomas.

	double r11 = this->ele[0][0];
	double r21 = this->ele[1][0];
	double r31 = this->ele[2][0];

	double r12 = this->ele[0][1];
	double r22 = this->ele[1][1];
	double r32 = this->ele[2][1];

	double r13 = this->ele[0][2];
	double r23 = this->ele[1][2];
	double r33 = this->ele[2][2];

	quat.w = 0.25 * ::sqrt(MW_SQUARED(r11 + r22 + r33 + 1.0) + MW_SQUARED(r32 - r23) + MW_SQUARED(r13 - r31) + MW_SQUARED(r21 - r12));
	quat.x = 0.25 * ::sqrt(MW_SQUARED(r32 - r23) + MW_SQUARED(r11 - r22 - r33 + 1.0) + MW_SQUARED(r21 + r12) + MW_SQUARED(r31 + r13)) * MW_SIGN(r32 - r23);
	quat.y = 0.25 * ::sqrt(MW_SQUARED(r13 - r31) + MW_SQUARED(r21 + r12) + MW_SQUARED(r22 - r11 - r33 + 1.0) + MW_SQUARED(r32 + r23)) * MW_SIGN(r13 - r31);
	quat.z = 0.25 * ::sqrt(MW_SQUARED(r21 - r12) + MW_SQUARED(r31 + r13) + MW_SQUARED(r32 + r23) + MW_SQUARED(r33 - r11 - r22 + 1.0)) * MW_SIGN(r21 - r12);

	return true;
}

bool Matrix3x3::SetInverse(const Matrix3x3& matrix)
{
	return matrix.GetInverse(*this);
}

bool Matrix3x3::GetInverse(Matrix3x3& matrix) const
{
	double det = this->Determinant();
	double scale = 1.0f / det;
	if (scale != scale || ::isinf(scale) || ::isnan(scale))
		return false;

	matrix.ele[0][0] = this->ele[1][1] * this->ele[2][2] - this->ele[2][1] * this->ele[1][2];
	matrix.ele[0][1] = this->ele[0][2] * this->ele[2][1] - this->ele[2][2] * this->ele[0][1];
	matrix.ele[0][2] = this->ele[0][1] * this->ele[1][2] - this->ele[1][1] * this->ele[0][2];

	matrix.ele[1][0] = this->ele[1][2] * this->ele[2][0] - this->ele[2][2] * this->ele[1][0];
	matrix.ele[1][1] = this->ele[0][0] * this->ele[2][2] - this->ele[2][0] * this->ele[0][2];
	matrix.ele[1][2] = this->ele[0][2] * this->ele[1][0] - this->ele[1][2] * this->ele[0][0];

	matrix.ele[2][0] = this->ele[1][0] * this->ele[2][1] - this->ele[2][0] * this->ele[1][1];
	matrix.ele[2][1] = this->ele[0][1] * this->ele[2][0] - this->ele[2][1] * this->ele[0][0];
	matrix.ele[2][2] = this->ele[0][0] * this->ele[1][1] - this->ele[1][0] * this->ele[0][1];

	matrix.Scale(scale);

	return true;
}

void Matrix3x3::SetTranspose(const Matrix3x3& matrix)
{
	matrix.GetTranspose(*this);
}

void Matrix3x3::GetTranspose(Matrix3x3& matrix) const
{
	assert(this != &matrix);
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			matrix.ele[i][j] = this->ele[j][i];
}

void Matrix3x3::SetProduct(const Matrix3x3& matrixA, const Matrix3x3& matrixB)
{
	Vector row[3], col[3];
	for (int i = 0; i < 3; i++)
	{
		row[i] = matrixA.GetRow(i);
		col[i] = matrixB.GetCol(i);
	}

	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			this->ele[i][j] = Vector::Dot(row[i], col[j]);
}

void Matrix3x3::MultiplyLeft(const Vector& inVector, Vector& outVector) const
{
	Vector col[3];
	for (int j = 0; j < 3; j++)
		col[j] = this->GetCol(j);

	outVector.x = Vector::Dot(inVector, col[0]);
	outVector.x = Vector::Dot(inVector, col[1]);
	outVector.x = Vector::Dot(inVector, col[2]);
}

void Matrix3x3::MultiplyRight(const Vector& inVector, Vector& outVector) const
{
	Vector row[3];
	for (int i = 0; i < 3; i++)
		row[i] = this->GetRow(i);

	outVector.x = Vector::Dot(inVector, row[0]);
	outVector.y = Vector::Dot(inVector, row[1]);
	outVector.z = Vector::Dot(inVector, row[2]);
}

Vector Matrix3x3::operator*(const Vector& vector) const
{
	Vector result;
	this->MultiplyRight(vector, result);
	return result;
}

double Matrix3x3::Determinant(void) const
{
	return
		+ this->ele[0][0] * (this->ele[1][1] * this->ele[2][2] - this->ele[2][1] * this->ele[1][2])
		- this->ele[0][1] * (this->ele[1][0] * this->ele[2][2] - this->ele[2][0] * this->ele[1][2])
		+ this->ele[0][2] * (this->ele[1][0] * this->ele[2][1] - this->ele[2][0] * this->ele[1][1]);
}

double Matrix3x3::Trace(void) const
{
	return this->ele[0][0] + this->ele[1][1] + this->ele[2][2];
}

void Matrix3x3::Scale(double scale)
{
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			this->ele[i][j] *= scale;
}