
///////////////////////////////////////////////////////////////////////////////
// BTree.cpp
///////////////////////////////////////////////////////////////////////////////


#include "BTree.h"

#include <algorithm>
#include <vector>

using namespace std;

// _____________________________________________________________________________
template<class T, class CMP> BTree<T, CMP>::BTree(uint64_t k, uint64_t l)
{
	this->k = k;
	this->l = l;
	root = new BTreeLeafNode<T>();
}


// _____________________________________________________________________________
template<class T, class CMP> BTree<T, CMP>::~BTree()
{
	writeToFile();
	delete root;
	for (auto &node : nodes) delete node;
}



// _____________________________________________________________________________
template<class T, class CMP> BTreeLeafNode<T>* BTree<T, CMP>::
	navigateToLeaf(T key, BTreeNode<T>* start)
{
	// Recursion base case
	if (start.isLeaf) return (BTreeLeafNode<T>*)start;

	// Navigate through an inner node: first check if all keys held in the node
	// are smaller than the given key -> follow last pointer in node
	// sort(start->keys.begin(), start->keys.end(), comp);
	if (comp(start->keys.back(), key)) 
		return navigateToLeaf(key, (BTreeInnerNode<T>*)start->values.back());


	// Otherwise, find the first key in the vector that is greater than or equal
	// to the search key -> follow pointer that immediately precedes this key
	auto low = lower_bound(start->keys.begin(), start->keys.end(), key, comp);
	int index = distance(start->keys.begin(), low);
	return navigateToLeaf(key, (BTreeInnerNode<T>*)start->values[index]);
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
		if (this->count == 0 || comp(insertLeaf->keys.back(), key))
		{
			insertLeaf->keys.push_back(key);
			insertLeaf->values.push_back(tid);
			insertLeaf->count++;
		}

		else
		{
			auto low = lower_bound(insertLeaf->keys.begin(), 
				                   insertLeaf->keys.end(), key, comp);
			int index = distance(insertLeaf->keys.begin(), low);
			insertLeaf->keys.emplace(low, key);
			insertLeaf->values.emplace(index, tid);
			insertLeaf->count++;
		}
	}

	// Second case: leaf is full -> add element and split node.
	else
	{
		if (comp(insertLeaf->keys.back(), key))
		{
			insertLeaf->keys.push_back(key);
			insertLeaf->values.push_back(tid);
		}
		else
		{
			auto low = lower_bound(insertLeaf->keys.begin(), 
				                   insertLeaf->keys.end(), key, comp);
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
	if (parentNode->count == 0 || comp(parentNode->keys.back(), median))
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
			                   parentNode->keys.end(), median, comp);
		int index = distance(parentNode->keys.begin(), low);
		parentNode->keys.emplace(low, median);
		parentNode->values.emplace(index+1, newNode);
		parentNode->count++;
	}
	// Parent node is also full -> split inner node (recursively)
	if (parentNode->count > 2*k) splitNode(parentNode);
}
