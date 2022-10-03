#pragma once

#include "Defines.h"
#include "AxisAlignedBox.h"
#include <list>

namespace MeshWarrior
{
	class MESH_WARRIOR_API BoundingBoxTree
	{
	public:
		BoundingBoxTree();
		virtual ~BoundingBoxTree();

		class Guest
		{
		public:
			Guest();
			virtual ~Guest();

			virtual AxisAlignedBox CalcBoundingBox() const = 0;
		};

		void SetRootBox(const AxisAlignedBox& box);
		bool AddGuest(Guest* guest);
		void FindGuests(const AxisAlignedBox& box, std::list<Guest*>& foundGuestList) const;
		void Clear();
		int TotalGuests() const;
		void GatherAllGuests(std::list<Guest*>& givenGuestList) const;

	private:
		
		class Node
		{
		public:
			Node();
			virtual ~Node();

			bool AddGuest(Guest* guest, int currentDepth, int maxDepth);
			void FindGuests(const AxisAlignedBox& box, std::list<Guest*>& foundGuestList) const;
			void TallyGuests(int& tally) const;
			void GatherAllGuests(std::list<Guest*>& givenGuestList) const;

			AxisAlignedBox boundingBox;
			Node* node[2];
			std::list<Guest*> guestList;
		};

		Node* rootNode;
	};
}