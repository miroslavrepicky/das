#ifndef HASH_CHAINING_H
#define HASH_CHAINING_H

#ifdef __cplusplus
extern "C" {
#endif

int find_prime(int n) {
    if (n <= 1) return 0;
    for (int i = 2; i * i <= n; i++) {
        if (n % i == 0) return find_prime(n--);
    }
    return n;
}
#define HASH_SIZE 1000
#define HASH1(key) ((key) % HASH_SIZE)
#define PRIME find_prime(HASH_SIZE - 1)
#define HASH2(key) (PRIME - ((key) % PRIME))
#define DOUBLE_HASH(key, i) (HASH1(key) + (i) * HASH2(key)) % HASH_SIZE

typedef struct HashTable{
    int size;       // velkost tabulky
    int *table;     // pole pre ulozenie klucov
} HashTable;

HashTable* create_table(int size);
void insert(HashTable *ht, int key);
int search(HashTable *ht, int key);
void delete_key(HashTable *ht, int key);


#ifdef __cplusplus
}
#endif

#endif