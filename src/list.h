/* 
 * File:   struct_test.h
 * Author: piotr
 *
 * Created on June 17, 2017, 1:16 PM
 */

#ifndef STRUCT_LIST_H
#define	STRUCT_LIST_H

#include <stdio.h>
#include <stdlib.h>

#ifdef	__cplusplus
extern "C"
{
#endif

    typedef enum
    {
        false, true
    } bool;

    struct Node
    {
        void * object;
        struct Node * next;
        struct Node * previous;
    };

    struct List
    {
        int size;
        struct Node* first;
        struct Node* last;
        struct Node * current;
    };

    void initList(struct List * list);
    void listPushBack(struct List * one, void * object);
    void listPushFront(struct List * one, void * object);
    void listInsert(struct List * one, void * object, int index);
    void * listGet(struct List * one, int index);
    bool listRemove(struct List *list, int index, bool freeObject);

    bool listHasNext(struct List * list);
    void * listNext(struct List * list);
    bool listHasPrev(struct List * list);
    void * listPrev(struct List * list);
    void resetIterator(struct List * list);
    
    void freeNode(struct Node * one, bool freeObject);
    void freeList(struct List * one, bool freeObject);


#ifdef	__cplusplus
}
#endif

#endif	/* STRUCT_LIST_H */

