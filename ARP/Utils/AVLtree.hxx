/*
	This file contains AVL tree definitions.

	Kalev Kask, March 2002.
*/

#ifndef AVL_HXX_INCLUDED
#define AVL_HXX_INCLUDED

#define AVL_DEFAULT_SIZE	1024
#define AVL_SIZE_LIMIT		1048576

/*
	This structure is a node in the AVL tree.
*/

typedef struct _MauiAVLNode {

	// the id of this entity (or the key)
	long m_id ;

	// this is a 4-byte field for the user to store data associated with this key
	long m_data ;

	// a linked list of elements in use, or not in use
	// for used nodes, 
	//		this is a doubly linked list, sorted (from left to right) in increasing order of keys
	// for unsed nodes, 
	//		this if a singly linked list (only m_next_entity is used) of unused nodes.
	unsigned long m_prev_entity, m_next_entity ;

	// left child and right child
	unsigned long m_LC, m_RC ;
	// height is the length of the longest path from this node to a leaf.
	// we want to use AVLtree[0].m_height = -1, so that we can do 'AVLtree[parent] = ++AVLtree[child].m_height' and it is correct when child=0.
	signed short m_height ;
	// balance = height of right subtree - height of left subtree
	signed char m_balance ;

	// 0 if not in use, 1 if in use
	unsigned char m_in_use ;

public :

	void operator<<(const _MauiAVLNode & Node) ;

} MauiAVLNode ;

/*
	This class is used to represent an AVL tree.
*/

class CMauiAVLTree
{

protected :

	// pointer to the actual AVL tree
	MauiAVLNode *m_db_tree ;

	// this is the amount of space allocated for the AVL tree
	unsigned long m_allocated_space ;

	// When reallocating the tree, system will reserve that much more memory.
	// If this variable is 0, system will never reallocate.
	// That means that when the tree is full and the user calls Insert() on the tree, it will fail.
	// Initially equal to AVL_DEFAULT_SIZE.
	unsigned long m_re_allocation_size ;

	// this is the actual (current) size of the AVL tree (ie. number of currently used blocks)
	unsigned long m_size ;

	// number of free blocks
	unsigned long m_free_size ;

	// points to the first free block
	unsigned long m_first_free_block ;

	// points to the node that has the smallest key.
	// this key is the first node in the ordered list of keys.
	// this list if build using m_next_entity and m_prev_entity pointers stored in each node.
	unsigned long m_first_used_block ;

	// a pointer to the root of the AVL tree.
	// if root pointer is less than 1, (eg. 0 or -1) the root node is undefined.
	// this happens when the tree is empty, or the memory for the tree is not allocated.
	unsigned long m_root ;

	// these variables are used to store the stack (usually) when traversing the tree
	// since the height of a AVL tree is always no more than 1.44 log(n) we don't need very much space.
	// assuming we don't allow more than 1G objects, the depth is bounded by 1.44*30.
	long Left[64], Right[64], Middle[64] ;

	// this variable is used during balancing of the AVL tree.
	unsigned long path_len ;

private :

	bool operator<<(const CMauiAVLTree & AVLtree) ;

private :

	// Management functions are private.

	void R_rotation(unsigned long k) ;
	void L_rotation(unsigned long k) ;
	void BalanceInsertTree(void) ;
	void BalanceDeleteTree(void) ;
	// initial memory allocation
	int AllocateMemory(unsigned long size) ;
	// when the tree is full, then system tries to reallocate the entire tree
	int ReAllocateMemory(void) ;
	void PrintTree(void) ;

// *****************************************************************************************************
// Timer.
// *****************************************************************************************************

public :

	inline unsigned long GetSize(void) const { return m_size ; }
	inline unsigned long GetSpace(void) const { return m_allocated_space ; }
	inline unsigned long GetFirstUsedBlockIndex(void) const { return m_first_used_block ; }

	void Empty(void) ;

