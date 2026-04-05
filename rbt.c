/**
 * rbt.c - Implementácia červeno-čierneho stromu (C99)
 */

#include "rbt.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

/* ------------------------------------------------------------------ */
/*  Interné pomocné makrá                                               */
/* ------------------------------------------------------------------ */

#define IS_NIL(tree, node)  ((node) == (tree)->nil)
#define IS_RED(tree, node)  (!IS_NIL(tree, node) && (node)->color == RBT_RED)
#define IS_BLACK(tree, node) (!IS_RED(tree, node))

/* ------------------------------------------------------------------ */
/*  Interné funkcie – rotácie a opravy                                  */
/* ------------------------------------------------------------------ */

/* Ľavá rotácia okolo uzla x:
 *
 *      x                y
 *     / \              / \
 *    a   y    -->     x   c
 *       / \          / \
 *      b   c        a   b
 */
static void rotate_left(rbt_tree_t *tree, rbt_node_t *x)
{
    rbt_node_t *y = x->right;

    /* b sa stane pravým synom x */
    x->right = y->left;
    if (!IS_NIL(tree, y->left))
        y->left->parent = x;

    /* y preberá pozíciu x */
    y->parent = x->parent;
    if (IS_NIL(tree, x->parent))
        tree->root = y;
    else if (x == x->parent->left)
        x->parent->left = y;
    else
        x->parent->right = y;

    /* x sa stane ľavým synom y */
    y->left   = x;
    x->parent = y;
}

/* Pravá rotácia okolo uzla y (symetrická k rotate_left) */
static void rotate_right(rbt_tree_t *tree, rbt_node_t *y)
{
    rbt_node_t *x = y->left;

    y->left = x->right;
    if (!IS_NIL(tree, x->right))
        x->right->parent = y;

    x->parent = y->parent;
    if (IS_NIL(tree, y->parent))
        tree->root = x;
    else if (y == y->parent->left)
        y->parent->left = x;
    else
        y->parent->right = x;

    x->right  = y;
    y->parent = x;
}

/* ------------------------------------------------------------------ */
/*  Oprava po vložení                                                   */
/* ------------------------------------------------------------------ */

static void insert_fixup(rbt_tree_t *tree, rbt_node_t *z)
{
    while (IS_RED(tree, z->parent)) {
        rbt_node_t *gp = z->parent->parent; /* starý otec */

        if (z->parent == gp->left) {
            /* rodič je ľavý syn starého otca */
            rbt_node_t *uncle = gp->right;

            if (IS_RED(tree, uncle)) {
                /* Prípad 1: strýko je červený → prefarbenie */
                z->parent->color = RBT_BLACK;
                uncle->color     = RBT_BLACK;
                gp->color        = RBT_RED;
                z = gp;
            } else {
                if (z == z->parent->right) {
                    /* Prípad 2: z je pravý syn → ľavá rotácia */
                    z = z->parent;
                    rotate_left(tree, z);
                }
                /* Prípad 3: z je ľavý syn → pravá rotácia */
                z->parent->color = RBT_BLACK;
                gp->color        = RBT_RED;
                rotate_right(tree, gp);
            }
        } else {
            /* Symetrický prípad: rodič je pravý syn starého otca */
            rbt_node_t *uncle = gp->left;

            if (IS_RED(tree, uncle)) {
                z->parent->color = RBT_BLACK;
                uncle->color     = RBT_BLACK;
                gp->color        = RBT_RED;
                z = gp;
            } else {
                if (z == z->parent->left) {
                    z = z->parent;
                    rotate_right(tree, z);
                }
                z->parent->color = RBT_BLACK;
                gp->color        = RBT_RED;
                rotate_left(tree, gp);
            }
        }
    }
    tree->root->color = RBT_BLACK;
}

/* ------------------------------------------------------------------ */
/*  Oprava po zmazaní                                                   */
/* ------------------------------------------------------------------ */

/*
 * Presunie podstrom zakorenený v v na miesto podstromu u.
 * Používa sa pri mazaní.
 */
static void transplant(rbt_tree_t *tree, rbt_node_t *u, rbt_node_t *v)
{
    if (IS_NIL(tree, u->parent))
        tree->root = v;
    else if (u == u->parent->left)
        u->parent->left = v;
    else
        u->parent->right = v;

    v->parent = u->parent;
}

