#include "Polygon.h"

using namespace MeshWarrior;

//--------------------------------- Polygon ---------------------------------

Polygon::Polygon()
{
	this->vertexArray = new std::vector<Vector>();
}

/*virtual*/ Polygon::~Polygon()
{
	delete this->vertexArray;
}

// Note that we're not catching certain cases of degenerate polygons here,
// such as those with zero area.
/*virtual*/ bool Polygon::IsValid() const
{
	if (this->vertexArray->size() < 3)
		return false;

	Plane plane;
	if (!this->CalcPlane(plane))
		return false;

	for (const Vector& vertex : *this->vertexArray)
		if (!plane.ContainsPoint(vertex))
			return false;

	return true;
}

/*virtual*/ double Polygon::ShortestSignedDistanceToPoint(const Vector& point) const
{
	// TODO: This is non-trivial, and requires tessellation.
	return 0.0;
}

/*virtual*/ bool Polygon::ContainsPoint(const Vector& point, double eps /*= 1e-5*/) const
{
	// TODO: This is non-trivial, and requires tessellation.
	return false;
}

void Polygon::ReverseWinding()
{
	for (int i = 0; i < (signed)this->vertexArray->size() / 2; i++)
	{
		int j = this->vertexArray->size() - 1 - i;
		Vector vertex = (*this->vertexArray)[i];
		(*this->vertexArray)[i] = (*this->vertexArray)[j];
		(*this->vertexArray)[j] = vertex;
	}
}

bool Polygon::CalcPlane(Plane& plane) const
{
	if (this->vertexArray->size() < 3)
		return false;
	
	// TODO: This might not be the most numerically stable choice.
	//       If the cross product length is too low, maybe just
	//       run a plane fit algorithm on the points?
	Vector vertexA = (*this->vertexArray)[0];
	Vector vertexB = (*this->vertexArray)[1];
	Vector vertexC = (*this->vertexArray)[2];

	plane.unitNormal.Cross(vertexB - vertexA, vertexC - vertexA);

	bool divByZero = false;
	plane.unitNormal.Normalize(&divByZero);
	if (divByZero)
		return false;

	plane.center = vertexA;
	return true;
}

// Find the plane that best fits our points.  This is convenient if the
// points in our array aren't really all coplanar.
bool Polygon::FitPlane(Plane& plane) const
{
	// TODO: Perform a least squares regression here.
	//       Note that this requires being able to calculate a 3x3 matrix inverse.
	return false;
}

// The projection of a polygon onto a plane can sometimes still be a valid polygon.
void Polygon::ProjectOntoPlane(Plane& plane)
{
	for (Vector& vertex : *this->vertexArray)
	{
		double distance = plane.ShortestSignedDistanceToPoint(vertex);
		vertex -= plane.unitNormal * distance;
	}
}

bool Polygon::SnapToNearestPlane()
{
	Plane plane;
	if (!this->FitPlane(plane))
		return false;

	this->ProjectOntoPlane(plane);
	return true;
}

Vector Polygon::CalcCenter() const
{
	Vector center(0.0, 0.0, 0.0);
	for (const Vector& vertex : *this->vertexArray)
		center += vertex;

	if (this->vertexArray->size() > 0)
		center.Scale(1.0 / double(this->vertexArray->size()));

	return center;
}

/*virtual*/ void Polygon::Tessellate(std::vector<ConvexPolygon>& polygonArray) const
{
	// TODO: Return a bunch of triangles that, together, form this polygon.
	//       This is non-trivial in the concave case.
}

//--------------------------------- ConvexPolygon ---------------------------------

ConvexPolygon::ConvexPolygon()
{
}

/*virtual*/ ConvexPolygon::~ConvexPolygon()
{
}

/*virtual*/ bool ConvexPolygon::IsValid() const
{
	if (!Polygon::IsValid())
		return false;

	std::vector<Plane> edgePlaneArray;
	this->GenerateEdgePlaneArray(edgePlaneArray);

	for (int i = 0; i < (signed)edgePlaneArray.size(); i++)
	{
		Plane& edgePlane = edgePlaneArray[i];
		for (int j = 0; j < (signed)this->vertexArray->size(); j++)
		{
			double distance = edgePlane.ShortestSignedDistanceToPoint((*this->vertexArray)[j]);
			if (distance > 0.0)
				return false;
		}
	}

	return true;
}

