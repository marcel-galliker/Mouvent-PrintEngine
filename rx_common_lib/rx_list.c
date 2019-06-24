// ****************************************************************************
//
//	rx_list.c
//
// ****************************************************************************
//
//	Copyright 2018 by Mouvent AG, Switzerland. All rights reserved.
//	Written by Hubert Zimmermann
//
// ****************************************************************************

#include "rx_common.h"
#include "rx_list.h"

//--- Defines -----------------------------------------------------------------

//--- Structures, Typedefs ----------------------------------------------------
typedef struct ListNode {
	struct ListNode	*prev;		/* Previous entry */
	struct ListNode	*next;		/* Next entry */
	void			*elem;		/* Content */
} SListNode;

typedef struct {
	SListNode	*head;			/* First entry */
	SListNode	*tail;			/* Last entry */
	int			count;			/* Number of entries */
	SListNode	*select;		/* Current entry */
} SListHead;

//--- Prototypes --------------------------------------------------------------

//*****************************************************************************

//--- rx_list_create ----------------------------------------------------------
int rx_list_create (HANDLE *handle)
{
	SListHead *head = NULL;
	
	if (!handle)
		return REPLY_ERROR;

	head = (SListHead*)malloc(sizeof(SListHead));
	head->head	= NULL;
	head->tail	= NULL;
	head->select = NULL;
	head->count = 0;

	*handle = head;
	return REPLY_OK;
}

//--- rx_list_close -----------------------------------------------------------
int rx_list_close(HANDLE handle)
{
	SListHead *head = (SListHead*)handle;
	SListNode *node;

	if (!head)
		return REPLY_ERROR;
	node = head->head;
	while (node) {
		SListNode *next = node->next;
		free(node);
		node = next;
	}
	free(head);
	return REPLY_OK;
}

//--- rx_list_add ------------------------------------------------------------- 
int rx_list_add (HANDLE handle, void* elem)
{
	SListHead *head = (SListHead*) handle;
	SListNode *node;

	if ((handle == NULL) || (elem == NULL))
		return REPLY_ERROR;

	node = (SListNode*)malloc(sizeof(SListNode));
	node->elem = elem;
	node->next = NULL;
	node->prev = head->tail;

	// First element
	if (head->tail == NULL) {
		if ((head->head != NULL) || (head->count != 0))
			return REPLY_ERROR;
		head->head = node;
		head->tail = node;
		head->count = 1;
		return REPLY_OK;
	}

	if (head->tail->next != NULL)
		return REPLY_ERROR;

	head->tail->next = node;

	head->tail =  node;
	head->select = NULL;

	head->count += 1;
	return REPLY_OK;
}

//--- rx_list_insert ----------------------------------------------------------
int rx_list_insert (HANDLE handle, void *elem)//, void **NODE)
{
	SListHead *head = (SListHead*)handle;
	SListNode *node;
	SListNode *prev;

	if (!head || !elem)
		return REPLY_ERROR;
		
	if (!head->tail) {
		if (head->head || head->count)
			return REPLY_ERROR;

		node = (SListNode*)malloc(sizeof(SListNode));
		node->elem = elem;
		node->next = NULL;
		node->prev = head->tail;
		head->count = 1;
		head->head = node;
		head->tail = node;
		head->select = NULL; 
		return REPLY_OK;
	}

	prev = head->select;
	if (!prev) {
		if (head->head->prev)
			return REPLY_ERROR;

		node = (SListNode*)malloc(sizeof(SListNode));
		node->elem = elem;
		node->prev = NULL;
		node->next = head->head;
		head->head->prev = node;
		head->head = node;
		head->count += 1;
		return REPLY_OK;
	}

	node = (SListNode*)malloc(sizeof(SListNode));
	node->elem = elem;
	node->prev = prev;
	node->next = prev->next;
	if (prev->next)
		prev->next->prev = node;
	prev->next = node;
	if (prev == head->tail)
		head->tail = node;
	head->count+= 1;
	return REPLY_OK;
}