static void delete_fixup(rbt_tree_t *tree, rbt_node_t *x)
{
    while (x != tree->root && IS_BLACK(tree, x)) {
        if (x == x->parent->left) {
            rbt_node_t *w = x->parent->right; /* brat */

            /* Prípad 1: brat je červený */
            if (IS_RED(tree, w)) {
                w->color         = RBT_BLACK;
                x->parent->color = RBT_RED;
                rotate_left(tree, x->parent);
                w = x->parent->right;
            }

            if (IS_BLACK(tree, w->left) && IS_BLACK(tree, w->right)) {
                /* Prípad 2: obaja synovia brata sú čierni */
                w->color = RBT_RED;
                x = x->parent;
            } else {
                if (IS_BLACK(tree, w->right)) {
                    /* Prípad 3: pravý syn brata je čierny */
                    w->left->color = RBT_BLACK;
                    w->color       = RBT_RED;
                    rotate_right(tree, w);
                    w = x->parent->right;
                }
                /* Prípad 4: pravý syn brata je červený */
                w->color         = x->parent->color;
                x->parent->color = RBT_BLACK;
                w->right->color  = RBT_BLACK;
                rotate_left(tree, x->parent);
                x = tree->root; /* ukončenie slučky */
            }
        } else {
            /* Symetrický prípad: x je pravý syn */
            rbt_node_t *w = x->parent->left;

            if (IS_RED(tree, w)) {
                w->color         = RBT_BLACK;
                x->parent->color = RBT_RED;
                rotate_right(tree, x->parent);
                w = x->parent->left;
            }

            if (IS_BLACK(tree, w->right) && IS_BLACK(tree, w->left)) {
                w->color = RBT_RED;
                x = x->parent;
            } else {
                if (IS_BLACK(tree, w->left)) {
                    w->right->color = RBT_BLACK;
                    w->color        = RBT_RED;
                    rotate_left(tree, w);
                    w = x->parent->left;
                }
                w->color         = x->parent->color;
                x->parent->color = RBT_BLACK;
                w->left->color   = RBT_BLACK;
                rotate_right(tree, x->parent);
                x = tree->root;
            }
        }
    }
    x->color = RBT_BLACK;
}

/* ------------------------------------------------------------------ */
/*  Interné hľadanie uzla                                               */
/* ------------------------------------------------------------------ */

static rbt_node_t *node_search(const rbt_tree_t *tree, long long key)
{
    rbt_node_t *cur = tree->root;
    while (!IS_NIL(tree, cur)) {
        if (key == cur->key)
            return cur;
        cur = (key < cur->key) ? cur->left : cur->right;
    }
    return NULL;
}

/* Minimum v podstrome zakorenenom v node */
static rbt_node_t *subtree_min(const rbt_tree_t *tree, rbt_node_t *node)
{
    while (!IS_NIL(tree, node->left))
        node = node->left;
    return node;
}

/* Rekurzívne uvoľnenie uzlov (post-order) */
static void free_nodes(rbt_tree_t *tree, rbt_node_t *node)
{
    if (IS_NIL(tree, node))
        return;
    free_nodes(tree, node->left);
    free_nodes(tree, node->right);
    free(node);
}

/* Rekurzívny in-order výpis */
static void print_inorder(const rbt_tree_t *tree, const rbt_node_t *node,
                           long long depth)
{
    if (IS_NIL(tree, node))
        return;
    print_inorder(tree, node->right, depth + 1);
    for (long long i = 0; i < depth; i++) printf("    ");
    printf("%lld (%s)\n", node->key,
           node->color == RBT_RED ? "R" : "B");
    print_inorder(tree, node->left, depth + 1);
}

/* ------------------------------------------------------------------ */
/*  Verejné API                                                         */
/* ------------------------------------------------------------------ */

