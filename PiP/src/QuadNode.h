#pragma once
#include "PiPMath.h"
#include <vector>

class QuadNode
{
public:
	QuadNode(math::Vector2 topRight = math::Vector2(), math::Vector2 bottomLeft = math::Vector2(), bool isLeaf = true);
	~QuadNode();

	//Deletes children, becomes a leaf node (Shouldn't be called on QNodes too far up in the hierarchy? Should only merge one level per frame (Spacetime coherency should help this))
	unsigned int GetLeafNodes(std::vector<QuadNode*>& leafNodes);
	void TrySubdivide();//See if conditions are fulfilled for subdividing this leaf node into 4 children
	void TryMerge();//See if conditions are fulfilled for merging children nodes on this leaf nodes parent

	math::Vector2 m_topRight;
	math::Vector2 m_bottomLeft;
	bool m_isLeaf;
	std::vector<Rigidbody*> m_ownedBodies;//Data owned by memory allocator
	QuadNode* m_children;
	QuadNode* m_owner;
};

