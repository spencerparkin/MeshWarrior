#include "Polygon.h"
#include "Compressor.h"
#include "Ray.h"

using namespace MeshWarrior;

//--------------------------------- Polygon ---------------------------------

Polygon::Polygon()
{
	this->vertexArray = new std::vector<Vector>();
}

Polygon::Polygon(const Polygon& polygon)
{
	this->vertexArray = new std::vector<Vector>();
	*this->vertexArray = *polygon.vertexArray;
}

/*virtual*/ Polygon::~Polygon()
{
	delete this->vertexArray;
}

void Polygon::operator=(const Polygon& polygon)
{
	*this->vertexArray = *polygon.vertexArray;
}

// Note that we're not catching certain cases of degenerate polygons here.
/*virtual*/ bool Polygon::IsValid(double eps /*= MW_EPS*/) const
{
	if (this->vertexArray->size() < 3)
		return false;

	Plane plane;
	if (!this->CalcPlane(plane))
		return false;

	// Make sure they're all approximately co-planar.
	for (const Vector& vertex : *this->vertexArray)
		if (!plane.ContainsPoint(vertex, 10.0 * eps))	// Why ten?  I don't know.
			return false;

	return true;
}

// This is essentially the case where the polygon has zero area.
// Rather than calculate area, though, here we will check to see
// if all points are co-linear.
/*virtual*/ bool Polygon::IsDegenerate(double eps /*= MW_EPS*/) const
{
	if (this->vertexArray->size() < 3)
		return true;

	Vector vertexA, vertexB;
	int chosen_i = -1, chosen_j = -1;
	double largestDistance = FLT_MIN;
	for (int i = 0; i < (int)this->vertexArray->size(); i++)
	{
		vertexA = (*this->vertexArray)[i];
		for (int j = i + 1; j < (int)this->vertexArray->size(); j++)
		{
			vertexB = (*this->vertexArray)[j];
			double distance = (vertexA - vertexB).Length();
			if (distance > largestDistance)
			{
				largestDistance = distance;
				chosen_i = i;
				chosen_j = j;
			}
		}
	}
	
	if (largestDistance < MW_EPS)
		return true;

	vertexA = (*this->vertexArray)[chosen_i];
	vertexB = (*this->vertexArray)[chosen_j];
	LineSegment lineSegment(vertexA, vertexB);
	for (int i = 0; i < (int)this->vertexArray->size(); i++)
		if (!lineSegment.ContainsPoint((*this->vertexArray)[i], eps))
			return false;

	return true;
}

/*virtual*/ double Polygon::ShortestSignedDistanceToPoint(const Vector& point) const
{
	// TODO: This is non-trivial as it requires tessellation.
	return 0.0;
}

/*virtual*/ bool Polygon::ContainsPoint(const Vector& point, double eps /*= MW_EPS*/) const
{
	// TODO: This is non-trivial as it requires tessellation.
	return false;
}

/*virtual*/ double Polygon::CalcArea() const
{
	// TODO: This is non-trivial as it requires tessellation.
	return 0.0f;
}

