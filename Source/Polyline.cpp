#include "Polyline.h"
#include "Shape.h"
#include "Compressor.h"

using namespace MeshWarrior;

Polyline::Polyline()
{
	this->vertexArray = new std::vector<Vector>();
}

/*virtual*/ Polyline::~Polyline()
{
	delete this->vertexArray;
}

/*static*/ void Polyline::GeneratePolylines(const std::vector<LineSegment*>& lineSegmentArray, std::vector<Polyline*>& polylineArray, double eps /*= 1e-6*/)
{
	polylineArray.clear();

	for (const LineSegment* lineSegment : lineSegmentArray)
	{
		Polyline* polyline = new Polyline();
		polyline->vertexArray->push_back(lineSegment->GetPoint(0));
		polyline->vertexArray->push_back(lineSegment->GetPoint(1));
		polylineArray.push_back(polyline);
	}

	CompressArray<Polyline>(polylineArray, [=](const Polyline* polylineA, const Polyline* polylineB) -> Polyline* {
		const Vector& firstPointA = (*polylineA->vertexArray)[0];
		const Vector& firstPointB = (*polylineB->vertexArray)[0];
		const Vector& lastPointA = (*polylineA->vertexArray)[polylineA->vertexArray->size() - 1];
		const Vector& lastPointB = (*polylineB->vertexArray)[polylineB->vertexArray->size() - 1];

		Polyline* polyline = nullptr;

		if ((lastPointA - firstPointB).Length() < eps)
		{
			polyline = new Polyline();
			for (int i = 0; i < (int)polylineA->vertexArray->size(); i++)
				polyline->vertexArray->push_back((*polylineA->vertexArray)[i]);
			for (int i = 1; i < (int)polylineB->vertexArray->size(); i++)
				polyline->vertexArray->push_back((*polylineB->vertexArray)[i]);
		}
		else if ((firstPointA - lastPointB).Length() < eps)
		{
			polyline = new Polyline();
			for (int i = 0; i < (int)polylineB->vertexArray->size(); i++)
				polyline->vertexArray->push_back((*polylineB->vertexArray)[i]);
			for (int i = 1; i < (int)polylineA->vertexArray->size(); i++)
				polyline->vertexArray->push_back((*polylineA->vertexArray)[i]);
		}
		else if ((firstPointA - firstPointB).Length() < eps)
		{
			polyline = new Polyline();
			for (int i = (int)polylineA->vertexArray->size() - 1; i >= 0; i--)
				polyline->vertexArray->push_back((*polylineA->vertexArray)[i]);
			for (int i = 1; i < (int)polylineB->vertexArray->size(); i++)
				polyline->vertexArray->push_back((*polylineB->vertexArray)[i]);
		}
		else if ((lastPointA - lastPointB).Length() < eps)
		{
			polyline = new Polyline();
			for (int i = 0; i < (int)polylineA->vertexArray->size(); i++)
				polyline->vertexArray->push_back((*polylineA->vertexArray)[i]);
			for (int i = (int)polylineB->vertexArray->size() - 2; i >= 0; i--)
				polyline->vertexArray->push_back((*polylineB->vertexArray)[i]);
		}

		if (polyline)
		{
			delete polylineA;
			delete polylineB;
		}

		return polyline;
	});
}

void Polyline::Reduce()
{
	// TODO: Remove triples of colinear points until none remain.
}