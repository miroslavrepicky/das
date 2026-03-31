#ifndef HASH_CHAINING_H
#define HASH_CHAINING_H

#ifdef __cplusplus
extern "C" {
#endif

#define HASH_SIZE 10

typedef struct Node {
    int key;
    struct Node *next;
} Node;

typedef struct HashTable {
    int size;
    Node **table;
} HashTable;

// funkcie
HashTable* create_table(int size);
void insert(HashTable *ht, int key);
Node* search(HashTable *ht, int key);
void delete_key(HashTable *ht, int key);

#ifdef __cplusplus
}
#endif

#endif