void Polygon::ReverseWinding()
{
	for (int i = 0; i < (signed)this->vertexArray->size() / 2; i++)
	{
		int j = (int)this->vertexArray->size() - 1 - i;
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

/*virtual*/ bool ConvexPolygon::IsValid(double eps /*= MW_EPS*/) const
{
	if (!Polygon::IsValid(eps))
		return false;

	std::vector<Plane> edgePlaneArray;
	this->GenerateEdgePlaneArray(edgePlaneArray);

	for (int i = 0; i < (signed)edgePlaneArray.size(); i++)
	{
		Plane& edgePlane = edgePlaneArray[i];
		for (int j = 0; j < (signed)this->vertexArray->size(); j++)
		{
			double distance = edgePlane.ShortestSignedDistanceToPoint((*this->vertexArray)[j]);
			if (distance > eps)
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
	polygonArray.clear();

	ConvexPolygon polygon;

	std::vector<Vector> workingVertexArray = *this->vertexArray;
	while (workingVertexArray.size() > 3)
	{
		polygon.vertexArray->clear();

		int bestVertex = -1;
		double maxTriangleArea = FLT_MIN;
		for (int i = 0; i < (int)workingVertexArray.size(); i++)
		{
			int j = (i + 1) % workingVertexArray.size();
			int k = (i + 2) % workingVertexArray.size();

			const Vector& vertexA = workingVertexArray[i];
			const Vector& vertexB = workingVertexArray[j];
			const Vector& vertexC = workingVertexArray[k];

			Vector vector;
			vector.Cross(vertexB - vertexA, vertexC - vertexA);
			double triangleArea = vector.Length() / 2.0;

			if (triangleArea > maxTriangleArea)
			{
				maxTriangleArea = triangleArea;
				bestVertex = i;
			}
		}

		int i = bestVertex;
		int j = (i + 1) % workingVertexArray.size();
		int k = (i + 2) % workingVertexArray.size();

		const Vector& vertexA = workingVertexArray[i];
		const Vector& vertexB = workingVertexArray[j];
		const Vector& vertexC = workingVertexArray[k];

		polygon.vertexArray->push_back(vertexA);
		polygon.vertexArray->push_back(vertexB);
		polygon.vertexArray->push_back(vertexC);

		polygonArray.push_back(polygon);
		workingVertexArray.erase(workingVertexArray.begin() + j);
	}

	*polygon.vertexArray = workingVertexArray;
	polygonArray.push_back(polygon);
}

/*virtual*/ double ConvexPolygon::ShortestSignedDistanceToPoint(const Vector& point) const
{
	// TODO: Write this.  Shouldn't be too hard.
	return 0.0;
}

/*virtual*/ bool ConvexPolygon::ContainsPoint(const Vector& point, double eps /*= MW_EPS*/) const
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

/*virtual*/ bool ConvexPolygon::RayCast(const Ray& ray, double& rayAlpha) const
{
	Plane plane;
	this->CalcPlane(plane);
	if (!plane.RayCast(ray, rayAlpha))
		return false;

	Vector rayPoint = ray.CalcRayPoint(rayAlpha);
	return this->ContainsPoint(rayPoint);
}

/*virtual*/ double ConvexPolygon::CalcArea() const
{
	// TODO: Write this.  Should be easy.  Just add up area of triangles made from center with edges.
	return 0.0f;
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
		// We just care about a non-trivial intersection case.

		std::vector<Point*> pointList;
		const Polygon* polygons[2] = { this, polygon };

		for (int i = 0; i < 2; i++)
		{
			const Polygon* polygonA = polygons[i];
			const Polygon* polygonB = polygons[1 - i];

			Plane plane;
			polygonB->CalcPlane(plane);

			for (int j = 0; j < (signed)polygonA->vertexArray->size(); j++)
			{
				int k = (j + 1) % polygonA->vertexArray->size();
				LineSegment edge((*polygonA->vertexArray)[j], (*polygonA->vertexArray)[k]);
				Point* point = (Point*)plane.IntersectWith(&edge);
				if (point)
				{
					if (polygonB->ContainsPoint(point->center))
						pointList.push_back(point);
					else
						delete point;
				}
			}
		}

		// Remove redundant points to see how many there really are.
		CompressArray<Point>(pointList, [](const Point* pointA, const Point* pointB) -> Point* {
			if (pointA->ContainsPoint(pointB->center))
			{
				delete pointB;
				return const_cast<Point*>(pointA);
			}
			return nullptr;
		});

		if (pointList.size() == 2)
			intersection = new LineSegment(pointList[0]->center, pointList[1]->center);
	
		for (Point* point : pointList)
			delete point;
	}

	return intersection;
}

void ConvexPolygon::AddMeshPolygon(std::vector<Mesh::ConvexPolygon>& polygonList, const Vector& color) const
{
	Mesh::ConvexPolygon polygon;

	Plane plane;
	this->CalcPlane(plane);

	for (int i = 0; i < (signed)this->vertexArray->size(); i++)
	{
		Mesh::Vertex vertex;
		vertex.point = (*this->vertexArray)[i];
		vertex.color = color;
		vertex.normal = plane.unitNormal;
		polygon.vertexArray.push_back(vertex);
	}

	polygonList.push_back(polygon);
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
			if (!point->ContainsPoint(edge.GetPoint(0)) && !point->ContainsPoint(edge.GetPoint(1)))
				pointArray.push_back(point->center);
			delete point;
		}
	}

	ConvexPolygon polygonFront, polygonBack;

	for (int i = 0; i < (signed)pointArray.size(); i++)
	{
		Vector& point = pointArray[i];
		double distance = plane.ShortestSignedDistanceToPoint(point);
		if (::fabs(distance) <= MW_EPS)
		{
			polygonFront.vertexArray->push_back(point);
			polygonBack.vertexArray->push_back(point);
		}
		else if (distance > 0.0)
			polygonFront.vertexArray->push_back(point);
		else if (distance < 0.0)
			polygonBack.vertexArray->push_back(point);
	}

	bool polygonFrontDegenerate = polygonFront.IsDegenerate();
	bool polygonBackDegenerate = polygonBack.IsDegenerate();

	if (!(polygonFrontDegenerate || polygonBackDegenerate))
	{
		polygonArray.push_back(polygonFront);
		polygonArray.push_back(polygonBack);
	}

	return polygonArray.size() > 0;
}