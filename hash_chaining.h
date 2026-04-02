#ifndef HASH_CHAINING_H
#define HASH_CHAINING_H

#ifdef __cplusplus
extern "C" {
#endif

#define ALPHA_THRESHOLD 0.75
#define LOWER_ALPHA 0.2

typedef struct Node {
    int key;
    struct Node *next;
} Node;

typedef struct HashTable {
    int size;
    int count; // počet ulozenych klucov
    Node **table;
} HashTable;

// funkcie
HashTable* create_table(int size);
void insert(HashTable *ht, int key);
Node* search(HashTable *ht, int key);
void delete_key(HashTable *ht, int key);
void free_table(HashTable *ht);
void rehash(HashTable *ht, double factor);

#ifdef __cplusplus
}
#endif

#endif