rbt_tree_t *rbt_create(void)
{
    rbt_tree_t *tree = (rbt_tree_t *)malloc(sizeof(rbt_tree_t));
    if (!tree)
        return NULL;

    /* Alokujeme zdieľaný NIL sentinel */
    tree->nil = (rbt_node_t *)malloc(sizeof(rbt_node_t));
    if (!tree->nil) {
        free(tree);
        return NULL;
    }

    tree->nil->color  = RBT_BLACK;
    tree->nil->left   = tree->nil;
    tree->nil->right  = tree->nil;
    tree->nil->parent = tree->nil;
    tree->nil->key    = 0; /* hodnota nie je dôležitá */

    tree->root = tree->nil;
    tree->size = 0;
    return tree;
}

void rbt_destroy(rbt_tree_t *tree)
{
    if (!tree)
        return;
    free_nodes(tree, tree->root);
    free(tree->nil);
    free(tree);
}

bool rbt_insert(rbt_tree_t *tree, long long key)
{
    assert(tree != NULL);

    /* Duplikát? */
    if (node_search(tree, key) != NULL)
        return false;

    rbt_node_t *z = (rbt_node_t *)malloc(sizeof(rbt_node_t));
    if (!z)
        return false;

    z->key    = key;
    z->color  = RBT_RED;
    z->left   = tree->nil;
    z->right  = tree->nil;
    z->parent = tree->nil;

    /* Štandardné BST vloženie */
    rbt_node_t *y = tree->nil;
    rbt_node_t *x = tree->root;

    while (!IS_NIL(tree, x)) {
        y = x;
        x = (key < x->key) ? x->left : x->right;
    }

    z->parent = y;

    if (IS_NIL(tree, y))
        tree->root = z;
    else if (key < y->key)
        y->left = z;
    else
        y->right = z;

    tree->size++;
    insert_fixup(tree, z);
    return true;
}

rbt_node_t *rbt_search(const rbt_tree_t *tree, long long key)
{
    assert(tree != NULL);
    return node_search(tree, key);
}

bool rbt_delete(rbt_tree_t *tree, long long key)
{
    assert(tree != NULL);

    rbt_node_t *z = node_search(tree, key);
    if (!z)
        return false;

    rbt_node_t *y = z;                   /* uzol, ktorý fyzicky odídeme */
    rbt_node_t *x;                        /* jeho náhradník              */
    rbt_color_t y_original_color = y->color;

    if (IS_NIL(tree, z->left)) {
        /* z nemá ľavý podstrom */
        x = z->right;
        transplant(tree, z, z->right);
    } else if (IS_NIL(tree, z->right)) {
        /* z nemá pravý podstrom */
        x = z->left;
        transplant(tree, z, z->left);
    } else {
        /* z má oboch synov → nájdeme následníka (min pravého podstromu) */
        y = subtree_min(tree, z->right);
        y_original_color = y->color;
        x = y->right;

        if (y->parent == z) {
            x->parent = y; /* zachovanie rodičovského odkazu pre NIL */
        } else {
            transplant(tree, y, y->right);
            y->right         = z->right;
            y->right->parent = y;
        }

        transplant(tree, z, y);
        y->left         = z->left;
        y->left->parent = y;
        y->color        = z->color;
    }

    free(z);
    tree->size--;

    if (y_original_color == RBT_BLACK)
        delete_fixup(tree, x);

    return true;
}

size_t rbt_size(const rbt_tree_t *tree)
{
    assert(tree != NULL);
    return tree->size;
}

bool rbt_is_empty(const rbt_tree_t *tree)
{
    assert(tree != NULL);
    return tree->size == 0;
}

rbt_node_t *rbt_min(const rbt_tree_t *tree)
{
    assert(tree != NULL);
    if (IS_NIL(tree, tree->root))
        return NULL;
    return subtree_min(tree, tree->root);
}

rbt_node_t *rbt_max(const rbt_tree_t *tree)
{
    assert(tree != NULL);
    if (IS_NIL(tree, tree->root))
        return NULL;
    rbt_node_t *cur = tree->root;
    while (!IS_NIL(tree, cur->right))
        cur = cur->right;
    return cur;
}

void rbt_print(const rbt_tree_t *tree)
{
    assert(tree != NULL);
    printf("RBT [size=%zu]:\n", tree->size);
    if (IS_NIL(tree, tree->root)) {
        printf("  (prázdny strom)\n");
        return;
    }
    print_inorder(tree, tree->root, 0);
}