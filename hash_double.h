#ifndef HASH_CHAINING_H
#define HASH_CHAINING_H

#ifdef __cplusplus
extern "C" {
#endif

#define ALPHA_THRESHOLD 0.5
#define LOWER_ALPHA 0.2

int find_prime(int n) {
    if (n <= 1) return 0;
    for (int i = 2; i * i <= n; i++) {
        if (n % i == 0) return find_prime(n--);
    }
    return n;
}

int hashd_1(int key, int size){
    return key % size;
}

int hashd_2(int key, int prime){
    return prime - (key % prime);
}

int double_hash(int key, int i, int size, int prime){
    return (hashd_1(key, size) + i * hashd_2(key, prime)) % size;
}

typedef struct HashTable{
    int size;       // velkost tabulky
    int prime;
    int used; // pocet obsadenych slotov, vratane vymazanych
    int count; // pocet skutocne vlozenych klucov (bez vymazanych)
    int *table;     // pole pre ulozenie klucov
} HashTable;

HashTable* create_table(int size);
void insert(HashTable *ht, int key);
int search(HashTable *ht, int key);
void delete_key(HashTable *ht, int key);
void free_tabled(HashTable *ht);
void rehash_double(HashTable *ht, double factor);



#ifdef __cplusplus
}
#endif

#endif