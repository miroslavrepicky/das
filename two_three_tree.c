/*
 * tree23.c  –  2-3 Tree  (C99)
 *
 * Build:
 *   gcc -std=c99 -Wall -Wextra -o demo tree23.c
 *
 * The implementation follows the classic textbook approach:
 *   Insert : split on the way back up (bottom-up splits).
 *   Delete : merge / redistribute on the way back up.
 *
 * All helper functions are static (translation-unit private).
 */

#include "two_three_tree.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ================================================================== */
/*  Internal helpers – node allocation / key comparison               */
/* ================================================================== */

static T23Node *node_new(void)
{
    T23Node *n = (T23Node *)calloc(1, sizeof(T23Node));
    if (!n) {
        fprintf(stderr, "tree23: out of memory\n");
        exit(EXIT_FAILURE);
    }
    return n;
}

/* Comparison wrapper – keeps the code easy to adapt to other key types. */
static inline long long cmp(T23Key a, T23Key b)
{
    return (a > b) - (a < b);   /* -1 / 0 / +1 */
}

/* ================================================================== */
/*  Public: init / free                                                */
/* ================================================================== */

void tree23_init(T23Tree *tree)
{
    assert(tree);
    tree->root = NULL;
}

static void free_nodes(T23Node *n)
{
    if (!n) return;
    free_nodes(n->child[0]);
    free_nodes(n->child[1]);
    free_nodes(n->child[2]);
    free(n);
}

void tree23_free(T23Tree *tree)
{
    assert(tree);
    free_nodes(tree->root);
    tree->root = NULL;
}

/* ================================================================== */
/*  Search                                                             */
/* ================================================================== */

static bool node_search(const T23Node *n, T23Key key)
{
    while (n) {
        long long c0 = cmp(key, n->keys[0]);
        if (c0 == 0) return true;

        if (n->num_keys == 2) {
            long long c1 = cmp(key, n->keys[1]);
            if (c1 == 0) return true;

            if (c0 < 0)
                n = n->child[0];
            else if (c1 < 0)
                n = n->child[1];
            else
                n = n->child[2];
        } else {
            n = (c0 < 0) ? n->child[0] : n->child[1];
        }
    }
    return false;
}

bool tree23_search(const T23Tree *tree, T23Key key)
{
    assert(tree);
    return node_search(tree->root, key);
}

/* ================================================================== */
/*  Insert                                                             */
/* ================================================================== */

/*
 * A "split result" carries the median key and the two halves that
 * result from splitting a temporarily-overfull (4-node) node.
 */
typedef struct {
    bool    split;      /* true  → the child produced a split           */
    T23Key  up_key;     /* key that must be pushed up to the parent      */
    T23Node *left;      /* left  sub-tree of the split                   */
    T23Node *right;     /* right sub-tree of the split                   */
} SplitResult;

static SplitResult insert_rec(T23Node *n, T23Key key);

/*
 * Absorb a split result (up_key, left, right) into node n at position pos.
 * If n is a 2-node it simply becomes a 3-node (no further split).
 * If n is a 3-node it overflows → we return another split upward.
 */