//--- rx_list_remove ------------------------------------------------------ */
int rx_list_remove (HANDLE handle, void *elem)
{
	SListHead *head = (SListHead*)handle;
	SListNode *node = NULL;

	if (!head || (!elem && !head->select))
		return REPLY_ERROR;

	if (!head->count)
		return REPLY_OK;

	if (head->count == 1) {
		if (!head->head || (head->head != head->tail))
			return REPLY_ERROR;
		free(head->head);
		head->head= NULL;
		head->tail= NULL;
		head->select = NULL;
		head->count = 0;
		return REPLY_OK;
	}

	if (elem) {
		for (node = head->head; node; node = node->next) {
			if (node->elem == elem)
				break;
		}
	}
	else if (head->select) {
		for (node = head->head; node; node = node->next) {
			if (head->select == node)
				break;
		}
	}

	if (!node)
		return REPLY_ERROR;

	if (node->prev)
		node->prev->next = node->next;
	if (node->next)
		node->next->prev = node->prev;

	if (head->head == node)
		head->head = node->next;
	if (head->tail == node)
		head->tail = node->prev;
	head->count--;
	head->select = node->next;
	free(node);

	return REPLY_OK;
}

//--- rx_list_count -----------------------------------------------------------
int rx_list_count(HANDLE handle) {
	SListHead *head = (SListHead*)handle;

	if (!head)
		return -1;
	return head->count;
}

//--- rx_list_select ----------------------------------------------------------
int rx_list_select(HANDLE handle, void *elem)
{
	SListHead *head = (SListHead*)handle;
	SListNode *node;

	if (!head || !elem)
		return REPLY_ERROR;
	head->select = NULL;
	if (!head->count)
		return REPLY_OK;
	if (head->count == 1) {
		if (head->head->elem == elem) {
			head->select = head->head;
		}
		return REPLY_OK;
	}
	if (elem) {
		for (node = head->head; node; node = node->next) {
			if (node->elem == elem)
				break;
		}
		head->select = node;
	}
	return REPLY_OK;
}

//--- rx_list_elem ------------------------------------------------------------
int rx_list_elem(HANDLE handle, void **elem)
{
	SListHead *head = (SListHead*)handle;

	if (!head || !elem)
		return REPLY_ERROR;
	*elem = NULL;
	if (!head->select)
		return REPLY_OK;
	*elem = head->select->elem;
	return REPLY_OK;
}

//--- rx_list_begin -----------------------------------------------------------
int rx_list_begin(HANDLE handle, void **elem)
{
	SListHead *head = (SListHead*)handle;

	if (!head || !elem)
		return REPLY_ERROR;
	*elem = NULL;
	head->select = NULL;
	if (!head->count)
		return REPLY_OK;
	*elem = head->head->elem;
	head->select = head->head;
	return REPLY_OK;
}

//--- rx_list_end -------------------------------------------------------------
int rx_list_end(HANDLE handle, void **elem)
{
	SListHead *head = (SListHead*)handle;

	if (!head || !elem)
		return REPLY_ERROR;
	*elem = NULL;
	head->select = NULL;
	if (!head->count)
		return REPLY_OK;
	*elem = head->tail->elem;
	head->select = head->tail;
	return REPLY_OK;
}

//--- rx_list_next ------------------------------------------------------------
int rx_list_next(HANDLE handle, void **elem)
{
	SListHead *head = (SListHead*)handle;
	SListNode *node;

	if (!head || !elem || !head->select)
		return REPLY_ERROR;
	*elem = NULL;
	node = head->select;
	if (!node)
		return REPLY_ERROR;
	if (node->next) {
		node = node->next;
		*elem = node->elem;
		head->select = node;
	}
	else
		head->select = NULL;
	return REPLY_OK;
}

//--- rx_list_prev ------------------------------------------------------------
int rx_list_prev(HANDLE handle, void **elem)
{
	SListHead *head = (SListHead*)handle;
	SListNode *node;

	if (!head || !elem || !head->select)
		return REPLY_ERROR;
	*elem = NULL;
	node = head->select;
	if (!node)
		return REPLY_ERROR;
	if (node->prev) {
		node = node->prev;
		*elem = node->elem;
		head->select = node;
	}
	else
		head->select = NULL;
	return REPLY_OK;
}

