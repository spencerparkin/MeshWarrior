#include "Quaternion.h"

using namespace MeshWarrior;

Quaternion::Quaternion()
{
	this->x = 0.0;
	this->y = 0.0;
	this->z = 0.0;
	this->w = 1.0;
}

/*virtual*/ Quaternion::~Quaternion()
{
}

Vector Quaternion::GetRotationAxis() const
{
	// TODO: Write this.
	return Vector(0.0, 0.0, 0.0);
}

double Quaternion::GetRotationAngle() const
{
	// TODO: Write this.
	return 0.0;
}

Vector Quaternion::RotateVector(const Vector& vector) const
{
	// TODO: Write this.
	return Vector(0.0, 0.0, 0.0);
}