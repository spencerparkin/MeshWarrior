#include "AxisAlignedBox.h"
#include <float.h>

using namespace MeshWarrior;

AxisAlignedBox::AxisAlignedBox()
{
	this->min.x = FLT_MIN;
	this->min.y = FLT_MIN;
	this->max.x = FLT_MAX;
	this->max.y = FLT_MAX;
}

AxisAlignedBox::AxisAlignedBox(const Vector& point)
{
	this->min = point;
	this->max = point;
}

AxisAlignedBox::AxisAlignedBox(const Vector& min, const Vector& max)
{
	this->min = min;
	this->max = max;
}

AxisAlignedBox::AxisAlignedBox(const AxisAlignedBox& box)
{
	this->min = box.min;
	this->max = box.max;
}

/*virtual*/ AxisAlignedBox::~AxisAlignedBox()
{
}

bool AxisAlignedBox::IsValid() const
{
	if (this->min.x > this->max.x)
		return false;

	if (this->min.y > this->max.y)
		return false;

	if (this->min.z > this->max.z)
		return false;

	return true;
}

bool AxisAlignedBox::ContainsPoint(const Vector& point) const
{
	if (this->min.x <= point.x && point.x <= this->max.x &&
		this->min.y <= point.y && point.y <= this->max.y &&
		this->min.z <= point.z && point.z <= this->max.z)
	{
		return true;
	}

	return false;
}

bool AxisAlignedBox::ContainsPointOnBoundary(const Vector& point, double eps /*= 1e-6*/) const
{
	AxisAlignedBox outerBox(*this), innerBox(*this);
	outerBox.ScaleAboutCenter(eps / 2.0);
	innerBox.ScaleAboutCenter(-eps / 2.0);
	return outerBox.ContainsPoint(point) && !innerBox.ContainsPoint(point);
}

bool AxisAlignedBox::ContainsBox(const AxisAlignedBox& box) const
{
	return this->ContainsPoint(box.min) && this->ContainsPoint(box.max);
}

void AxisAlignedBox::ScaleAboutCenter(double delta)
{
	Vector center = this->CalcCenter();
	Vector cornerDelta = this->max - center;
	double length = 0.0;
	bool divByZero = false;
	cornerDelta.Normalize(&divByZero, &length);
	if (!divByZero)
	{
		length += delta;
		cornerDelta *= length;
		this->min = center + cornerDelta;
		this->max = center - cornerDelta;
	}
}

Vector AxisAlignedBox::CalcCenter() const
{
	return (this->min + this->max) / 2.0;
}

double AxisAlignedBox::CalcWidth() const
{
	return this->max.x - this->min.x;
}

double AxisAlignedBox::CalcHeight() const
{
	return this->max.y - this->min.y;
}

double AxisAlignedBox::CalcDepth() const
{
	return this->max.z - this->min.z;
}

double AxisAlignedBox::CalcVolume() const
{
	return this->CalcWidth() * this->CalcHeight() * this->CalcDepth();
}

void AxisAlignedBox::MinimallyExpandToContainPoint(const Vector& point)
{
	if (this->min.x == FLT_MIN && this->min.y == FLT_MIN &&
		this->max.x == FLT_MAX && this->max.x == FLT_MAX)
	{
		this->min = point;
		this->max = point;
	}
	else
	{
		this->min.Min(this->min, point);
		this->max.Max(this->max, point);
	}
}

void AxisAlignedBox::MinimallyExpandToContainBox(const AxisAlignedBox& box)
{
	this->MinimallyExpandToContainPoint(box.min);
	this->MinimallyExpandToContainPoint(box.max);
}

void AxisAlignedBox::Combine(const AxisAlignedBox& boxA, const AxisAlignedBox& boxB)
{
	this->min.Min(boxA.min, boxB.min);
	this->max.Max(boxA.max, boxB.max);
}

bool AxisAlignedBox::Intersect(const AxisAlignedBox& boxA, const AxisAlignedBox& boxB)
{
	this->min.Max(boxA.min, boxB.min);
	this->max.Min(boxA.max, boxB.max);
	return this->IsValid();
}

bool AxisAlignedBox::OverlapsWith(const AxisAlignedBox& box) const
{
	AxisAlignedBox intersection;
	return intersection.Intersect(*this, box);
}

void AxisAlignedBox::SplitReasonably(AxisAlignedBox& boxA, AxisAlignedBox& boxB) const
{
	double width = this->CalcWidth();
	double height = this->CalcHeight();
	double depth = this->CalcDepth();

	double maxDimension = MW_MAX(width, MW_MAX(height, depth));

	Vector center = this->CalcCenter();

	boxA.min = this->min;
	boxA.max = this->max;
	boxB.min = this->min;
	boxB.max = this->max;

	if (width == maxDimension)
	{
		boxA.max.x = center.x;
		boxB.min.x = center.x;
	}
	else if (height == maxDimension)
	{
		boxA.max.y = center.y;
		boxB.min.y = center.y;
	}
	else if (depth == maxDimension)
	{
		boxA.max.z = center.z;
		boxB.min.z = center.z;
	}
}