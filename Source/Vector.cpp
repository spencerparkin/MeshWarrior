#include "Vector.h"

using namespace MeshWarrior;

Vector::Vector()
{
	this->x = 0.0;
	this->y = 0.0;
	this->z = 0.0;
}

Vector::Vector(double x, double y, double z)
{
	this->x = x;
	this->y = y;
	this->z = z;
}

Vector::Vector(const Vector& vector)
{
	this->x = vector.x;
	this->y = vector.y;
	this->z = vector.z;
}

/*virtual*/ Vector::~Vector()
{
}

double Vector::Length() const
{
	return ::sqrt(Dot(*this, *this));
}

Vector& Vector::Normalize(bool* divByZero /*= nullptr*/, double* length /*= nullptr*/)
{
	if (divByZero)
		*divByZero = false;

	double lengthStorage;
	if (!length)
		length = &lengthStorage;

	*length = this->Length();
	if (*length == 0.0)
	{
		if (divByZero)
			*divByZero = true;
	}
	else
	{
		double scale = 1.0 / *length;
		if (scale != scale || ::isinf(scale) || ::isnan(scale))
		{
			if (divByZero)
				*divByZero = true;
		}
		else
		{
			this->Scale(scale);
		}
	}

	return *this;
}

Vector& Vector::Scale(double scalar)
{
	this->x *= scalar;
	this->y *= scalar;
	this->z *= scalar;
	return *this;
}

Vector& Vector::Cross(const Vector& vectorA, const Vector& vectorB)
{
	this->x = vectorA.y * vectorB.z - vectorA.z * vectorB.y;
	this->y = vectorA.z * vectorB.x - vectorA.x * vectorB.z;
	this->z = vectorA.x * vectorB.y - vectorA.y * vectorB.x;
	return *this;
}

/*static*/ double Vector::Dot(const Vector& vectorA, const Vector& vectorB)
{
	return vectorA.x * vectorB.x + vectorA.y * vectorB.y + vectorA.z * vectorB.z;
}

Vector& Vector::Add(const Vector& vectorA, const Vector& vectorB)
{
	this->x = vectorA.x + vectorB.x;
	this->y = vectorA.y + vectorB.y;
	this->z = vectorA.z + vectorB.z;
	return *this;
}

Vector& Vector::Subtract(const Vector& vectorA, const Vector& vectorB)
{
	this->x = vectorA.x - vectorB.x;
	this->y = vectorA.y - vectorB.y;
	this->z = vectorA.z - vectorB.z;
	return *this;
}

Vector& Vector::Multiply(const Vector& vector, double scalar)
{
	this->x = vector.x * scalar;
	this->y = vector.y * scalar;
	this->z = vector.z * scalar;
	return *this;
}

Vector& Vector::Divide(const Vector& vector, double scalar)
{
	this->x = vector.x / scalar;
	this->y = vector.y / scalar;
	this->z = vector.z / scalar;
	return *this;
}

Vector& Vector::Project(const Vector& vectorA, const Vector& vectorB)
{
	this->Multiply(vectorB, Dot(vectorB, vectorA));
	return *this;
}

Vector& Vector::Reject(const Vector& vectorA, const Vector& vectorB)
{
	Vector projection;
	projection.Project(vectorA, vectorB);
	this->Subtract(vectorA, projection);
	return *this;
}

Vector& Vector::Rotate(const Vector& vector, const Vector& unitLengthAxis, double angleRadians)
{
	Vector xAxis, yAxis;
	double radius;
	Vector parallelComponent, perpendicularComponent;
	
	// No rotation is necessary if the rejection is zero.
	bool divByZero = false;
	parallelComponent.Project(vector, unitLengthAxis);
	xAxis.Subtract(vector, parallelComponent).Normalize(&divByZero, &radius);
	if (!divByZero)
	{
		yAxis.Cross(unitLengthAxis, xAxis).Normalize();
		Vector perpendicularComponent = (xAxis * ::cos(angleRadians) + yAxis * ::sin(angleRadians)) * radius;
		this->Add(parallelComponent, perpendicularComponent);
	}

	return *this;
}

Vector& Vector::Min(const Vector& vectorA, const Vector& vectorB)
{
	this->x = MW_MIN(vectorA.x, vectorB.x);
	this->y = MW_MIN(vectorA.y, vectorB.y);
	this->z = MW_MIN(vectorA.z, vectorB.z);
	return *this;
}

Vector& Vector::Max(const Vector& vectorA, const Vector& vectorB)
{
	this->x = MW_MAX(vectorA.x, vectorB.x);
	this->y = MW_MAX(vectorA.y, vectorB.y);
	this->z = MW_MAX(vectorA.z, vectorB.z);
	return *this;
}

void Vector::operator=(const Vector& vector)
{
	this->x = vector.x;
	this->y = vector.y;
	this->z = vector.z;
}

void Vector::operator+=(const Vector& vector)
{
	this->x += vector.x;
	this->y += vector.y;
	this->z += vector.z;
}

void Vector::operator-=(const Vector& vector)
{
	this->x -= vector.x;
	this->y -= vector.y;
	this->z -= vector.z;
}

void Vector::operator*=(double scalar)
{
	this->x *= scalar;
	this->y *= scalar;
	this->z *= scalar;
}

void Vector::operator/=(double scalar)
{
	this->x /= scalar;
	this->y /= scalar;
	this->z /= scalar;
}