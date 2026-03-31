#include <stdio.h>
#include <stdlib.h>


typedef struct Node {
    int keys[2];          // 1 alebo 2 kľúče
    struct Node *children[4]; // 0, 2 alebo 3 deti (4 pre jednoduchšiu manipuláciu pri zlučovaní)
    int num_keys;         // počet kľúčov (1 alebo 2)
} Node;

typedef struct {
    Node *root;
} Tree23;

typedef struct {
    int promoted_key;   // kľúč ktorý ide nahor
    Node *right_child;  // nový pravý uzol po štiepeni
    int split_occurred; // 0 alebo 1
} InsertResult;

Node* new_node(void) {
    Node *n = (Node*)calloc(1, sizeof(Node));
    return n;
}

int is_leaf(Node *n) {
    return n->children[0] == NULL;
}

// Vráti 1 ak uzol má underflow (ostal prázdny po delete)
typedef struct {
    int underflow; // 1 ak treba riešiť rebalancing
} DeleteResult;

// Nájde najmenší kľúč v podstrome (in-order nasledovník)
int find_min(Node *node) {
    while (!is_leaf(node))
        node = node->children[0];
    return node->keys[0];
}


InsertResult split_node(Node *node, int key, Node *new_child) {
    InsertResult res;

    // Zoradíme 3 kľúče a 4 deti do pomocných polí
    int keys[3];
    Node *children[4];

    // Zaradíme nový kľúč na správne miesto
    if (key < node->keys[0]) {
        keys[0] = key;
        keys[1] = node->keys[0];
        keys[2] = node->keys[1];
        children[0] = new_child;
        children[1] = node->children[0];
        children[2] = node->children[1];
        children[3] = node->children[2];
    } else if (key < node->keys[1]) {
        keys[0] = node->keys[0];
        keys[1] = key;
        keys[2] = node->keys[1];
        children[0] = node->children[0];
        children[1] = node->children[1];
        children[2] = new_child;
        children[3] = node->children[2];
    } else {
        keys[0] = node->keys[0];
        keys[1] = node->keys[1];
        keys[2] = key;
        children[0] = node->children[0];
        children[1] = node->children[1];
        children[2] = node->children[2];
        children[3] = new_child;
    }

    // Stredný kľúč ide nahor
    res.promoted_key = keys[1];
    res.split_occurred = 1;

    // Ľavý uzol - prepoužijeme pôvodný
    node->keys[0] = keys[0];
    node->num_keys = 1;
    node->children[0] = children[0];
    node->children[1] = children[1];
    node->children[2] = NULL;

    // Pravý uzol - nový
    Node *right = new_node();
    right->keys[0] = keys[2];
    right->num_keys = 1;
    right->children[0] = children[2];
    right->children[1] = children[3];

    res.right_child = right;
    return res;
}

// new_child je pravý potomok promoted_key (NULL ak sme na liste)
InsertResult insert_into_node(Node *node, int key, Node *new_child) {
    InsertResult res = {0, NULL, 0};

    // --- 2-uzol → jednoducho pridáme, vznikne 3-uzol ---
    if (node->num_keys == 1) {
        if (key < node->keys[0]) {
            node->keys[1] = node->keys[0];
            node->keys[0] = key;
            node->children[2] = node->children[1];
            node->children[1] = new_child ? new_child : node->children[1];
        } else {
            node->keys[1] = key;
            node->children[2] = new_child ? new_child : node->children[2];
        }
        node->num_keys = 2;
        return res; // split_occurred = 0
    }

    // --- 3-uzol → musíme štiepif, vzniknú 2 uzly + promoted kľúč ---
    return split_node(node, key, new_child);
}

InsertResult insert_recursive(Node *node, int key) {
    InsertResult res = {0, NULL, 0};

    // === LIST ===
    if (is_leaf(node)) {
        return insert_into_node(node, key, NULL);
    }

    // === VNÚTORNÝ UZOL - vyber správne dieťa ===
    InsertResult child_res;

    if (key < node->keys[0]) {
        child_res = insert_recursive(node->children[0], key);
    } else if (node->num_keys == 1 || key < node->keys[1]) {
        child_res = insert_recursive(node->children[1], key);
    } else {
        child_res = insert_recursive(node->children[2], key);
    }

    // Dieťa sa neštiepilo - hotovo
    if (!child_res.split_occurred) return res;

    // Dieťa sa štiepilo - vložíme promoted_key do tohto uzla
    return insert_into_node(node, child_res.promoted_key, child_res.right_child);
}


