#ifndef HASH_CHAINING_H
#define HASH_CHAINING_H

#ifdef __cplusplus
extern "C" {
#endif

#define ALPHA_THRESHOLD 0.75
#define LOWER_ALPHA 0.2

typedef struct Node {
    long long key;
    struct Node *next;
} Node;

typedef struct HashTable {
    long long size;
    long long count; // počet ulozenych klucov
    Node **table;
} HashTable;

// funkcie
HashTable* create_table(long long size);
void insert(HashTable *ht, long long key);
Node* search(HashTable *ht, long long key);
void delete_key(HashTable *ht, long long key);
void free_table(HashTable *ht);
void rehash(HashTable *ht, double factor);

#ifdef __cplusplus
}
#endif

#endif