bool ConvexPolygon::GenerateEdgePlaneArray(std::vector<Plane>& edgePlaneArray) const
{
	Plane plane;
	if (!this->CalcPlane(plane))
		return false;
	
	edgePlaneArray.clear();

	for (int i = 0; i < (signed)this->vertexArray->size(); i++)
	{
		int j = (i + 1) % this->vertexArray->size();
		Vector vertexA = (*this->vertexArray)[i];
		Vector vertexB = (*this->vertexArray)[j];

		Plane edgePlane;
		edgePlane.center = vertexA;

		bool divByZero = false;
		edgePlane.unitNormal.Cross(vertexB - vertexA, plane.unitNormal).Normalize(&divByZero);
		if (divByZero)
			return false;

		edgePlaneArray.push_back(edgePlane);
	}

	return true;
}

// Note that the base-class implementation of this method should work, but
// we override it here, because we can do it faster knowing we're convex.
/*virtual*/ void ConvexPolygon::Tessellate(std::vector<ConvexPolygon>& polygonArray) const
{
	// TODO: Write this.
}

/*virtual*/ double ConvexPolygon::ShortestSignedDistanceToPoint(const Vector& point) const
{
	// TODO: Write this.  Shouldn't be too hard.
	return 0.0;
}

/*virtual*/ bool ConvexPolygon::ContainsPoint(const Vector& point, double eps /*= 1e-5*/) const
{
	Plane plane;
	this->CalcPlane(plane);
	if (!plane.ContainsPoint(point, eps))
		return false;

	std::vector<Plane> edgePlaneArray;
	this->GenerateEdgePlaneArray(edgePlaneArray);

	for (Plane& edgePlane : edgePlaneArray)
	{
		double distance = edgePlane.ShortestSignedDistanceToPoint(point);
		if (distance > eps)
			return false;
	}

	return true;
}

/*virtual*/ Shape* ConvexPolygon::IntersectWith(const Shape* shape) const
{
	Shape* intersection = nullptr;

	const ConvexPolygon* polygon = dynamic_cast<const ConvexPolygon*>(shape);
	if (polygon)
	{
		// We don't count here the case where the two polygons are the same polygon,
		// or the case where they share just a single point, or the case where just
		// an edge of one polygon is contained, fully or partially, in the other.

		Point* pointA = nullptr;
		Point* pointB = nullptr;

		Plane plane;
		polygon->CalcPlane(plane);

		for (int i = 0; i < (signed)this->vertexArray->size(); i++)
		{
			int j = (i + 1) % this->vertexArray->size();
			LineSegment edge((*this->vertexArray)[i], (*this->vertexArray)[j]);
			pointA = (Point*)plane.IntersectWith(&edge);
			if (pointA)
				break;
		}

		if (pointA)
		{
			this->CalcPlane(plane);

			for (int i = 0; i < (signed)polygon->vertexArray->size(); i++)
			{
				int j = (i + 1) % polygon->vertexArray->size();
				LineSegment edge((*this->vertexArray)[i], (*this->vertexArray)[j]);
				pointB = (Point*)plane.IntersectWith(&edge);
				if (pointB)
					break;
			}

			if (pointB)
			{
				intersection = new LineSegment(pointA->center, pointB->center);
				delete pointB;
			}

			delete pointA;
		}
	}

	return intersection;
}

bool ConvexPolygon::SplitAgainstPlane(const Plane& plane, std::vector<ConvexPolygon>& polygonArray) const
{
	std::vector<Vector> pointArray;

	for (int i = 0; i < (signed)this->vertexArray->size(); i++)
	{
		pointArray.push_back((*this->vertexArray)[i]);

		int j = (i + 1) % this->vertexArray->size();
		LineSegment edge((*this->vertexArray)[i], (*this->vertexArray)[j]);
		Point* point = (Point*)plane.IntersectWith(&edge);
		if (point)
		{
			pointArray.push_back(point->center);
			delete point;
		}
	}

	ConvexPolygon polygonFront, polygonBack;

	double eps = 1e-6;
	for (int i = 0; i < (signed)pointArray.size(); i++)
	{
		Vector& point = pointArray[i];
		double distance = plane.ShortestSignedDistanceToPoint(point);
		if (::fabs(distance) < eps)
		{
			polygonFront.vertexArray->push_back(point);
			polygonBack.vertexArray->push_back(point);
		}
		else if (distance > 0.0)
			polygonFront.vertexArray->push_back(point);
		else if (distance < 0.0)
			polygonBack.vertexArray->push_back(point);
	}

	if (polygonFront.vertexArray->size() >= 3 && polygonBack.vertexArray->size() >= 3)
	{
		polygonArray.push_back(polygonFront);
		polygonArray.push_back(polygonBack);
	}

	return polygonArray.size() > 0;
}