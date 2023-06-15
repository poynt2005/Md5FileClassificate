#ifndef __MY_BUFF_H__
#define __MY_BUFF_H__

#include <string.h>

#ifdef __WIN32
#include <Windows.h>
#define ALLOC(ALLOCATE_TYPE) (ALLOCATE_TYPE *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(ALLOCATE_TYPE))
#define ALLOC_N(ALLOCATE_TYPE, N) (ALLOCATE_TYPE *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, N)
#define DEALLOC(ALLOCATED_INSTANCE) HeapFree(GetProcessHeap(), 0, ALLOCATED_INSTANCE)
#else
#define ALLOC(ALLOCATE_TYPE) (ALLOCATE_TYPE *)malloc(sizeof(ALLOCATE_TYPE))
#define ALLOC_N(ALLOCATE_TYPE, N)                             \
    {                                                         \
        ALLOCATE_TYPE *INSTANCE = (ALLOCATE_TYPE *)malloc(N); \
        memset(INSTANCE, 0, N)                                \
    }
#define DEALLOC(ALLOCATED_INSTANCE) free(ALLOCATED_INSTANCE)
#endif

enum ELEM_STATUS
{
    // For store
    NODE_EMPTY = 0,
    NODE_OCCUPY,
    NODE_DELETE,

    // For search
    NODE_INASSABLE,
    NODE_ASSABLE
};

struct BuffNode
{
    int status;
    int newAlloc;
    void *elem;
    struct BuffNode *next;
};

struct BuffList
{
    struct BuffNode *nodeList;
    int cap;
    int size;
};

typedef struct BuffList MyBuff;

static inline struct BuffList *NewBuff()
{
    struct BuffList *list = ALLOC(struct BuffList);

    struct BuffNode *node = ALLOC(struct BuffNode);
    node->elem = NULL;
    node->next = NULL;
    node->status = NODE_EMPTY;
    node->newAlloc = 0;

    list->nodeList = node;
    list->cap = 1;
    list->size = 0;

    struct BuffNode *it = list->nodeList;

    return list;
}

static inline void *AtBuff(struct BuffList *list, int idx, int *status)
{
    if (idx >= list->size)
    {
        if (status != NULL)
        {
            *status = NODE_INASSABLE;
        }

        return NULL;
    }

    if (idx < 0)
    {
        if (status != NULL)
        {
            *status = NODE_INASSABLE;
        }
        return NULL;
    }

    struct BuffNode *it = list->nodeList;
    int currentIdx = 0;

    while (currentIdx != idx)
    {
        ++currentIdx;
        it = it->next;
    }

    if (it->status == NODE_OCCUPY)
    {
        if (status != NULL)
        {
            *status = NODE_ASSABLE;
        }
        return it->elem;
    }

    if (status != NULL)
    {
        *status = NODE_INASSABLE;
    }
    return NULL;
}

static inline int SizeBuff(struct BuffList *list)
{
    return list->size;
}

static inline int CapsBuff(struct BuffList *list)
{
    return list->cap;
}

static inline int PushBuff(struct BuffList *list, void *elem, int newAllocSize)
{
    int growth = 0;
    if (list->cap <= list->size)
    {
        list->cap = (list->cap) * 2;
        growth = list->cap - list->size;
    }

    struct BuffNode *it = list->nodeList;

    for (it; it->next != NULL; it = it->next)
    {
    }

    while ((--growth) >= 0)
    {
        struct BuffNode *node = ALLOC(struct BuffNode);

        node->elem = NULL;
        node->next = NULL;
        node->status = NODE_EMPTY;

        it->next = node;
        it = it->next;
    }

    it = list->nodeList;
    struct BuffNode *lastEmptyNode = NULL;
    for (it; it != NULL; it = it->next)
    {
        if (it->status == NODE_EMPTY && lastEmptyNode == NULL)
        {
            lastEmptyNode = it;
            break;
        }
    }

    lastEmptyNode->status = NODE_OCCUPY;

    if (newAllocSize > 0)
    {
        lastEmptyNode->newAlloc = 1;
        lastEmptyNode->elem = ALLOC_N(void, newAllocSize);
        memcpy(lastEmptyNode->elem, elem, newAllocSize);
    }
    else
    {
        lastEmptyNode->elem = elem;
    }

    list->size = (list->size) + 1;
    return list->size;
}

static inline int HasBuff(struct BuffList *list, void *elem)
{
    struct BuffNode *it = list->nodeList;

    for (it; it != NULL; it = it->next)
    {
        if (it->elem == elem && it->status == NODE_OCCUPY)
        {
            return 1;
        }
    }

    return 0;
}

static inline int IndexOfBuff(struct BuffList *list, void *elem)
{
    struct BuffNode *it = list->nodeList;
    int idx = 0;

    for (it; it != NULL; it = it->next)
    {
        if (it->elem == elem && it->status == NODE_OCCUPY)
        {
            return idx;
        }
        ++idx;
    }

    return -1;
}

static inline int EraseBuff(struct BuffList *list, int idx)
{
    if (idx >= list->size)
    {
        return list->size;
    }

    if (idx < 0)
    {
        return list->size;
    }

    struct BuffNode *it = list->nodeList;
    int canErase = 0;

    if (idx == 0 && it->status == NODE_OCCUPY)
    {
        list->nodeList = list->nodeList->next;
        canErase = 1;
    }
    else
    {
        struct BuffNode *prevIt = NULL;

        int currentIdx = 0;

        while (currentIdx != idx)
        {
            ++currentIdx;
            prevIt = it;
            it = it->next;
        }

        if (it->status == NODE_OCCUPY)
        {
            prevIt->next = it->next;

            canErase = 1;
        }
    }

    if (canErase)
    {
        if (it->newAlloc)
        {
            DEALLOC(it->elem);
        }

        DEALLOC(it);
        it = NULL;

        list->size = list->size - 1;

        int currentIdx = 0;

        it = list->nodeList;
        for (it; it->next != NULL; it = it->next)
        {
        }

        struct BuffNode *node = ALLOC(struct BuffNode);

        node->elem = NULL;
        node->next = NULL;
        node->status = NODE_EMPTY;
        it->next = node;
    }

    return list->size;
}

static inline void FreeBuff(struct BuffList **list)
{
    struct BuffNode *it = (*list)->nodeList;
    struct BuffNode *next = NULL;

    while (it != NULL)
    {
        next = it->next;

        if (it->newAlloc)
        {
            DEALLOC(it->elem);
            it->elem = NULL;
        }

        DEALLOC(it);
        it = next;
    }

    // DEALLOC((*list)->nodeList);
    // (*list)->nodeList = NULL;
    // (*list)->size = 0;
    // (*list)->cap = 0;

    DEALLOC(*list);
    *list = NULL;
}

#endif