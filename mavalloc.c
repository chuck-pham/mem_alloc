// The MIT License (MIT)
// 
// Copyright (c) 2021, 2022 Trevor Bakker 
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES UTA OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include "mavalloc.h"
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

/* The maximum entries in our linked list / array */
#define MAX_LINKED_LIST_SIZE 10000

/* *** INTERNAL USE ONLY *** In an in-line implementation the root node
 * is always 0
 */
#define ROOTNODE 0

 /* *** INTERNAL USE ONLY *** Has the list been initialized */
static int initialized = 0;

/* *** INTERNAL USE ONLY *** Keep track of the last item placed
* so we can optimize the shifting slightly and only shift what is needed.
*/
static int lastUsed = -1;

/**
*
* \struct Node
*
* \brief The node in the linked list
*
* This structure represents the node in our linked list implementation.
* Since this linked list is implemented in an array the previous and next
* members, which would be pointers in a dynamically allocated linked list,
* are integers in this implementation.  Their values correspond to the array
* index where the previous and next nodes reside.
*
* The value in_use is to let us track which array entries are currently used.
* If you are re-using this code in a heap or arena assignment this value does
* NOT represent whether your heap block is free or in-use.  You will need to add
* a new member to this struct to track that.
*
* The value element is an integer to demonstrate the functionality of a sorted list.
* If you are reusing this code you will need to change that datatype to be the
* actual data type in your implementation, e.g you're creating a linked list of strings
* then you would change the dataype of value from an int to a char*.
*
*/

/**
* This array is the linked list we are implementing.  The linked list represented by this array
* is a sorted linked list using the "value" member to determine the sorting.  This array is
* not sorted in the same order as the list and, after multiple insertions and removals, may not
* even be close to the same order.  This is by design.  Rather than sort each element in the array
* every time a new node is added instead the next and previous links are adjusted to link
* the linked list element in the correct spot.
*
* This array is *** INTERNAL USE ONLY *** and should not be exposed to or used by the end user.
* The user will interface with the linked list through insertNode() and removeNode().  From the
* end-user perspective it's a linked list.  We hide that we have implemented the list in an array.
*/
enum TYPE {
    H = 0,
    P
}; 

struct Node {
	int in_use;
    int size;
    void *arena;
    enum TYPE type;
};

static struct Node LinkedList[MAX_LINKED_LIST_SIZE];
static void *gArena;
static enum ALGORITHM alg;
static void *lastAlloc;

// print list function for debugging purposes
void printList() {
    for (int i = 0; i <= lastUsed; i++) {
        printf("LinkedList[%d].in_use = %d\n", i, LinkedList[i].in_use);
        printf("LinkedList[%d].size = %d\n", i, LinkedList[i].size);
        printf("LinkedList[%d].arena = %p\n", i, LinkedList[i].arena);
        printf("LinkedList[%d].type = %d\n", i, LinkedList[i].type);
        printf("\n");
    }
}

/**
 *
 * \fn removeNode(int prev)
 *
 * \brief Coalesce the node after prev into prev
 *
 * This function will remove the next node that is specified by the parameter
 * from the linked list and combine it into the node at prev. This involves
 * shifting the entire list and checking if further combinations are necessary.
 *
 * \param prev The index of the node that will be
 *              removed from the linked list
 *
 * Assumes node at and after prev are both holes.
 */
void removeNode(int prev) {
    // combine size of node after prev into node at prev
    LinkedList[prev].size = LinkedList[prev].size + LinkedList[prev+1].size;

    // shift list from prev+1 to lastUsed
    for (int i = prev + 1; i <= lastUsed; i++) {
        LinkedList[i].in_use = LinkedList[i + 1].in_use;
        LinkedList[i].size = LinkedList[i + 1].size;
        LinkedList[i].arena = LinkedList[i + 1].arena;
        LinkedList[i].type = LinkedList[i + 1].type;
    }
    lastUsed--;

    // recursively call removeNode again at prev if a hole was shifted next to prev
    if (LinkedList[prev + 1].in_use && LinkedList[prev + 1].type == H) {
        removeNode(prev);
    }

    return;
}


/**
 *
 * \fn insertNode(int size, int prev)
 *
 * \brief Insert a node into the linked list that contains the given size at prev
 *
 * This function will insert a node with a size that is specified by the parameter
 * into the linked list.
 *
 * \param size The size of the node that will be
 *              inserted into the linked list
 *
 * \param prev The index of the linked list where
 *              the new node will be placed
 *
 * Assumes node at prev is a valid hole that was selected by the specified algorithm.
 */
