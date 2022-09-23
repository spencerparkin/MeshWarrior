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
		void FindGuests(const AxisAlignedBox& box, std::list<Guest*>& foundGuestList);
		void Clear();

	private:
		
		class Node
		{
		public:
			Node();
			virtual ~Node();

			bool AddGuest(Guest* guest);
			void FindGuests(const AxisAlignedBox& box, std::list<Guest*>& foundGuestList);

			AxisAlignedBox boundingBox;
			Node* node[2];
			std::list<Guest*> guestList;
		};

		Node* rootNode;
	};
}