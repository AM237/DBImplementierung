
///////////////////////////////////////////////////////////////////////////////
// BTreeRangeIterator.cpp
///////////////////////////////////////////////////////////////////////////////


#include "BTreeRangeIterator.h"

using namespace std;

// _____________________________________________________________________________
template<class T, class CMP> bool BTreeRangeIterator<T,CMP>::cmp(T key1, T key2)
{
	return comp(key1, key2);
}

// _____________________________________________________________________________
template<class T, class CMP> BTreeRangeIterator<T, CMP>:: 
	BTreeRangeIterator(BTree<T, CMP>* btree, T* keyStart, T* keyEnd) 
	: keyStart(keyStart), keyEnd(keyEnd) 
{
	// Set keyStart, keyEnd, switch if necessary
	if (keyStart == nullptr && keyEnd == nullptr) throw invalidRange;
	if (keyStart != nullptr)
	{
		this->keyStart = *keyStart;
		leftBound = true;
	}
	if (keyEnd != nullptr)
	{
		this->keyEnd = *keyEnd;
		rightBound = true;
	}
	if (comp(this->keyEnd, this->keyStart))
	{
		T temp = this->keyStart;
		this->keyStart = this->keyEnd;
		this->keyEnd = temp;
	}

	// Travel to first leaf that could possibly contain relevant values
	currentLeaf = this->btree->navigateToLeaf(this->keyStart);

	// Check if key is greatest element in leaf -> update currentLeaf
	if (currentLeaf->count == 0 || comp(currentLeaf->keys.back(), keyStart))
		currentLeaf = currentLeaf->next;

	// If no more leaves available, then the start key is larger than any
	// element in the btree -> throw exception.
	if (currentLeaf == nullptr) throw keyOutOfBounds;
	else lastSafeLeaf = currentLeaf;

	// Now currentLeaf is correctly initialized -> get first key greater
	// than or equal to keyStart
	auto low = lower_bound(currentLeaf->keys.begin(), 
		                   currentLeaf->keys.end(), keyStart, cmp);
	index = distance(currentLeaf->keys.begin(), low);
	currentTID = lastSafeLeaf->values[index];
}



// _____________________________________________________________________________
template<class T, class CMP> TID BTreeRangeIterator<T, CMP>::next()
{
	// If current key is "larger" than the keyEnd, then return the value
	// under the last compared key.
	if (comp(keyEnd, lastSafeLeaf->keys[index])) return currentTID;

	// Otherwise update the current tid
	currentTID = lastSafeLeaf->values[index];
		
	// Next call causes an out of bounds on the current leaf -> 
	// follow pointer to sibling leaf
	if (index+1 >= lastSafeLeaf->count)
	{
		// Find first neighbor with a non zero entry count
		while (currentLeaf->next != nullptr && currentLeaf->next->count == 0)
			currentLeaf = currentLeaf->next;
			
		// Find cause for loop exit
		if (currentLeaf->next == nullptr) { }
		else if (currentLeaf->next->count != 0)
		{
			currentLeaf = currentLeaf->next;
			lastSafeLeaf = currentLeaf;
			index = 0;
		}
	}
	else index++;

	return currentTID;
}