#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
    int key;            // kľúč uzla
    int color;          // 0 = červený, 1 = čierný
    struct Node *left;  // ľavé dítě
    struct Node *right; // pravé dítě
    struct Node *parent;// rodičovský uzel
} Node;

typedef struct {
    Node *root;
} RedBlackTree;

Node* new_node(int key) {
    Node *node = (Node*)malloc(sizeof(Node));
    node->key = key;
    node->color = 0; // nový uzel je vždy červený
    node->left = NULL;
    node->right = NULL;
    node->parent = NULL;
    return node;
}

void rotate_left(RedBlackTree *tree, Node *x) {
    Node *y = x->right; // y je pravý syn x
    x->right = y->left; // levý syn y se stává pravým synem x
    if (y->left != NULL) {
        y->left->parent = x;
    }
    y->parent = x->parent; // rodič y se stává rodičem x
    if (x->parent == NULL) {
        tree->root = y; // pokud je x kořen, tak se stává y kořenem
    } else if (x == x->parent->left) {
        x->parent->left = y; // pokud je x levým synem, tak se stává y levým synem
    } else {
        x->parent->right = y; // pokud je x pravým synem, tak se stává y pravým synem
    }
    y->left = x; // x se stává levým synem y
    x->parent = y; // rodič x se stává y
}

void rotate_right(RedBlackTree *tree, Node *y) {
    Node *x = y->left; // x je levý syn y
    y->left = x->right; // pravý syn x se stává levým synem y
    if (x->right != NULL) {
        x->right->parent = y;
    }
    x->parent = y->parent; // rodič x se stává rodičem y
    if (y->parent == NULL) {
        tree->root = x; // pokud je y kořen, tak se stává x kořenem
    } else if (y == y->parent->right) {
        y->parent->right = x; // pokud je y pravým synem, tak se stává x pravým synem
    } else {
        y->parent->left = x; // pokud je y levým synem, tak se stává x levým synem
    }
    x->right = y; // y se stává pravým synem x
    y->parent = x; // rodič y se stává x
}

void fix_insert(RedBlackTree *tree, Node *z) {
    while (z->parent != NULL && z->parent->color == 0) {
        if (z->parent == z->parent->parent->left) {
            // rodič je ľavý syn starého rodiča
            Node *uncle = z->parent->parent->right;
            if (uncle != NULL && uncle->color == 0) {
                // prípad 1: strýko je červený → prefarbenie
                z->parent->color = 1;
                uncle->color = 1;
                z->parent->parent->color = 0;
                z = z->parent->parent;
            } else {
                if (z == z->parent->right) {
                    // prípad 2: z je pravý syn → rotácia doľava
                    z = z->parent;
                    rotate_left(tree, z);
                }
                // prípad 3: z je ľavý syn → rotácia doprava
                z->parent->color = 1;
                z->parent->parent->color = 0;
                rotate_right(tree, z->parent->parent);
            }
        } else {
            // zrkadlový prípad: rodič je pravý syn starého rodiča
            Node *uncle = z->parent->parent->left;
            if (uncle != NULL && uncle->color == 0) {
                z->parent->color = 1;
                uncle->color = 1;
                z->parent->parent->color = 0;
                z = z->parent->parent;
            } else {
                if (z == z->parent->left) {
                    z = z->parent;
                    rotate_right(tree, z);
                }
                z->parent->color = 1;
                z->parent->parent->color = 0;
                rotate_left(tree, z->parent->parent);
            }
        }
    }
    tree->root->color = 1; // koreň je vždy čierny
}

void insert(RedBlackTree *tree, int key) {
    Node *z = new_node(key);
    Node *parent = NULL;
    Node *curr = tree->root;

    // štandardné BST vloženie
    while (curr != NULL) {
        parent = curr;
        if (z->key < curr->key)
            curr = curr->left;
        else
            curr = curr->right;
    }

    z->parent = parent;
    if (parent == NULL) {
        tree->root = z;         // strom bol prázdny
    } else if (z->key < parent->key) {
        parent->left = z;
    } else {
        parent->right = z;
    }

    fix_insert(tree, z);
}

Node* search(RedBlackTree *tree, int key) {
    Node *curr = tree->root;
    while (curr != NULL) {
        if (key == curr->key)
            return curr;
        else if (key < curr->key)
            curr = curr->left;
        else
            curr = curr->right;
    }
    return NULL; // nenašlo sa
}

