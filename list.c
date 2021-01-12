#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

static List lists[LIST_MAX_NUM_HEADS];
static Node nodes[LIST_MAX_NUM_NODES];

// Variables to keep track of the available node and list.
Node* pAvailableNode;
List* pAvailableList;

// Initializes pAvailableList and pAvailableNode.
// For each list in lists[], stores pointers to its adjacent lists and
// for each node in nodes[], stores pointers to its adjacent nodes.
static bool notSetup = true;
static void setup()
{
    pAvailableList = &lists[0];
    pAvailableNode = &nodes[0];
    List* pCurrentList;
    Node* pCurrentNode;

    for (int i = 0; i < LIST_MAX_NUM_HEADS - 1; i++) {
        pCurrentList = &lists[i];
        pCurrentList->nextAvailableList = &lists[i+1];
    }
    lists[LIST_MAX_NUM_HEADS-1].nextAvailableList = NULL;

    for (int i = 0; i < LIST_MAX_NUM_NODES - 1; i++) {
        pCurrentNode = &nodes[i];
        pCurrentNode->next = &nodes[i+1];
    }
    nodes[LIST_MAX_NUM_NODES-1].next = NULL;
}

// Resets the fields in pList
static void resetList(List* pList) {
    assert(pList != NULL);
    pList->size = 0;
    pList->head = NULL;
    pList->tail = NULL;
    pList->current = NULL;
    pList->state = before;
    pList->nextAvailableList = NULL;
    return;
}

// Resets the fields in pNode
static void resetNode(Node* pNode) {
    assert(pNode != NULL);
    pNode->item = NULL;
    pNode->previous = NULL;
    pNode->next = NULL;
    return;
}

// Returns a pointer to an available list in lists[].
// Returns NULL if lists are exhausted.
static List* nextAvailableList()
{
    if (pAvailableList == NULL)  {
        return NULL;
    }
    else {
        List* pList = pAvailableList;
        pAvailableList = pAvailableList->nextAvailableList;
        resetList(pList);
        return pList;
    }
}

// Returns a pointer to an available node in nodes[].
// Returns NULL if nodes are exhaused.
static Node* nextAvailableNode()
{
    if (pAvailableNode == NULL) {
        return NULL;
    }
    else {
        Node* pNode = pAvailableNode;
        pAvailableNode = pAvailableNode->next;
        resetNode(pNode);
        return pNode;
    }
}

// Adds pList to the available lists pool
static void markAsAvailableList(List* pList)
{
    pList->nextAvailableList = pAvailableList;
    pAvailableList = pList;
    return;
}

// Adds pNode to the available nodes pool
// Does not change the item in pNode
static void markAsAvailableNode(Node* pNode)
{
    pNode->next = pAvailableNode;
    pAvailableNode = pNode;
    return;
}

// Returns true if pList is empty, false if non-empty.
static bool isEmptyList(List* pList)
{
    return (pList->size == 0) && (pList->head == NULL) && (pList->tail == NULL) && (pList->current == NULL) && (pList->state != within);
}

// Returns true if pList is non-empty, false if empty.
static bool isNonEmptyList(List* pList)
{
    return (pList->size > 0) && (pList->head != NULL) && (pList->tail != NULL);
}

// Adds item to the empty list, and makes the new item the current one.
// Returns 0 on success, -1 on failure.
static int addToEmptyList(List* pList, void* pItem)
{
    assert(pList != NULL);
    assert(isEmptyList(pList));
    Node* pNode = nextAvailableNode();
    if (pNode == NULL) {
        return -1;
    }
    pNode->item = pItem;
    pNode->previous = NULL;
    pNode->next = NULL;
    pList->size++;
    pList->head = pNode;
    pList->tail = pNode;
    pList->current = pNode;
    pList->state = within;
    return 0;
}

// Makes a new, empty list, and returns its reference on success.
// Returns a NULL pointer on failure.
// Calls setup() to store the address of each node and list the first time it is called
List* List_create()
{
    if (notSetup) {
        setup();
        notSetup = false;
    }
    List* pList = nextAvailableList();
    if (pList != NULL) {
        assert(isEmptyList(pList));
    }
    return pList;
}