static SplitResult absorb_split(T23Node *n, long long pos,
                                T23Key up_key,
                                T23Node *left, T23Node *right)
{
    /* ---------- n is a 2-node: just grow it into a 3-node ---------- */
    if (n->num_keys == 1) {
        if (pos == 0) {
            /* insert on the left */
            n->keys[1]  = n->keys[0];
            n->keys[0]  = up_key;
            n->child[2] = n->child[1];
            n->child[0] = left;
            n->child[1] = right;
        } else {
            /* insert on the right */
            n->keys[1]  = up_key;
            n->child[1] = left;
            n->child[2] = right;
        }
        n->num_keys = 2;
        return (SplitResult){ .split = false };
    }

    /* ---------- n is a 3-node: temporarily build a 4-node, then split ---------- */
    /*
     * Represent the 4-node as arrays of 3 keys and 4 children,
     * then find the median and produce two 2-nodes.
     */
    T23Key   k[3];
    T23Node *c[4];

    /* Copy existing state */
    k[0] = n->keys[0]; k[1] = n->keys[1];
    c[0] = n->child[0]; c[1] = n->child[1]; c[2] = n->child[2];

    /* Insert the new key/children at position pos */
    if (pos == 0) {
        k[2] = k[1]; k[1] = k[0]; k[0] = up_key;
        c[3] = c[2]; c[2] = c[1]; c[0] = left; c[1] = right;
    } else if (pos == 1) {
        k[2] = k[1]; k[1] = up_key;
        c[3] = c[2]; c[1] = left; c[2] = right;
    } else {
        k[2] = up_key;
        c[2] = left; c[3] = right;
    }

    /* median is k[1]; left half gets k[0], right half gets k[2] */
    T23Node *lnode = n;   /* reuse the existing allocation for the left node */
    T23Node *rnode = node_new();

    lnode->num_keys  = 1;
    lnode->keys[0]   = k[0];
    lnode->child[0]  = c[0];
    lnode->child[1]  = c[1];
    lnode->child[2]  = NULL;

    rnode->num_keys  = 1;
    rnode->keys[0]   = k[2];
    rnode->child[0]  = c[2];
    rnode->child[1]  = c[3];
    rnode->child[2]  = NULL;

    return (SplitResult){ .split = true, .up_key = k[1],
                          .left  = lnode, .right  = rnode };
}

static SplitResult insert_rec(T23Node *n, T23Key key)
{
    /* --- leaf node --- */
    if (!n->child[0]) {
        /* Insert key into this leaf */
        if (n->num_keys == 1) {
            long long c0 = cmp(key, n->keys[0]);
            if (c0 == 0) return (SplitResult){ .split = false }; /* duplicate */
            if (c0 < 0) {
                n->keys[1] = n->keys[0];
                n->keys[0] = key;
            } else {
                n->keys[1] = key;
            }
            n->num_keys = 2;
            return (SplitResult){ .split = false };
        } else { /* 3-node leaf – must split */
            T23Key k[3];
            if (cmp(key, n->keys[0]) == 0 || cmp(key, n->keys[1]) == 0)
                return (SplitResult){ .split = false }; /* duplicate */

            k[0] = n->keys[0]; k[1] = n->keys[1]; k[2] = key;
            /* sort three values */
            if (k[0] > k[1]) { T23Key t = k[0]; k[0] = k[1]; k[1] = t; }
            if (k[1] > k[2]) { T23Key t = k[1]; k[1] = k[2]; k[2] = t; }
            if (k[0] > k[1]) { T23Key t = k[0]; k[0] = k[1]; k[1] = t; }

            T23Node *lnode = n;
            T23Node *rnode = node_new();

            lnode->num_keys = 1; lnode->keys[0] = k[0];
            rnode->num_keys = 1; rnode->keys[0] = k[2];
            /* children stay NULL (leaves) */
            lnode->child[0] = lnode->child[1] = lnode->child[2] = NULL;
            rnode->child[0] = rnode->child[1] = rnode->child[2] = NULL;

            return (SplitResult){ .split = true, .up_key = k[1],
                                  .left  = lnode, .right  = rnode };
        }
    }

    /* --- internal node: recurse into the correct child --- */
    long long pos;
    SplitResult sr;

    long long c0 = cmp(key, n->keys[0]);
    if (c0 == 0) return (SplitResult){ .split = false }; /* duplicate */

    if (n->num_keys == 2) {
        long long c1 = cmp(key, n->keys[1]);
        if (c1 == 0) return (SplitResult){ .split = false }; /* duplicate */
        if (c0 < 0)      { pos = 0; sr = insert_rec(n->child[0], key); }
        else if (c1 < 0) { pos = 1; sr = insert_rec(n->child[1], key); }
        else             { pos = 2; sr = insert_rec(n->child[2], key); }
    } else {
        if (c0 < 0) { pos = 0; sr = insert_rec(n->child[0], key); }
        else        { pos = 1; sr = insert_rec(n->child[1], key); }
    }

    if (!sr.split) return (SplitResult){ .split = false };

    /* Child split – absorb the split result */
    return absorb_split(n, pos, sr.up_key, sr.left, sr.right);
}

