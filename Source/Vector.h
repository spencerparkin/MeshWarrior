#pragma once

#include "Defines.h"
#include <math.h>

namespace MeshWarrior
{
	class MESH_WARRIOR_API Vector
	{
	public:
		Vector();
		Vector(double x, double y, double z);
		Vector(const Vector& vector);
		virtual ~Vector();

		double Length() const;
		Vector& Normalize(bool* divByZero = nullptr, double* length = nullptr);
		Vector& Scale(double scalar);
		Vector& Cross(const Vector& vectorA, const Vector& vectorB);
		static double Dot(const Vector& vectorA, const Vector& vectorB);
		Vector& Add(const Vector& vectorA, const Vector& vectorB);
		Vector& Subtract(const Vector& vectorA, const Vector& vectorB);
		Vector& Multiply(const Vector& vector, double scalar);
		Vector& Divide(const Vector& vector, double scalar);
		Vector& Project(const Vector& vectorA, const Vector& vectorB);
		Vector& Reject(const Vector& vectorA, const Vector& vectorB);
		Vector& Rotate(const Vector& vector, const Vector& unitLengthAxis, float angleRadians);
		Vector& Min(const Vector& vectorA, const Vector& vectorB);
		Vector& Max(const Vector& vectorA, const Vector& vectorB);

		void operator=(const Vector& vector);
		void operator+=(const Vector& vector);
		void operator-=(const Vector& vector);
		void operator*=(double scalar);
		void operator/=(double scalar);

		double x, y, z;
	};

	inline Vector operator+(const Vector& vectorA, const Vector& vectorB)
	{
		Vector sum;
		sum.Add(vectorA, vectorB);
		return sum;
	}

	inline Vector operator-(const Vector& vectorA, const Vector& vectorB)
	{
		Vector difference;
		difference.Subtract(vectorA, vectorB);
		return difference;
	}

	inline Vector operator^(const Vector& vectorA, const Vector& vectorB)
	{
		Vector crossProduct;
		crossProduct.Cross(vectorA, vectorB);
		return crossProduct;
	}

	inline Vector operator*(const Vector& vector, double scalar)
	{
		Vector scalarProduct;
		scalarProduct.Multiply(vector, scalar);
		return scalarProduct;
	}

	inline Vector operator/(const Vector& vector, double scalar)
	{
		Vector scalarProduct;
		scalarProduct.Divide(vector, scalar);
		return scalarProduct;
	}
}