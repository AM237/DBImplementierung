
///////////////////////////////////////////////////////////////////////////////
// BTree.cpp
///////////////////////////////////////////////////////////////////////////////


#include "BTree.h"
#include <assert.h>

using namespace std;

// _____________________________________________________________________________
template<class T, class CMP> BTree<T, CMP>::BTree(SegmentManager* sm, 
	uint64_t k, uint64_t l)
{
	this->sm = sm;
	this->k = k;
	this->l = l;
	root = new BTreeLeafNode<T>();
	this->segId = sm->createSegment(true);
}



// _____________________________________________________________________________
template<class T, class CMP> BTree<T, CMP>::~BTree()
{
	writeToFile();
	delete root;
	for (auto &node : nodes) delete node;
	for (auto &it : rangeIterators) delete it;
}



// _____________________________________________________________________________
template<class T, class CMP> bool BTree<T, CMP>::cmp(T key1, T key2)
{
	return comp(key1, key2);
}



// _____________________________________________________________________________
template<class T, class CMP> BTreeLeafNode<T>* BTree<T, CMP>::
	navigateToLeaf(T key, BTreeNode<T>* start)
{
	// Recursion base case
	if (start.isLeaf) return (BTreeLeafNode<T>*)start;

	// Navigate through an inner node: first check if all keys held in the node
	// are smaller than the given key -> follow last pointer in node
	// sort(start->keys.begin(), start->keys.end(), cmp);
	if (cmp(start->keys.back(), key)) 
		return navigateToLeaf(key, (BTreeInnerNode<T>*)start->values.back());


	// Otherwise, find the first key in the vector that is greater than or equal
	// to the search key -> follow pointer that immediately precedes this key
	auto low = lower_bound(start->keys.begin(), start->keys.end(), key, cmp);
	int index = distance(start->keys.begin(), low);
	return navigateToLeaf(key, (BTreeInnerNode<T>*)start->values[index]);
}



//______________________________________________________________________________
template<class T, class CMP> TID BTree<T, CMP>::lookup(T key)
{
	// Get leaf and use binary search to locate the given key.
	auto leaf = navigateToLeaf(key, root);
	if (leaf->count == 0) throw keyNotFound;
	auto bounds = equal_range(leaf->keys.begin(), leaf->keys.end(), key, cmp);
	if (bounds.first == bounds.second) throw keyNotFound;
	return leaf->values[bounds.first-leaf->keys.begin()];
}



//______________________________________________________________________________
template<class T, class CMP> bool BTree<T, CMP>::erase(T key)
{
	// Get leaf and use binary search to locate the given key.
	auto leaf = navigateToLeaf(key);
	if (leaf->count == 0) return false;
	auto bounds = equal_range(leaf->keys.begin(), leaf->keys.end(), key, cmp);
	if (bounds.first == bounds.second) return false;
	else
	{
		leaf->keys.erase(bounds.first);
		leaf->values.erase(bounds.first-leaf->keys.begin());
	}
	return true;
}



//______________________________________________________________________________
template<class T, class CMP> BTreeRangeIterator<T, CMP>* BTree<T, CMP>::
	lookupRange(T start, T end)
{
	// Local parameter copies to decouple pointer parameters to the iterator.
	T s = start;
	T e = end;

	rangeIterators.push_back(new BTreeRangeIterator<T, CMP>(s, e));
	auto it = rangeIterators.back();

	return it;
}