void tree23_insert(T23Tree *tree, T23Key key)
{
    assert(tree);

    if (!tree->root) {
        tree->root = node_new();
        tree->root->num_keys = 1;
        tree->root->keys[0]  = key;
        return;
    }

    SplitResult sr = insert_rec(tree->root, key);

    if (sr.split) {
        /* The root was split – create a new root */
        T23Node *new_root = node_new();
        new_root->num_keys = 1;
        new_root->keys[0]  = sr.up_key;
        new_root->child[0] = sr.left;
        new_root->child[1] = sr.right;
        tree->root = new_root;
    }
}

/* ================================================================== */
/*  Delete                                                             */
/* ================================================================== */

/*
 * Deletion strategy (classic 2-3 approach):
 *
 * 1. Find the key. If it is in an internal node, swap it with its
 *    in-order successor (which is always in a leaf), then delete
 *    from the leaf.
 * 2. If the leaf is a 3-node, simply remove the key → done.
 * 3. If the leaf is a 2-node it becomes "empty" (underflow). Repair
 *    on the way back up:
 *      a. If a sibling is a 3-node → redistribute (rotate).
 *      b. Otherwise → merge with a sibling (and pull down a key from parent).
 *         The parent may then underflow → propagate upward.
 */

/* Return values from delete_rec: describes what happened to child[pos]. */
typedef enum {
    DEL_OK,        /* subtree is fine                */
    DEL_UNDERFLOW, /* the child at pos is now empty  */
    DEL_NOT_FOUND  /* key was not in this subtree    */
} DelStatus;

static DelStatus delete_rec(T23Node *n, T23Key key);

/*
 * Find the minimum key in a subtree (= leftmost leaf key[0]).
 */
static T23Key subtree_min(T23Node *n)
{
    while (n->child[0]) n = n->child[0];
    return n->keys[0];
}

/*
 * Fix an underflow in child[pos] of node n.
 * Returns DEL_OK or DEL_UNDERFLOW (if n itself now underflows).
 */