void tree23_insert(Tree23 *tree, int key) {
    if (tree->root == NULL) {
        tree->root = new_node();
        tree->root->keys[0] = key;
        tree->root->num_keys = 1;
        return;
    }

    InsertResult res = insert_recursive(tree->root, key);

    // Koreň sa štiepil - vytvoríme nový koreň
    if (res.split_occurred) {
        Node *new_root = new_node();
        new_root->keys[0] = res.promoted_key;
        new_root->num_keys = 1;
        new_root->children[0] = tree->root;
        new_root->children[1] = res.right_child;
        tree->root = new_root;
    }
}

Node *tree23_search_node(Node *node, int key) {
    if (node == NULL) return NULL;
    if (is_leaf(node)) {
        for (int i = 0; i < node->num_keys; i++) {
            if (node->keys[i] == key) return node;
        }
        return NULL;
    }
    if(node->num_keys == 1){
        if(key == node->keys[0]) return node;
        if(key < node->keys[0]){
            return tree23_search_node(node->children[0], key);
        }
        return tree23_search_node(node->children[1], key);
    }
    else{
        if(key == node->keys[0]) return node;
        if(key == node->keys[1]) return node;
        if(key < node->keys[0]){
            return tree23_search_node(node->children[0], key);
        }
        else if(key < node->keys[1]){
            return tree23_search_node(node->children[1], key);
        }
        else{
            return tree23_search_node(node->children[2], key);
        }
    }
        
    

    return NULL;
}

Node* tree23_search(Tree23 *tree, int key) {
    // Implementácia vyhľadávania v 2-3 strome
    // Táto funkcia by mala prehľadávať strom a nájsť daný kľúč
    // ...
    if (tree->root == NULL) {
        printf("Key %d not found\n", key);
        return NULL;
    }
    
    Node *result = tree23_search_node(tree->root, key);
    if (result != NULL) {
        printf("Key %d found\n", key);
    } else {
        printf("Key %d not found\n", key);
    }

    return result;
}    

// Rotácia: požičaj kľúč od pravého súrodenca cez rodiča
void rotate_left(Node *parent, int idx) {
    Node *child   = parent->children[idx];
    Node *sibling = parent->children[idx + 1];

    // Kľúč rodiča ide do child
    child->keys[child->num_keys] = parent->keys[idx];
    child->num_keys++;

    // Najľavejší kľúč súrodenca ide do rodiča
    parent->keys[idx] = sibling->keys[0];

    // Presuň najľavejšie dieťa súrodenca do child
    child->children[child->num_keys] = sibling->children[0];

    // Posuň kľúče a deti súrodenca doľava
    for (int i = 0; i < sibling->num_keys - 1; i++) {
        sibling->keys[i]     = sibling->keys[i + 1];
        sibling->children[i] = sibling->children[i + 1];
    }
    sibling->children[sibling->num_keys - 1] = sibling->children[sibling->num_keys];
    sibling->num_keys--;
}

// Rotácia: požičaj kľúč od ľavého súrodenca cez rodiča
void rotate_right(Node *parent, int idx) {
    Node *child   = parent->children[idx];
    Node *sibling = parent->children[idx - 1];

    // Posunieme kľúče a deti child doprava
    for (int i = child->num_keys; i > 0; i--) {
        child->keys[i]         = child->keys[i - 1];
        child->children[i + 1] = child->children[i];
    }
    child->children[1] = child->children[0];

    // Kľúč rodiča ide do child
    child->keys[0] = parent->keys[idx - 1];
    child->num_keys++;

    // Napravý kľúč súrodenca ide do rodiča
    parent->keys[idx - 1] = sibling->keys[sibling->num_keys - 1];

    // Odober posledné dieťa súrodenca, presun do child
    child->children[0] = sibling->children[sibling->num_keys];
    sibling->children[sibling->num_keys] = NULL;
    sibling->num_keys--;
}

