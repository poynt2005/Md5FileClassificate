#ifndef __C_MAP_H__
#define __C_MAP_H__

#include <Windows.h>
#include <string.h>

#ifdef __WIN32
#define ALLOC(ALLOCATE_TYPE) (ALLOCATE_TYPE *)HeapAlloc(GetProcessHeap(), 0, sizeof(ALLOCATE_TYPE))
#define DEALLOC(ALLOCATED_INSTANCE) HeapFree(GetProcessHeap(), 0, ALLOCATED_INSTANCE)
#else
#define ALLOC(ALLOCATE_TYPE) (ALLOCATE_TYPE *)malloc(sizeof(ALLOCATE_TYPE))
#define DEALLOC(ALLOCATED_INSTANCE) free(ALLOCATED_INSTANCE)
#endif

struct Node
{
    unsigned long key;
    void *el;
    struct Node *next;
};

typedef struct Node *Map;

static inline unsigned long MapKeyCalcul(const char *key)
{
    const char *base = "PASSWORD";
    unsigned long digit = 1;
    for (int i = 0; i < strlen(key); i++)
    {
        digit = (digit << (unsigned long)i) ^ (key[i] ^ base[(i + 1) % strlen(base)]);
    }
    return digit;
}

static inline struct Node *MapCreate()
{
    struct Node *map = ALLOC(struct Node);

    map->key = 0;
    map->el = NULL;
    map->next = NULL;

    return map;
}

static inline void *MapInsert(struct Node *map, const char *key, void *el)
{

    unsigned long keyHash = MapKeyCalcul(key);

    if (map->el == NULL)
    {
        map->key = keyHash;
        map->el = el;
    }
    else
    {
        struct Node *it = map;

        for (it; it->next != NULL; it = it->next)
        {
            if (it->key == keyHash)
            {
                it->el = el;
                return el;
            }
        }

        it->next = ALLOC(struct Node);
        it->next->key = keyHash;
        it->next->el = el;
    }

    return el;
}

static inline void *MapFind(struct Node *map, const char *key)
{
    unsigned long keyHash = MapKeyCalcul(key);

    if (map->el == NULL)
    {
        return NULL;
    }

    if (map->key == keyHash)
    {
        return map->el;
    }

    struct Node *it = map;

    for (it; it != NULL; it = it->next)
    {
        if (it->key == keyHash)
        {
            return it->el;
        }
    }

    return NULL;
}

static inline void *MapErase(struct Node **map, const char *key)
{
    unsigned long keyHash = MapKeyCalcul(key);

    if ((*map)->el == NULL)
    {
        return NULL;
    }

    if ((*map)->key == keyHash && (*map)->next == NULL)
    {
        void *el = (*map)->el;
        DEALLOC((*map));
        return el;
    }

    struct Node *it = (*map);
    struct Node *prev = NULL;

    for (it; it != NULL; it = it->next)
    {
        if (it->key == keyHash)
        {
            if (prev != NULL)
            {
                prev->next = it->next;
                void *el = it->el;
                DEALLOC(it);
                return el;
            }
            else
            {
                (*map) = it->next;
                void *el = it->el;
                DEALLOC(it);
                return el;
            }
        }
        prev = it;
    }

    return NULL;
}

static inline void MapClear(struct Node **map)
{

    if ((*map)->next == NULL)
    {
        DEALLOC((*map));
        (*map) = NULL;
        return;
    }

    struct Node *it = (*map);
    struct Node *nextNode = NULL;

    while (it != NULL)
    {
        nextNode = it->next;
        DEALLOC(it);
        it = nextNode;
    }

    (*map) = NULL;
}

#endif