static DelStatus fix_underflow(T23Node *n, long long pos)
{
    /*
     * We have 3 cases depending on whether n is a 2-node or 3-node,
     * and which sibling(s) we can borrow from.
     */

    /* ---------------------------------------------------------------- */
    /* n is a 3-node (has 3 children; pos ∈ {0,1,2})                   */
    /* ---------------------------------------------------------------- */
    if (n->num_keys == 2) {
        if (pos == 0) {
            T23Node *sib = n->child[1];
            if (sib->num_keys == 2) {
                /* rotate: move n->keys[0] down into child[0], move sib->keys[0] up */
                T23Node *def = n->child[0];

                T23Node *new_def = node_new();
                new_def->num_keys = 1;
                new_def->keys[0]  = n->keys[0];
                new_def->child[0] = def->child[0]; /* child[0] was empty; carry its subtree */
                new_def->child[1] = sib->child[0];

                free(def);
                n->child[0] = new_def;
                n->keys[0]  = sib->keys[0];
                sib->keys[0] = sib->keys[1];
                sib->child[0] = sib->child[1];
                sib->child[1] = sib->child[2];
                sib->child[2] = NULL;
                sib->num_keys = 1;
                return DEL_OK;
            } else {
                /* merge child[0] (empty) + n->keys[0] + sib into one 3-node */
                T23Node *def = n->child[0];
                sib->keys[1]  = sib->keys[0];
                sib->keys[0]  = n->keys[0];
                sib->child[2] = sib->child[1];
                sib->child[1] = sib->child[0];
                sib->child[0] = def->child[0];
                sib->num_keys = 2;
                free(def);
                /* pull the separator up */
                n->keys[0]  = n->keys[1];
                n->child[0] = sib;
                n->child[1] = n->child[2];
                n->child[2] = NULL;
                n->num_keys = 1;
                return DEL_OK; /* n is still a 2-node, no underflow */
            }
        } else if (pos == 1) {
            /* Try left sibling first */
            T23Node *lsib = n->child[0];
            T23Node *def  = n->child[1];
            if (lsib->num_keys == 2) {
                T23Node *new_def = node_new();
                new_def->num_keys = 1;
                new_def->keys[0]  = n->keys[0];
                new_def->child[0] = lsib->child[2];
                new_def->child[1] = def->child[0];
                free(def);
                n->child[1] = new_def;
                n->keys[0]  = lsib->keys[1];
                lsib->child[2] = NULL;
                lsib->num_keys = 1;
                return DEL_OK;
            }
            /* Try right sibling */
            T23Node *rsib = n->child[2];
            if (rsib->num_keys == 2) {
                T23Node *new_def = node_new();
                new_def->num_keys = 1;
                new_def->keys[0]  = n->keys[1];
                new_def->child[0] = def->child[0];
                new_def->child[1] = rsib->child[0];
                free(def);
                n->child[1] = new_def;
                n->keys[1]  = rsib->keys[0];
                rsib->keys[0] = rsib->keys[1];
                rsib->child[0] = rsib->child[1];
                rsib->child[1] = rsib->child[2];
                rsib->child[2] = NULL;
                rsib->num_keys = 1;
                return DEL_OK;
            }
            /* merge with left sibling */
            lsib->keys[1]  = n->keys[0];
            lsib->child[2] = def->child[0];
            lsib->num_keys = 2;
            free(def);
            n->keys[0]  = n->keys[1];
            n->child[1] = n->child[2];
            n->child[2] = NULL;
            n->num_keys = 1;
            return DEL_OK;
        } else { /* pos == 2 */
            T23Node *sib = n->child[1];
            T23Node *def = n->child[2];
            if (sib->num_keys == 2) {
                T23Node *new_def = node_new();
                new_def->num_keys = 1;
                new_def->keys[0]  = n->keys[1];
                new_def->child[0] = sib->child[2];
                new_def->child[1] = def->child[0];
                free(def);
                n->child[2] = new_def;
                n->keys[1]  = sib->keys[1];
                sib->child[2] = NULL;
                sib->num_keys = 1;
                return DEL_OK;
            }
            /* merge */
            sib->keys[1]  = n->keys[1];
            sib->child[2] = def->child[0];
            sib->num_keys = 2;
            free(def);
            n->child[2] = NULL;
            n->num_keys = 1;
            return DEL_OK;
        }
    }

    /* ---------------------------------------------------------------- */
    /* n is a 2-node (has 2 children; pos ∈ {0,1})                     */
    /* ---------------------------------------------------------------- */
    T23Node *def, *sib;

    if (pos == 0) {
        def = n->child[0]; sib = n->child[1];
    } else {
        def = n->child[1]; sib = n->child[0];
    }

    if (sib->num_keys == 2) {
        /* Redistribute */
        T23Node *new_def = node_new();
        new_def->num_keys = 1;

        if (pos == 0) {
            new_def->keys[0]  = n->keys[0];
            new_def->child[0] = def->child[0];
            new_def->child[1] = sib->child[0];
            n->keys[0]        = sib->keys[0];
            sib->keys[0]      = sib->keys[1];
            sib->child[0]     = sib->child[1];
            sib->child[1]     = sib->child[2];
        } else {
            new_def->keys[0]  = n->keys[0];
            new_def->child[0] = sib->child[2];
            new_def->child[1] = def->child[0];
            n->keys[0]        = sib->keys[1];
        }
        sib->child[2] = NULL;
        sib->num_keys = 1;
        free(def);
        n->child[pos] = new_def;
        return DEL_OK;
    }

    /* Merge: the two children and the separator key form a 3-node. */
    T23Node *merged;
    if (pos == 0) {
        merged = sib; /* reuse sib */
        merged->keys[1]  = merged->keys[0];
        merged->keys[0]  = n->keys[0];
        merged->child[2] = merged->child[1];
        merged->child[1] = merged->child[0];
        merged->child[0] = def->child[0];
        merged->num_keys = 2;
        free(def);
        n->child[0] = merged;
        n->child[1] = NULL;
    } else {
        merged = sib; /* reuse sib */
        merged->keys[1]  = n->keys[0];
        merged->child[2] = def->child[0];
        merged->num_keys = 2;
        free(def);
        n->child[1] = NULL;
    }
    /* n (a 2-node) has lost a child → underflow */
    n->num_keys = 0; /* sentinel: will be cleaned up by caller */
    return DEL_UNDERFLOW;
}

