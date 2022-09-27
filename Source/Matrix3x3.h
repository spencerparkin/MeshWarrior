#pragma once

#include "Defines.h"
#include "Vector.h"
#include "Quaternion.h"

namespace MeshWarrior
{
	class MESH_WARRIOR_API Matrix3x3
	{
	public:

		Matrix3x3();
		virtual ~Matrix3x3();

		void SetIdentity(void);

		void SetRow(int i, const Vector& vector);
		void SetCol(int j, const Vector& vector);
		Vector GetRow(int i) const;
		Vector GetCol(int j) const;

		void Orthonormalize(void);

		void SetCopy(const Matrix3x3& matrix);
		void GetCopy(Matrix3x3& matrix) const;

		void SetFromAxisAngle(const Vector& axis, double angle);
		bool GetToAxisAngle(Vector& axis, double& angle) const;

		void SetFromQuat(const Quaternion& quat);
		bool GetToQuat(Quaternion& quat) const; // Must be orthonormal with determinant +1 for this to work.

		bool SetInverse(const Matrix3x3& matrix);
		bool GetInverse(Matrix3x3& matrix) const;

		void SetTranspose(const Matrix3x3& matrix);
		void GetTranspose(Matrix3x3& matrix) const;

		void SetProduct(const Matrix3x3& matrixA, const Matrix3x3& matrixB);

		void MultiplyLeft(const Vector& inVector, Vector& outVector) const;
		void MultiplyRight(const Vector& inVector, Vector& outVector) const;

		Vector operator*(const Vector& vector) const;

		double Determinant(void) const;
		double Trace(void) const;

		void Scale(double scale);

		double ele[3][3];
	};
}