/*
 * InfoNode.cpp
 *
 *  Created on: 19 feb 2015
 *      Author: Daniel
 */
#include "InfoNode.h"
#include "InfomapTypes.h"

namespace infomap {


InfoNode::~InfoNode()
{
	if (m_infomap != nullptr) {
		delete m_infomap;
	}

	deleteChildren();

	if (next != 0)
		next->previous = previous;
	if (previous != 0)
		previous->next = next;
	if (parent != 0)
	{
		if (parent->firstChild == this)
			parent->firstChild = next;
		if (parent->lastChild == this)
			parent->lastChild = previous;
	}

	// Delete outgoing edges.
	// TODO: Renders ingoing edges invalid. Assume or assert that all nodes on the same level are deleted?
	for (edge_iterator outEdgeIt(begin_outEdge());
			outEdgeIt != end_outEdge(); ++outEdgeIt)
	{
		delete *outEdgeIt;
	}

}


// InfomapBase& InfoNode::getInfomap(bool reset) {
// 	if (!m_infomap || reset)
// 		m_infomap = std::unique_ptr<InfomapBase>(new M1Infomap());
// 	return *m_infomap;
// }

// InfomapBase& InfoNode::getMemInfomap(bool reset) {
// 	if (!m_infomap || reset)
// 		m_infomap = std::unique_ptr<InfomapBase>(new M2Infomap());
// 	return *m_infomap;
// }

// InfoNode* InfoNode::getInfomapRoot() {
// 	return m_infomap? &m_infomap->root() : nullptr;
// }

// bool InfoNode::disposeInfomap()
// {
// 	if (m_infomap) {
// 		m_infomap.reset();
// 		return true;
// 	}
// 	return false;
// }

InfomapBase& InfoNode::getInfomap(bool reset) {
	if (!m_infomap || reset) {
		disposeInfomap();
		m_infomap = new M1Infomap();
	}
	return *m_infomap;
}

InfomapBase& InfoNode::getMemInfomap(bool reset) {
	if (!m_infomap || reset) {
		disposeInfomap();
		m_infomap = new M2Infomap();
	}
	return *m_infomap;
}

InfoNode* InfoNode::getInfomapRoot() {
	return m_infomap != nullptr? &m_infomap->root() : nullptr;
}

InfoNode const* InfoNode::getInfomapRoot() const {
	return m_infomap != nullptr? &m_infomap->root() : nullptr;
}

bool InfoNode::disposeInfomap()
{
	if (m_infomap != nullptr) {
		delete m_infomap;
		m_infomap = nullptr;
		return true;
	}
	return false;
}

bool InfoNode::isLeaf() const
{
	return firstChild == nullptr;
}

bool InfoNode::isLeafModule() const
{
	//TODO: Safe to assume all children are leaves if first child is leaf?
	return firstChild != nullptr && firstChild->firstChild == nullptr;
}

bool InfoNode::isRoot() const
{
	return parent == nullptr;
}

unsigned int InfoNode::depth() const
{
	unsigned int depth = 0;
	InfoNode* n = parent;
	while (n != nullptr) {
		++depth;
		n = n->parent;
	}
	return depth;
}

unsigned int InfoNode::childIndex() const
{
	unsigned int childIndex = 0;
	const InfoNode* n(this);
	while (n->previous) {
		n = n->previous;
		++childIndex;
	}
	return childIndex;
}

unsigned int InfoNode::childDegree() const
{
//	if (!m_childrenChanged)
		return m_childDegree;
//	calcChildDegree();
//	return m_childDegree;
}

void InfoNode::addChild(InfoNode* child)
{
//	m_childrenChanged = true;
	if (firstChild == 0)
	{
//		DEBUG_OUT("\t\t-->Add node " << *child << " as FIRST CHILD to parent " << *this << std::endl);
		child->previous = nullptr;
		firstChild = child;
	}
	else
	{
//		DEBUG_OUT("\t\t-->Add node " << *child << " as LAST CHILD to parent " << *this << std::endl);
		child->previous = lastChild;
		lastChild->next = child;
	}
	lastChild = child;
	child->next = nullptr;
	child->parent = this;
	++m_childDegree;
}

void InfoNode::releaseChildren()
{
	firstChild = nullptr;
	lastChild = nullptr;
	m_childDegree = 0;
}

unsigned int InfoNode::replaceChildrenWithGrandChildren()
{
	if (firstChild == nullptr)
		return 0;
	InfoNode::sibling_iterator nodeIt = begin_child();
	unsigned int numOriginalChildrenLeft = m_childDegree;
	unsigned int numChildrenReplaced = 0;
	do
	{
		InfoNode* n = nodeIt.base();
		++nodeIt;
		numChildrenReplaced += n->replaceWithChildren();
	}
	while (--numOriginalChildrenLeft != 0);
	return numChildrenReplaced;
}

unsigned int InfoNode::replaceWithChildren()
{
	if (isLeaf() || isRoot())
		return 0;

	// Reparent children
	unsigned int deltaChildDegree = 0;
	InfoNode* child = firstChild;
	do
	{
		child->parent = parent;
		child = child->next;
		++deltaChildDegree;
	} while (child != nullptr);
	parent->m_childDegree += deltaChildDegree - 1; // -1 as this node is deleted

	firstChild->previous = previous;
	lastChild->next = next;

	if (parent->firstChild == this) {
		parent->firstChild = firstChild;
	} else {
		previous->next = firstChild;
	}

	if (parent->lastChild == this) {
		parent->lastChild = lastChild;
	} else {
		next->previous = lastChild;
	}

	// Release connected nodes before delete, otherwise children are deleted and neighbours are reconnected.
	firstChild = nullptr;
	lastChild = nullptr;
	next = nullptr;
	previous = nullptr;
	parent = nullptr;
	delete this;
	return 1;
}

void InfoNode::replaceChildrenWithGrandChildrenDebug()
{
	if (firstChild == 0)
		return;
	InfoNode::sibling_iterator nodeIt = begin_child();
	unsigned int numOriginalChildrenLeft = m_childDegree;
	do
	{
		InfoNode* n = nodeIt.base();
		++nodeIt;
		n->replaceWithChildrenDebug();
	}
	while (--numOriginalChildrenLeft != 0);
}


void InfoNode::replaceWithChildrenDebug()
{
	if (isLeaf() || isRoot())
		return;


	// Reparent children
	unsigned int deltaChildDegree = 0;
	InfoNode* child = firstChild;
	do
	{
		child->parent = parent;
		child = child->next;
		++deltaChildDegree;
	} while (child != 0);
	parent->m_childDegree += deltaChildDegree - 1; // -1 as this node is deleted

	if (parent->firstChild == this) {
		parent->firstChild = firstChild;
	} else {
		previous->next = firstChild;
		firstChild->previous = previous;
	}

	if (parent->lastChild == this) {
		parent->lastChild = lastChild;
	} else {
		next->previous = lastChild;
		lastChild->next = next;
	}

	// Release connected nodes before delete, otherwise children are deleted and neighbours are reconnected.
	firstChild = nullptr;
	lastChild = nullptr;
	next = nullptr;
	previous = nullptr;
	parent = nullptr;

	delete this;
}

void InfoNode::remove(bool removeChildren)
{
	firstChild = removeChildren ? 0 : firstChild;
	delete this;
}

void InfoNode::deleteChildren()
{
	if (firstChild == nullptr)
		return;

	sibling_iterator nodeIt = begin_child();
	do
	{
		InfoNode* n = nodeIt.base();
		++nodeIt;
		delete n;
	}
	while (nodeIt.base() != nullptr);

	firstChild = nullptr;
	lastChild = nullptr;
	m_childDegree = 0;
}

void InfoNode::calcChildDegree()
{
	m_childrenChanged = false;
	if (firstChild == nullptr)
		m_childDegree = 0;
	else if (firstChild == lastChild)
		m_childDegree = 1;
	else
	{
		m_childDegree = 0;
		for (sibling_iterator childIt(begin_child()), endIt(end_child());
				childIt != endIt; ++childIt, ++m_childDegree);
	}
}

void InfoNode::setChildDegree(unsigned int value)
{
	m_childDegree = value;
	m_childrenChanged = false;
}

void InfoNode::setNumLeafNodes(unsigned int value)
{
	m_numLeafMembers = value;
}

void InfoNode::initClean()
{
	releaseChildren();
	previous = next = parent = nullptr;

	physicalNodes.clear();
}

unsigned int InfoNode::collapseChildren()
{
	std::swap(collapsedFirstChild, firstChild);
	std::swap(collapsedLastChild, lastChild);
	unsigned int numCollapsedChildren = childDegree();
	releaseChildren();
	return numCollapsedChildren;
}

unsigned int InfoNode::expandChildren()
{
	bool haveCollapsedChildren = collapsedFirstChild != nullptr;
	if (haveCollapsedChildren) {
		if (firstChild != nullptr || lastChild != nullptr)
			throw InternalOrderError("Expand collapsed children called on a node that already has children.");
		std::swap(collapsedFirstChild, firstChild);
		std::swap(collapsedLastChild, lastChild);
		calcChildDegree();
		return childDegree();
	}
	return 0;
}

}