void merge(Node *parent, int idx) {
    Node *left  = parent->children[idx];
    Node *right = parent->children[idx + 1];

    // Ľavý uzol dostane: svoj kľúč (už má), kľúč rodiča, kľúč pravého
    left->keys[1] = parent->keys[idx];
    // right má vždy 1 kľúč pri merge (inak by sme rotovali)
    // Ale left už má 1 kľúč → 3-uzol má 2 kľúče
    // Pozor: right->keys[0] NEideme do left, left ostane s 2 kľúčmi:
    //   left->keys[0] = pôvodný kľúč left
    //   left->keys[1] = kľúč rodiča
    // A right sa stane stredným dieťaťom? Nie — right zanikne.
    // 
    // Správna logika merge pre 2-3 strom:
    // left (1 kľúč) + parent->keys[idx] + right (1 kľúč) → 3-uzol (2 kľúče)
    left->keys[1]     = parent->keys[idx];
    left->num_keys    = 2;
    left->children[2] = right->children[0];
    left->children[3] = right->children[1]; // POZOR: Node.children má size 3!

    // Posuň rodiča
    for (int i = idx; i < parent->num_keys - 1; i++) {
        parent->keys[i]         = parent->keys[i + 1];
        parent->children[i + 1] = parent->children[i + 2];
    }
    parent->children[parent->num_keys] = NULL;
    parent->num_keys--;
    free(right);
}

DeleteResult delete_recursive(Node *node, int key) {
    DeleteResult res = {0};

    // === LIST ===
    if (is_leaf(node)) {
        // Nájdi a vymaž kľúč
        if (node->num_keys == 2) {
            if (node->keys[0] == key) {
                node->keys[0] = node->keys[1];
            }
            // ak keys[1] == key, len znížime počet
            node->num_keys = 1;
            return res; // žiadny underflow
        } else {
            // 2-uzol → po delete ostane prázdny → underflow
            node->num_keys = 0;
            res.underflow = 1;
            return res;
        }
    }

    // === VNÚTORNÝ UZOL ===
    // Ak kľúč je tu, nahraď ho in-order nasledovníkom
    int child_idx;
    if (node->num_keys == 2 && key == node->keys[1]) {
        // Nahraď in-order nasledovníkom (min z pravého podstromu)
        node->keys[1] = find_min(node->children[2]);
        key = node->keys[1]; // maž nasledovníka z podstromu
        child_idx = 2;
    } else if (key == node->keys[0]) {
        node->keys[0] = find_min(node->children[1]);
        key = node->keys[0];
        child_idx = 1;
    } else if (key < node->keys[0]) {
        child_idx = 0;
    } else if (node->num_keys == 1 || key < node->keys[1]) {
        child_idx = 1;
    } else {
        child_idx = 2;
    }

    DeleteResult child_res = delete_recursive(node->children[child_idx], key);

    if (!child_res.underflow) return res;

    // === REBALANCING ===
    // Skús rotáciu od pravého súrodenca
    if (child_idx < node->num_keys &&
        node->children[child_idx + 1]->num_keys == 2) {
        rotate_left(node, child_idx);
        return res;
    }
    // Skús rotáciu od ľavého súrodenca
    if (child_idx > 0 &&
        node->children[child_idx - 1]->num_keys == 2) {
        rotate_right(node, child_idx);
        return res;
    }
    // Merge
    if (child_idx < node->num_keys) {
        merge(node, child_idx);
    } else {
        merge(node, child_idx - 1);
    }

    if (node->num_keys == 0)
        res.underflow = 1;

    return res;
}

void tree23_delete(Tree23 *tree, int key) {
    if (tree->root == NULL) {
        printf("Key %d not found\n", key);
        return;
    }

    DeleteResult res = delete_recursive(tree->root, key);

    // Ak koreň ostal prázdny po merge
    if (res.underflow) {
        Node *old_root = tree->root;
        tree->root = tree->root->children[0];
        free(old_root);
    }
}



int main(){

    Tree23 tree;
    tree.root = NULL;
    for (int i = 1; i <= 10000; i++) {
        tree23_insert(&tree, i);
        if(i % 1000 == 0){
            printf("Inserted %d\n", i);
        }
    }
    tree23_search(&tree, 5000);
    tree23_delete(&tree, 5000);
    tree23_search(&tree, 5000);


    return 0;
}