// Returns the number of items in pList.
int List_count(List* pList)
{
    assert(pList != NULL);
    return pList->size;
}

// Returns a pointer to the first item in pList and makes the first item the current item.
// Returns NULL and sets current item to NULL if list is empty.
void* List_first(List* pList)
{
    assert(pList != NULL);
    if (pList->size == 0) {
        assert(isEmptyList(pList));
        return NULL;
    }
    else {
        assert(isNonEmptyList(pList));
        pList->current = pList->head;
        pList->state = within;
        return pList->head->item;
    }
}

// Returns a pointer to the last item in pList and makes the last item the current item.
// Returns NULL and sets current item to NULL if list is empty.
void* List_last(List* pList)
{
    assert(pList != NULL);
    if (pList->size == 0) {
        assert(isEmptyList(pList));
        return NULL;
    } 
    else {
        assert(isNonEmptyList(pList));
        pList->current = pList->tail;
        pList->state = within;
        return pList->current->item;
    }
}

// Advances pList's current item by one, and returns a pointer to the new current item.
// If this operation advances the current item beyond the end of the pList, a NULL pointer 
// is returned and the current item is set to be beyond end of pList.
void* List_next(List* pList)
{
    assert(pList != NULL);
    if (pList->size == 0) {
        assert(isEmptyList(pList));
        pList->state = beyond;
        return NULL;
    }     
    else if (pList->state == before) {
        assert(isNonEmptyList(pList));
        pList->current = pList->head;
        pList->state = within;
        return pList->current->item;
    } 
    else if (pList->state == beyond) {
        assert(isNonEmptyList(pList));
        return NULL;
    } 
    else if (pList->current->next == NULL) {
        assert(isNonEmptyList(pList));
        pList->current = NULL;
        pList->state = beyond;
        return NULL;
    } 
    else {
        assert(isNonEmptyList(pList));
        pList->current = pList->current->next;
        return pList->current->item;
    }
}

// Backs up pList's current item by one, and returns a pointer to the new current item. 
// If this operation backs up the current item beyond the start of the pList, a NULL pointer 
// is returned and the current item is set to be before the start of pList.
void* List_prev(List* pList)
{
    assert(pList != NULL);
    if (pList->size == 0) {
        assert(isEmptyList(pList));
        pList->state = before;
        return NULL;
    }
    else if (pList->state == before) {
        assert(isNonEmptyList(pList));
        return NULL;
    }
    else if (pList->state == beyond) {
        assert(isNonEmptyList(pList));
        pList->current = pList->tail;
        pList->state = within;
        return pList->current->item;
    }
    else if (pList->current->previous == NULL) {
        assert(isNonEmptyList(pList));
        pList->current = NULL;
        pList->state = before;
        return NULL;
    }
    else {
        assert(isNonEmptyList(pList));
        pList->current = pList->current->previous;
        return pList->current->item;
    }
}

// Returns a pointer to the current item in pList.
void* List_curr(List* pList)
{
    assert(pList != NULL);
    if (pList->current == NULL) {
        return NULL;
    }
    assert(isNonEmptyList(pList));
    return pList->current->item;
}

// Adds the new item to pList directly after the current item, and makes item the current item. 
// If the current pointer is before the start of the pList, the item is added at the start. If 
// the current pointer is beyond the end of the pList, the item is added at the end. 
// Returns 0 on success, -1 on failure.
int List_add(List* pList, void* pItem)
{
    assert(pList != NULL);
    if (pList->size == 0) {
        assert(isEmptyList(pList));
        return addToEmptyList(pList, pItem);
    }
    else if (pList->state == before) {
        assert(isNonEmptyList(pList));
        return List_prepend(pList, pItem);
    }
    else if (pList->state == beyond || (pList->current->next == NULL)) {
        assert(isNonEmptyList(pList));
        return List_append(pList, pItem);
    }
    else {
        assert(isNonEmptyList(pList));
        Node* pNode = nextAvailableNode();
        if (pNode == NULL) {
            return -1;
        }
        pNode->item = pItem;
        pNode->next = pList->current->next;
        pNode->previous = pList->current;
        pList->current->next->previous = pNode;
        pList->current->next = pNode;
        pList->current = pNode;
        pList->state = within;
        return 0;
    }
}