	// returns TRUE iff the key in in the tree
	long Find(long key, long *data /* return data here */) ;
	long FindNext(long key, long *next_key, long *next_data /* return data here */) ;
	int FindData(long data) ;

	// returns FALSE iff it fails (no memory), otherwise idx of the item in the data structure. if key is in use, it will replace.
	long Insert(long key, long data) ;

	// set data at a specific index; often used after GetNext().
	int SetData(unsigned long idx, long data) ;

	// Remove a key from the tree.
	// Note that it is not a good idea to delete keys while the tree is being traversed using the
	// GetNext() function, because the selection/current pointer might be invalid.
	long Remove(long key, long *data /* return data here. can be NULL */) ; // returns TRUE iff everything is fine
	long RemoveFirst(long *key /* output */, long *data /* return data here. can be NULL */) ; // returns TRUE iff everything is fine

	void SetReallocationSize(unsigned long reallocation_size) ; // has to be non-negative. default AVL_DEFAULT_SIZE.

	// to find the smallest positive key not in the tree.
	// return value is > 0.
	long FindSmallestPositiveKeyNotUsed(void) ;

	// Increment the data of the given key by 1.
	long Increment(long key) ;

	// Adjust the data of the given key.
	long Adjust(long key, long data_adjustment, bool OnlyIfExists) ;
	inline void Adjust_Idx(long idx, long data_adjustment) { m_db_tree[idx].m_data += data_adjustment ; }

	// Decrement the data of the given key by 1. Optionally, if data is 0, remove it.
	// Returns: >0 iff was found and stays in the tree, 0 iff not found, <0 if was found, but deleted.
	long Decrement(long key, int remove_if_0) ;

	// this function will decrement all keys larger than the given key.
	// if this key itself is present, it will check if the key+1 is also present.
	// if yes, this function fails because we would have duplicate keys.
	// note that the structure of the tree does not change (if the function does not fail).
	int DecrementLargerKeys(long key) ;

/*
	Member functions to traverse the tree.
	Returns FALSE iff the tree is empty or no next element.
	Note that the keys are returned in an increasing order of keys.
*/
	// Note that this function does set the current pointer to the first element in the tree
	long GetFirst(long *key /* can be NULL */, long *data /* can be NULL */, long & current) const ;
	// if current = -1 is passed in, this function returns the first key.
	// otherwise this function returns the key next to the current in the increasing order of keys.
	long GetNext(long *key /* can be NULL */, long *data /* can be NULL */, long & current) const ;
	// this function is like GetNext, except it sets a new value for the data value of the key.
	long GetNextAndSetData(long *key /* can be NULL */, long *data /* can be NULL */, long NewData, long & current) const ;
	// this function returns the key/data pointed to by 'current' 
	// (if -1==current, then this means the first, if 0==current, then this means NONE).
	// it also forwars the 'current' pointer to the next element in the set.
	long GetCurrentAndAdvance(long *key /* can be NULL */, long *data /* can be NULL */, long & current) const ;

	void PrintString(char *) { }
	long CheckTree(char **pErrorStr = NULL) ; // returns TRUE iff the tree is OK.
	long TestTree(int numIterations, char **pErrorStr = NULL) ; // returns TRUE iff the tree is OK.
	bool TestConsistency(long checkbitvector = 255) const ;

// *****************************************************************************************************
// Construction and destruction.
// *****************************************************************************************************

public :

	// Default constructor creates an empty tree of size AVL_DEFAULT_SIZE.
	CMauiAVLTree(void) ;

	// This constructor creates a tree from a set of keys.
	CMauiAVLTree(unsigned long size /* number of keys (has to be >= 0) */, 
			long *key /* an array of keys */, 
			long *data /* data associated with each key */, 
			unsigned long space /* how much space to allocate for the AVL tree (> 0) */, 
			unsigned long re_allocation_size = AVL_DEFAULT_SIZE) ;

	// Destructor has to release the memory that the AVL tree uses.
	~CMauiAVLTree(void) ;

} ;

#endif // AVL_HXX_INCLUDED
