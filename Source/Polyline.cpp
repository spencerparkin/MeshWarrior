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
	// Not the most efficient way of doing it, but I don't imagine I'll be
	// processing polylines that are terrible big, so maybe it's fine.
	bool makeAnotherPass = true;
	while (makeAnotherPass)
	{
		makeAnotherPass = false;
		for (int i = 0; i < (int)this->vertexArray->size() - 2; i++)
		{
			LineSegment lineSegment((*this->vertexArray)[i], (*this->vertexArray)[i + 2]);
			if (lineSegment.ContainsPoint((*this->vertexArray)[i + 1]))
			{
				for (int j = i + 1; j < (int)this->vertexArray->size() - 1; j++)
					(*this->vertexArray)[j] = (*this->vertexArray)[j + 1];
				this->vertexArray->pop_back();
				makeAnotherPass = true;
			}
		}
	}
}

bool Polyline::HasVertex(const Vector& vertex, double eps /*= 1e-6*/) const
{
	for (int i = 0; i < (int)this->vertexArray->size(); i++)
		if (((*this->vertexArray)[i] - vertex).Length() <= eps)
			return true;

	return false;
}