// Adds item to pList directly before the current item, and makes the new item the current one. 
// If the current pointer is before the start of the pList, the item is added at the start. 
// If the current pointer is beyond the end of the pList, the item is added at the end. 
// Returns 0 on success, -1 on failure.
int List_insert(List* pList, void* pItem)
{
    assert(pList != NULL);
    if (pList->size == 0) {
        assert(isEmptyList(pList));
        return addToEmptyList(pList, pItem);
    }
    else if(pList->state == beyond) {
        assert(isNonEmptyList(pList));
        return List_append(pList, pItem);
    }
    else if (pList->state == before || (pList->current->previous == NULL)) {
        assert(isNonEmptyList(pList));
        return List_prepend(pList, pItem);
    }
    else {
        assert(isNonEmptyList(pList));
        Node* pNode = nextAvailableNode();
        if (pNode == NULL) {
            return -1;
        }
        pNode->item = pItem;
        pNode->next = pList->current;
        pNode->previous = pList->current->previous;
        pList->current->previous->next = pNode;
        pList->current->previous = pNode;
        pList->current = pNode;
        pList->state = within;
        pList->size++;
        return 0;
    }
}

// Adds item to the end of pList, and makes the new item the current one. 
// Returns 0 on success, -1 on failure.
int List_append(List* pList, void* pItem)
{
    assert(pList != NULL);
    if (pList->size == 0) {
        assert(isEmptyList(pList));
        return addToEmptyList(pList, pItem);
    }
    else {
        assert(isNonEmptyList(pList));
        Node* pNode = nextAvailableNode();
        if (pNode == NULL) {
            return -1;
        }
        pNode->item = pItem;
        pNode->previous = pList->tail;
        pNode->next = NULL;
        pList->tail->next = pNode;
        pList->tail = pNode;
        pList->size++;
        pList->current = pNode;
        pList->state = within;
        return 0;
    }
}

// Adds item to the front of pList, and makes the new item the current one. 
// Returns 0 on success, -1 on failure.
int List_prepend(List* pList, void* pItem)
{
    assert(pList != NULL);
    if (pList->size == 0) {
        assert(isEmptyList(pList));
        return addToEmptyList(pList, pItem);
    }
    else {
        assert(isNonEmptyList(pList));
        Node* pNode = nextAvailableNode();
        if (pNode == NULL) {
            return -1;
        }
        pNode->item = pItem;
        pNode->previous = NULL;
        pNode->next = pList->head;
        pList->head->previous = pNode;
        pList->head = pNode;
        pList->size++;
        pList->current = pNode;
        pList->state = within;
        return 0;
    }
}

// Return current item and take it out of pList. Make the next item the current one.
// If the current pointer is before the start of the pList, or beyond the end of the pList,
// then do not change the pList and return NULL.
void* List_remove(List* pList)
{
    assert(pList != NULL);
    if ((pList->state == before) || (pList->state == beyond)) {
        return NULL;
    }
    assert(isNonEmptyList(pList) && (pList->current != NULL));
    Node* pNode = pList->current;
    if (pList->size == 1) {
        pList->head = NULL;
        pList->tail = NULL;
        pList->current = NULL;
        pList->state = beyond;
    }
    else if (pNode->previous == NULL) {
        pNode->next->previous = NULL;
        pList->head = pNode->next;
        pList->current = pNode->next;
    }
    else if (pNode->next == NULL) {
        pNode->previous->next = NULL;
        pList->current = NULL;
        pList->state = beyond;
    }
    else {
        pNode->previous->next = pNode->next;
        pNode->next->previous = pNode->previous;
        pList->current = pNode->next;
    }
    pList->size--;
    markAsAvailableNode(pNode);
    return pNode->item;
}

