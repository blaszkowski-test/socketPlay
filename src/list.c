
#include "list.h"

void initList(struct List *list)
{
    list->size = 0;
    list->current = NULL;
    list->first = NULL;
    list->last = NULL;
}

void listAddRoot(struct List *list, void *object)
{
    list->first = (struct Node *)malloc(sizeof(struct Node));
    list->first->object = object;
    list->first->next = NULL;
    list->first->previous = NULL;
    list->last = list->first;
    list->size = 1;
}

void listPushBack(struct List *list, void *object)
{
    if (list->size == 0)
    {
        listAddRoot(list, object);
    }
    else
    {
        struct Node *pointer = list->first;
        while (pointer->next != NULL)
        {
            pointer = pointer->next;
        }
        pointer->next = (struct Node *)malloc(sizeof(struct Node));
        pointer->next->object = object;
        pointer->next->next = NULL;
        pointer->next->previous = pointer;
        list->last = pointer->next;
        ++list->size;
    }
};

void listPushFront(struct List *list, void *object)
{
    if (list->size == 0)
    {
        listAddRoot(list, object);
    }
    else
    {
        struct Node *pointer = list->first;

        pointer->previous = (struct Node *)malloc(sizeof(struct Node));
        pointer->previous->object = object;
        pointer->previous->next = pointer;
        pointer->previous->previous = NULL;
        list->first = pointer->previous;
        ++list->size;
    }
};

void listInsert(struct List *list, void *object, int index)
{
    if (index < 1 || index >= list->size)
    {
        return;
    }

    int counter = 0;
    struct Node *pointer = list->first;
    struct Node *previous = NULL;

    while (pointer->next != NULL && counter != index)
    {
        pointer = pointer->next;
        ++counter;
    }

    previous = pointer->previous;
    previous->next = (struct Node *)malloc(sizeof(struct Node));
    previous->next->object = object;
    previous->next->next = pointer;
    previous->next->previous = previous;
}

void *listGet(struct List *list, int index)
{
    if (index < 0 || index >= list->size)
    {
        return ((void *)0);
    }
    int counter = 0;
    struct Node *pointer = list->first;
    while (pointer->next != NULL && counter != index)
    {
        pointer = pointer->next;
        ++counter;
    }

    return pointer->object;
}

bool listRemove(struct List *list, int index, bool freeObject)
{
    if (index < 0 || index >= list->size)
    {
        return false;
    }

    int counter = 0;
    struct Node *pointer = list->first;
    while (pointer->next != NULL && counter != index)
    {
        pointer = pointer->next;
        ++counter;
    }

    if (list->size == 1)
    {
        freeList(list, false);
        return true;
    }
    else if (index == 0)
    {
        list->first = pointer->next;
    }
    else
    {
        pointer->previous->next = pointer->next;
    }
    if (freeObject)
    {
        free(pointer->object);
    }
    free(pointer);
    list->size -= 1;

    return true;
}

bool listHasNext(struct List *list)
{
    if (list->first == NULL)
    {
        return false;
    }

    if (list->current == NULL)
    {
        return true;
    }

    return list->current->next != NULL;
}

void *listNext(struct List *list)
{
    if (list->current == NULL)
    {
        list->current = list->first;
        return list->current->object;
    }

    if (list->current->next != NULL)
    {
        list->current = list->current->next;
    }

    return list->current->object;
}

bool listHasPrev(struct List *list)
{
    if (list->last == NULL)
    {
        return false;
    }

    if (list->current == NULL)
    {
        return true;
    }

    return list->current->previous != NULL;
}

void *listPrev(struct List *list)
{
    if (list->current == NULL)
    {
        list->current = list->last;
        return list->current->object;
    }

    if (list->current->previous != NULL)
    {
        list->current = list->current->previous;
    }

    return list->current->object;
}

void resetIterator(struct List *list)
{
    list->current = NULL;
}

void freeNode(struct Node *node, bool freeObject)
{
    if (node->next != NULL)
    {
        freeNode(node->next, freeObject);
    }
    if (freeObject)
    {
        free(node->object);
    }
    free(node);
}

void freeList(struct List *list, bool freeObject)
{
    freeNode(list->first, freeObject);
    list->first = NULL;
    list->last = NULL;
    list->current = NULL;
    list->size = 0;
}