// _____________________________________________________________________________
template<class T, class CMP> void BTree<T, CMP>::insert(T key, TID tid)
{
	// Get leaf into which to insert to
	auto insertLeaf = navigateToLeaf(root);

	// First case: there is space in this leaf -> insert pair directly
	if(insertLeaf->count +1 <= 2*l)
	{
		// Key is larger than all other entries -> append pair to end
		if (this->count == 0 || cmp(insertLeaf->keys.back(), key))
		{
			insertLeaf->keys.push_back(key);
			insertLeaf->values.push_back(tid);
			insertLeaf->count++;
		}

		else
		{
			auto low = lower_bound(insertLeaf->keys.begin(), 
				                   insertLeaf->keys.end(), key, cmp);
			int index = distance(insertLeaf->keys.begin(), low);
			insertLeaf->keys.emplace(low, key);
			insertLeaf->values.emplace(index, tid);
			insertLeaf->count++;
		}
	}

	// Second case: leaf is full -> add element and split node.
	else
	{
		if (cmp(insertLeaf->keys.back(), key))
		{
			insertLeaf->keys.push_back(key);
			insertLeaf->values.push_back(tid);
		}
		else
		{
			auto low = lower_bound(insertLeaf->keys.begin(), 
				                   insertLeaf->keys.end(), key, cmp);
			int index = distance(insertLeaf->keys.begin(), low);
			insertLeaf->keys.emplace(low, key);
			insertLeaf->values.emplace(index, tid);
		}
		splitNode(insertLeaf);
	}
}



// _____________________________________________________________________________
template<class T, class CMP> void BTree<T, CMP>::splitNode(BTreeNode<T>* node)
{
	// The new sibling node. Because in this case the node being split is a leaf
	// this sibling node will always be a leaf.
	auto newNode = node->isLeaf ? new BTreeLeafNode<T>*(node->parent) :
								  new BTreeInnerNode<T>*(node->parent);
	nodes.push_back(newNode);
	node->next = newNode;

	// Create and configure new root if the current root overflows.
	if (node == root)
	{
		auto newRoot = new BTreeInnerNode<T>*();
		node->parent = newRoot;
		newNode->parent = newRoot;
		root = newRoot;
	}
	
	// Cut contents larger than the median to the newly created node
	uint64_t med = (node->count + 1) / 2;
	T median = node->at(med);

	newNode->keys.assign(node->keys.begin()+med+1, node->keys.end());
	node->keys.erase(node->keys.begin()+med+1, node->keys.end());

	newNode->values.assign(node->values.begin()+med+1, node->keys.end());
	node->values.erase(node->values.begin()+med+1, node->values.end());	

	node->count = node->keys.size();
	newNode->count = newNode->keys.size();
	
	// Send median key to parent node
	auto parentNode = node->parent;

	// Median is larger than all other entries -> append median to end
	// and update child pointers
	if (parentNode->count == 0 || cmp(parentNode->keys.back(), median))
	{
		parentNode->values[parentNode->values.end()-1] = node; 
		parentNode->keys.push_back(median);
		parentNode->values.push_back(newNode);
		parentNode->count++;
	}
	else
	{
		// Get iterator / index to smallest element larger than the
		// median, and insert the key (median) and the pointer to the
		// newly created node at the appropriate locations
		auto low = lower_bound(parentNode->keys.begin(), 
			                   parentNode->keys.end(), median, cmp);
		int index = distance(parentNode->keys.begin(), low);
		parentNode->keys.emplace(low, median);
		parentNode->values.emplace(index+1, newNode);
		parentNode->count++;
	}
	// Parent node is also full -> split inner node (recursively)
	if (parentNode->count > 2*k) splitNode(parentNode);
}



// _____________________________________________________________________________
template<class T, class CMP> void BTree<T, CMP>::writeToFile()
{
	// Manage space requirements
	Segment* bTreeSeg = sm->retrieveSegmentById(this->segId);
	BufferManager* bm = sm->bm;
	while (bTreeSeg->getSize() < nodes.size()) sm->growSegment(this->segId);

	// Write a node per page
	for (auto it = 0; it < nodes.size(); it++)
	{
		BTreeNode<T>* node = nodes[it];
		assert(sizeof(node) <= constants::pageSize);
		uint64_t pageNo = bTreeSeg->nextPage();
		BufferFrame& bf = bm->fixPage(pageNo, true);
		memcpy(bf.data(), &node, sizeof(node));
		bm->unfixPage(bf, true);
	}
}