// Adds pList2 to the end of pList1. The current pointer is set to the current pointer of pList1. 
// pList2 no longer exists after the operation; its head is available for future operations.
void List_concat(List* pList1, List* pList2)
{
    assert((pList1 != NULL) && (pList2 != NULL));
    if (pList1->size == 0) {
        pList1->size = pList2->size;
        pList1->head = pList2->head;
        pList1->tail = pList2->tail;
        pList1->current = NULL;
        pList1->state = before;
    }
    else if (pList2->size != 0){
        pList1->size = pList1->size + pList2->size;
        pList1->tail->next = pList2->head;
        pList2->head->previous = pList1->tail;
        pList1->tail = pList2->tail;
        if (pList1->current == NULL) {
            pList1->state = before;
        }
    }
    resetList(pList2);
    markAsAvailableList(pList2);
    return;
}

// Delete pList. pItemFreeFn is a pointer to a routine that frees an item. 
// It should be invoked (within List_free) as: (*pItemFreeFn)(itemToBeFreedFromNode);
// pList and all its nodes no longer exists after the operation; its head and nodes are available for future operations.
typedef void (*FREE_FN)(void* pItem);
void List_free(List* pList, FREE_FN pItemFreeFn)
{
    assert(pList != NULL);
    Node* pCurrent = pList->head;
    Node* pNext;
    while (pCurrent != NULL) {
        pNext = pCurrent->next;
        (*pItemFreeFn)(pCurrent->item);
        markAsAvailableNode(pCurrent);
        pCurrent = pNext;
    }
    resetList(pList);
    markAsAvailableList(pList);
    return;
}

// Return last item and take it out of pList. Make the new last item the current one.
// Return NULL if pList is initially empty.
void* List_trim(List* pList)
{
    assert(pList != NULL);
    if (pList->size == 0) {
        assert(isEmptyList(pList));
        return NULL;
    }
    assert(isNonEmptyList(pList));
    Node* pNode = pList->tail;
    if (pList->size == 1) {
        pList->head = NULL;
        pList->tail = NULL;
        pList->current = NULL;
        pList->state = before;
    }
    else {
        pList->tail = pNode->previous;
        pList->tail->next = NULL;
        pList->current = pList->tail;
    }
    pList->size--;
    markAsAvailableNode(pNode);
    return pNode->item;
}

// Search pList, starting at the current item, until the end is reached or a match is found. 
// In this context, a match is determined by the comparator parameter. This parameter is a
// pointer to a routine that takes as its first argument an item pointer, and as its second 
// argument pComparisonArg. Comparator returns 0 if the item and comparisonArg don't match, 
// or 1 if they do. Exactly what constitutes a match is up to the implementor of comparator. 
// 
// If a match is found, the current pointer is left at the matched item and the pointer to 
// that item is returned. If no match is found, the current pointer is left beyond the end of 
// the list and a NULL pointer is returned.
// 
// If the current pointer is before the start of the pList, then start searching from
// the first node in the list (if any).
typedef bool (*COMPARATOR_FN)(void* pItem, void* pComparisonArg);
void* List_search(List* pList, COMPARATOR_FN pComparator, void* pComparisonArg)
{
    assert((pList != NULL) && (pComparisonArg != NULL));

    if (pList->size == 0) {
        assert(isEmptyList(pList));
        pList->state = beyond;
        return NULL;
    }
    else if (pList->state == before) {
        assert(isNonEmptyList(pList));
        pList->current = pList->head;
        pList->state = within;
    }
    else if (pList->state == beyond) {
        assert(isNonEmptyList(pList));
        return NULL;
    }
    Node* pCurrent = pList->current;
    Node* pNext;
    bool matchFound;
    while (pCurrent != NULL) {
        pNext = pCurrent->next;
        matchFound = (*pComparator)(pCurrent->item, pComparisonArg);
        if ( matchFound ) {
            pList->current = pCurrent;
            return pCurrent->item;
        }
        pCurrent = pNext;
    }
    pList->current = NULL;
    pList->state = beyond;
    return NULL;
}