static DelStatus delete_rec(T23Node *n, T23Key key)
{
    long long c0 = cmp(key, n->keys[0]);

    /* ---- leaf ---- */
    if (!n->child[0]) {
        if (n->num_keys == 2) {
            long long c1 = cmp(key, n->keys[1]);
            if (c0 == 0) {
                n->keys[0] = n->keys[1];
                n->num_keys = 1;
                return DEL_OK;
            }
            if (c1 == 0) {
                n->num_keys = 1;
                return DEL_OK;
            }
            return DEL_NOT_FOUND;
        } else {
            if (c0 == 0) {
                n->num_keys = 0; /* will signal underflow */
                return DEL_UNDERFLOW;
            }
            return DEL_NOT_FOUND;
        }
    }

    /* ---- internal node ---- */
    long long child_pos;
    DelStatus st;

    if (n->num_keys == 2) {
        long long c1 = cmp(key, n->keys[1]);

        if (c0 == 0) {
            /* Replace with in-order successor, then delete successor */
            T23Key succ = subtree_min(n->child[1]);
            n->keys[0]  = succ;
            child_pos   = 1;
            st = delete_rec(n->child[1], succ);
        } else if (c1 == 0) {
            T23Key succ = subtree_min(n->child[2]);
            n->keys[1]  = succ;
            child_pos   = 2;
            st = delete_rec(n->child[2], succ);
        } else if (c0 < 0) {
            child_pos = 0;
            st = delete_rec(n->child[0], key);
        } else if (c1 < 0) {
            child_pos = 1;
            st = delete_rec(n->child[1], key);
        } else {
            child_pos = 2;
            st = delete_rec(n->child[2], key);
        }
    } else {
        if (c0 == 0) {
            T23Key succ = subtree_min(n->child[1]);
            n->keys[0]  = succ;
            child_pos   = 1;
            st = delete_rec(n->child[1], succ);
        } else if (c0 < 0) {
            child_pos = 0;
            st = delete_rec(n->child[0], key);
        } else {
            child_pos = 1;
            st = delete_rec(n->child[1], key);
        }
    }

    if (st == DEL_NOT_FOUND) return DEL_NOT_FOUND;
    if (st == DEL_OK)        return DEL_OK;

    /* Child underflowed */
    return fix_underflow(n, child_pos);
}

void tree23_delete(T23Tree *tree, T23Key key)
{
    assert(tree);
    if (!tree->root) return;

    DelStatus st = delete_rec(tree->root, key);

    if (st == DEL_UNDERFLOW) {
        /* Root became empty – shrink the tree */
        T23Node *old_root = tree->root;
        tree->root = old_root->child[0]; /* may be NULL for empty tree */
        free(old_root);
    }
}

/* ================================================================== */
/*  Prlong long (in-order, indented)                                        */
/* ================================================================== */

static void print_rec(const T23Node *n, long long depth)
{
    if (!n) return;

    /* Prlong long right subtree first (rotated view) */
    if (n->num_keys == 2)
        print_rec(n->child[2], depth + 1);

    print_rec(n->child[1], depth + 1);

    for (long long i = 0; i < depth; ++i) printf("    ");

    if (n->num_keys == 2)
        printf("[%lld | %lld]\n", n->keys[0], n->keys[1]);
    else
        printf("[%lld]\n", n->keys[0]);

    print_rec(n->child[0], depth + 1);
}

void tree23_print(const T23Tree *tree)
{
    assert(tree);
    if (!tree->root) {
        printf("(empty tree)\n");
        return;
    }
    print_rec(tree->root, 0);
}