void insertNode(int size, int prev) {
	int i = 0;

	if (initialized && prev < 0 || prev >= MAX_LINKED_LIST_SIZE) {
	    printf("ERROR: Tried to insert a node beyond our bounds %d\n", prev);
		return;
	}

    // just make hole a process alloc if no leftover size
    if (size == LinkedList[prev].size) {
        LinkedList[prev].type = P;
        return;
    }

    // Shift everything down and make room for the new node. But we only
	// need to shift from lastUsed to prev + 1
	if (lastUsed == -1) {
        // whole block can be removed, as is it not needed
        LinkedList[i].in_use = 1;
		LinkedList[i].size = size;
        LinkedList[i].arena = gArena;
        LinkedList[i].type = H;
		lastUsed = i;
        return;
	} 
    else {
        // calculate leftover size
        int leftover = LinkedList[prev].size - size;

        // shift list from lastUsed to prev
		for (i = lastUsed; i >= prev; i--) {
            LinkedList[i + 1].in_use = LinkedList[i].in_use;
			LinkedList[i + 1].size = LinkedList[i].size;
            LinkedList[i + 1].arena = LinkedList[i].arena;
            LinkedList[i + 1].type = LinkedList[i].type;
		}
	
        // create new node of size at prev
		LinkedList[prev].in_use = 1;
		LinkedList[prev].size = size;
        LinkedList[prev].arena = LinkedList[prev + 1].arena;
        LinkedList[prev].type = P;

        // change size and arena ptr of node after new node
        unsigned char *new_ptr = (unsigned char *)LinkedList[prev].arena + size;
        LinkedList[prev + 1].size = leftover;
        LinkedList[prev + 1].arena = (void *)new_ptr;

		lastUsed++;
	}

    return;
}

int mavalloc_init(size_t size, enum ALGORITHM algorithm) {
    // check for invalid size
    if (size <= 0) {
        return -1;
    }
    
    // allign memory request and malloc memory
    size_t req_size = ALIGN4(size);
    gArena = malloc(req_size);

    // check if allocation worked
    if (!gArena) {
        return -1;
    }

    // initialize list, create first entry, and set algorithm
    for (int i = 0; i < MAX_LINKED_LIST_SIZE; i++) {
        LinkedList[i].in_use = 0;
        LinkedList[i].size = 0;
        LinkedList[i].arena = NULL;
        LinkedList[i].type = H;
    }
    initialized = 1;
    
    alg = algorithm;

    LinkedList[0].in_use = 1;
    LinkedList[0].size = req_size;
    LinkedList[0].arena = gArena;
    LinkedList[0].type = H;
    lastUsed = 0;
    lastAlloc = NULL;

    // allocation successful
    return 0;
}

void mavalloc_destroy() {
    // free malloc'd memory using pointer from first node
    free(LinkedList[0].arena);

    // reset list
    for (int i = 0; i < MAX_LINKED_LIST_SIZE; i++) {
        LinkedList[i].in_use = 0;
        LinkedList[i].size = 0;
        LinkedList[i].arena = NULL;
        LinkedList[i].type = H;
    }
    initialized = 0;

    return;
}

