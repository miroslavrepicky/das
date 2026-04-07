#ifndef TREE23_H
#define TREE23_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * 2-3 Tree implementation (C99)
 *
 * A 2-3 tree is a balanced search tree where:
 *   - Every internal node has either 2 or 3 children.
 *   - All leaf nodes are at the same depth.
 *   - A 2-node holds 1 key and has 2 children.
 *   - A 3-node holds 2 keys and has 3 children.
 */

#include <stdbool.h>

/* ------------------------------------------------------------------ */
/*  Types                                                               */
/* ------------------------------------------------------------------ */

typedef long long T23Key;   /* Change to any comparable type as needed. */

typedef struct T23Node {
    T23Key          keys[2];      /* keys[0] (and optionally keys[1])  */
    struct T23Node *child[3];     /* child[0..1] for 2-node, [0..2] for 3-node */
    long long             num_keys;     /* 1 → 2-node,  2 → 3-node          */
} T23Node;

typedef struct {
    T23Node *root;
} T23Tree;


/**
 * tree23_init - Initialise an empty 2-3 tree.
 * @tree: pointer to the tree structure to initialise.
 */
void tree23_init(T23Tree *tree);
 
/**
 * tree23_insert - Insert a key into the tree.
 * @tree: the tree.
 * @key:  the key to insert.
 *
 * Duplicate keys are silently ignored.
 */
void tree23_insert(T23Tree *tree, T23Key key);
 
/**
 * tree23_search - Check whether a key exists in the tree.
 * @tree: the tree.
 * @key:  the key to look up.
 *
 * Returns true if the key is found, false otherwise.
 */
bool tree23_search(const T23Tree *tree, T23Key key);
 
/**
 * tree23_delete - Remove a key from the tree.
 * @tree: the tree.
 * @key:  the key to remove.
 *
 * Does nothing if the key is not present.
 */
void tree23_delete(T23Tree *tree, T23Key key);
 
/**
 * tree23_free - Release all memory used by the tree.
 * @tree: the tree.
 *
 * After this call the tree is re-initialised to an empty state and may
 * be used again without calling tree23_init() first.
 */
void tree23_free(T23Tree *tree);





#ifdef __cplusplus
}
#endif

#endif