#include "BoundingBoxTree.h"

using namespace MeshWarrior;

BoundingBoxTree::BoundingBoxTree()
{
	this->rootNode = nullptr;
}

/*virtual*/ BoundingBoxTree::~BoundingBoxTree()
{
	delete rootNode;
}

void BoundingBoxTree::SetRootBox(const AxisAlignedBox& box)
{
	if (!this->rootNode)
	{
		this->rootNode = new Node();
		this->rootNode->boundingBox = box;
	}
}

bool BoundingBoxTree::AddGuest(Guest* guest)
{
	if (!this->rootNode)
		return false;

	return this->rootNode->AddGuest(guest);
}

void BoundingBoxTree::FindGuests(const AxisAlignedBox& box, std::list<Guest*>& foundGuestList) const
{
	foundGuestList.clear();

	if (this->rootNode)
		this->rootNode->FindGuests(box, foundGuestList);
}

int BoundingBoxTree::TotalGuests() const
{
	int total = 0;

	if (this->rootNode)
		this->rootNode->TallyGuests(total);

	return total;
}

void BoundingBoxTree::GatherAllGuests(std::list<Guest*>& givenGuestList) const
{
	givenGuestList.clear();

	if (this->rootNode)
		this->rootNode->GatherAllGuests(givenGuestList);
}

void BoundingBoxTree::Clear()
{
	delete this->rootNode;
	this->rootNode = nullptr;
}

BoundingBoxTree::Guest::Guest()
{
}

/*virtual*/ BoundingBoxTree::Guest::~Guest()
{
}

BoundingBoxTree::Node::Node()
{
	this->node[0] = nullptr;
	this->node[1] = nullptr;
}

/*virtual*/ BoundingBoxTree::Node::~Node()
{
	delete this->node[0];
	delete this->node[1];
}

bool BoundingBoxTree::Node::AddGuest(Guest* guest)
{
	if (!this->boundingBox.ContainsBox(guest->CalcBoundingBox()))
		return false;

	if (!this->node[0])
	{
		this->node[0] = new Node();
		this->node[1] = new Node();

		this->boundingBox.SplitReasonably(this->node[0]->boundingBox, this->node[1]->boundingBox);
	}

	for (int i = 0; i < 2; i++)
		if (this->node[i]->AddGuest(guest))
			return true;

	this->guestList.push_back(guest);
	return true;
}

void BoundingBoxTree::Node::FindGuests(const AxisAlignedBox& box, std::list<Guest*>& foundGuestList) const
{
	if (!this->boundingBox.OverlapsWith(box))
		return;

	for (int i = 0; i < 2; i++)
		if (this->node[i])
			this->node[i]->FindGuests(box, foundGuestList);

	for (Guest* guest : this->guestList)
		if (box.OverlapsWith(guest->CalcBoundingBox()))
			foundGuestList.push_back(guest);
}

void BoundingBoxTree::Node::TallyGuests(int& tally) const
{
	tally += (int)this->guestList.size();

	for (int i = 0; i < 2; i++)
		if (this->node[i])
			this->node[i]->TallyGuests(tally);
}

void BoundingBoxTree::Node::GatherAllGuests(std::list<Guest*>& givenGuestList) const
{
	for (Guest* guest : this->guestList)
		givenGuestList.push_back(guest);

	for (int i = 0; i < 2; i++)
		if (this->node[i])
			this->node[i]->GatherAllGuests(givenGuestList);
}