void * mavalloc_alloc(size_t size) {
    // check for invalid size
    if (size <= 0) {
        return NULL;
    }

    // check if list not initialzied i.e. _destroy called
    if (!initialized) {
        return NULL;
    }

    // align memory request
    size_t req_size = ALIGN4(size);

    // allocate memory using algorithm specified
    if (alg == FIRST_FIT) {
        // iterate through list starting at the beginning
        for (int i = 0; i < MAX_LINKED_LIST_SIZE; i++) {
			// optimization: stop search when at end of list
			if (!LinkedList[i].in_use) {
				break;
			}

            //search for first hole with enough size to fit request
            if (LinkedList[i].type == H && LinkedList[i].size >= req_size) {
                // insert new node of requested size at index of valid hole and return the pointer
                insertNode(req_size, i);
                return LinkedList[i].arena;
            }
        }
    } 
    else if (alg == NEXT_FIT) {
        // because combining blocks will mess with the last allocated index,
        // store the ptr of the last allocation instead, then key off of 
        // the index of the node holding that ptr

        // find index of last allocated node
        int idx = -1;
        for (int i = 0; i < MAX_LINKED_LIST_SIZE; i++) {
            // optimization: stop search when at end of list
            if (!LinkedList[i].in_use) {
                break;
            }

            // if ptr in linked list, set that node as a hole
            if (lastAlloc == LinkedList[i].arena) {
                idx = i;
            }
        }   

        // iterate through list starting at the last allocated index
        // i+1 used in case last allocation was freed (looks at next node instead)
        for (int i = idx; i < MAX_LINKED_LIST_SIZE; i++) {
			// optimization: stop search when at end of list
			if (!LinkedList[i + 1].in_use) {
				break;
			}

            //search for first hole with enough size to fit request
            if (LinkedList[i + 1].type == H && LinkedList[i + 1].size >= req_size) {
                // insert new node of requested size at index of valid hole,
                // update lastAlloc for future allocations, and return the pointer
                insertNode(req_size, i + 1);
                lastAlloc = LinkedList[i + 1].arena;
                return LinkedList[i + 1].arena;
            }
        }

        // iterate through list starting at the beginning to the last allocated index
        // assumes all nodes in this search are in_use so no need for optimization
        for (int i = 0; i <= idx; i++) {
            //search for first hole with enough size to fit request
            if (LinkedList[i].type == H && LinkedList[i].size >= req_size) {
                // insert new node of requested size at index of valid hole,
                // update lastAlloc for future allocations, and return the pointer
                insertNode(req_size, i);
                lastAlloc = LinkedList[i].arena;
                return LinkedList[i].arena;
            }
        }
    } 
    else if (alg == BEST_FIT) {
        int idx = -1;
        int winner = INT_MAX;

        // iterate through list starting at the beginning
        for (int i = 0; i < MAX_LINKED_LIST_SIZE; i++) {
            // optimization: stop search when at end of list
			if (!LinkedList[i].in_use) {
				break;
			}

            // look for holes that could fit requested size
            if (LinkedList[i].type == H && LinkedList[i].size >= req_size) {
                // calculate leftover size and see if it is less than winner
                int leftover = LinkedList[i].size - req_size;
                if (leftover < winner) {
                    // if it is, update winner and grab the index
                    winner = leftover;
                    idx = i;
                }
            }
        }

        // once search is done, if a valid index was grabbed, insert a node at that index
        if (idx != -1) {
            insertNode(req_size, idx);
            return LinkedList[idx].arena;
        }
    } 
    else if (alg == WORST_FIT) {
        int idx = -1;
        int winner = -1;

        // iterate through list starting at the beginning
        for (int i = 0; i < MAX_LINKED_LIST_SIZE; i++) {
            // optimization: stop search when at end of list
			if (!LinkedList[i].in_use) {
				break;
			}

            // look for holes that could fit requested size
            if (LinkedList[i].type == H && LinkedList[i].size >= req_size) {
                // calculate leftover size and see if it is bigger than winner
                int leftover = LinkedList[i].size - req_size;
                if (leftover > winner) {
                    // if it is, update winner and grab the index
                    winner = leftover;
                    idx = i;
                }
            }
        }

        // once search is done, if a valid index was grabbed, insert a node at that index
        if (idx != -1) {
            insertNode(req_size, idx);
            return LinkedList[idx].arena;
        }
    }

    // only return NULL on failure
    return NULL;
}

void mavalloc_free(void *ptr) {
    // optimization: if NULL ptr passed, just return
    if (!ptr) {
        return;
    }

    // search for ptr in linked list
    for (int i = 0; i < MAX_LINKED_LIST_SIZE; i++) {
        // optimization: stop search when at end of list
        if (!LinkedList[i].in_use) {
            break;
        }

        // if ptr in linked list, set that node as a hole
        if (ptr == LinkedList[i].arena) {
            LinkedList[i].type = H;
        }
    }

    // search for adjacent holes
    for (int i = 0; i < MAX_LINKED_LIST_SIZE - 1; i++) {
        // optimization: stop search when at end of list
        if (!LinkedList[i].in_use) {
            break;
        }

        // if current node and next node are both holes, combine them
        // will also take care of further combinations in removeNode
        if (LinkedList[i].type == H && LinkedList[i + 1].in_use && LinkedList[i + 1].type == H) {
            removeNode(i);
        }
    }

    return;
}

int mavalloc_size() {
    int number_of_nodes = 0;

    // iterate through entire list, counting all nodes in_use
    for (int i = 0; i < MAX_LINKED_LIST_SIZE; i++) {
        if (LinkedList[i].in_use) {
            number_of_nodes++;
        }
    }

    return number_of_nodes;
}