void transplant(RedBlackTree *tree, Node *u, Node *v) {
    if (u->parent == NULL)
        tree->root = v;
    else if (u == u->parent->left)
        u->parent->left = v;
    else
        u->parent->right = v;
    if (v != NULL)
        v->parent = u->parent;
}

void fix_delete(RedBlackTree *tree, Node *x, Node *x_parent) {
    while (x != tree->root && (x == NULL || x->color == 1)) {
        if (x == x_parent->left) {
            Node *w = x_parent->right; // súrodenec
            if (w != NULL && w->color == 0) {
                // prípad 1: súrodenec je červený
                w->color = 1;
                x_parent->color = 0;
                rotate_left(tree, x_parent);
                w = x_parent->right;
            }
            if ((w == NULL) ||
                ((w->left == NULL  || w->left->color == 1) &&
                 (w->right == NULL || w->right->color == 1))) {
                // prípad 2: obaja synovia súrodenca sú čierni
                if (w != NULL) w->color = 0;
                x = x_parent;
                x_parent = x->parent;
            } else {
                if (w->right == NULL || w->right->color == 1) {
                    // prípad 3: pravý syn súrodenca je čierny
                    if (w->left != NULL) w->left->color = 1;
                    w->color = 0;
                    rotate_right(tree, w);
                    w = x_parent->right;
                }
                // prípad 4: pravý syn súrodenca je červený
                w->color = x_parent->color;
                x_parent->color = 1;
                if (w->right != NULL) w->right->color = 1;
                rotate_left(tree, x_parent);
                x = tree->root; // koniec
            }
        } else {
            // zrkadlový prípad
            Node *w = x_parent->left;
            if (w != NULL && w->color == 0) {
                w->color = 1;
                x_parent->color = 0;
                rotate_right(tree, x_parent);
                w = x_parent->left;
            }
            if ((w == NULL) ||
                ((w->right == NULL || w->right->color == 1) &&
                 (w->left == NULL  || w->left->color == 1))) {
                if (w != NULL) w->color = 0;
                x = x_parent;
                x_parent = x->parent;
            } else {
                if (w->left == NULL || w->left->color == 1) {
                    if (w->right != NULL) w->right->color = 1;
                    w->color = 0;
                    rotate_left(tree, w);
                    w = x_parent->left;
                }
                w->color = x_parent->color;
                x_parent->color = 1;
                if (w->left != NULL) w->left->color = 1;
                rotate_right(tree, x_parent);
                x = tree->root;
            }
        }
    }
    if (x != NULL) x->color = 1;
}

void delete(RedBlackTree *tree, int key) {
    Node *z = search(tree, key);
    if (z == NULL) return; // kľúč neexistuje

    Node *y = z;
    Node *x = NULL;
    Node *x_parent = NULL;
    int y_original_color = y->color;

    if (z->left == NULL) {
        // prípad A: ľavý syn chýba
        x = z->right;
        x_parent = z->parent;
        transplant(tree, z, z->right);
    } else if (z->right == NULL) {
        // prípad B: pravý syn chýba
        x = z->left;
        x_parent = z->parent;
        transplant(tree, z, z->left);
    } else {
        // prípad C: uzol má oboch synov → nájdi následníka (minimum pravého podstromu)
        y = z->right;
        while (y->left != NULL)
            y = y->left;
        y_original_color = y->color;
        x = y->right;

        if (y->parent == z) {
            x_parent = y;
        } else {
            x_parent = y->parent;
            transplant(tree, y, y->right);
            y->right = z->right;
            y->right->parent = y;
        }
        transplant(tree, z, y);
        y->left = z->left;
        y->left->parent = y;
        y->color = z->color;
    }

    free(z);

    // oprava iba ak sme odstránili čierny uzol
    if (y_original_color == 1)
        fix_delete(tree, x, x_parent);
}


int main() {
    RedBlackTree tree;
    tree.root = NULL;

    // Vložení klíčů do stromu
    for (int i = 1; i <= 100000; i++) {
        insert(&tree, i);
    }
    for(int i = 1; i <= 100000; i++) {
        delete(&tree, i);
    }
    printf("Vložení a mazání 100000 klíčů dokončeno.\n");

    // Můžete přidat další testy pro vyhledávání a mazání

    return 0;
}


