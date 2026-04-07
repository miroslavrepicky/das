#ifndef HASH_DOUBLE_H
#define HASH_DOUBLE_H

#ifdef __cplusplus
extern "C" {
#endif

#define ALPHA_THRESHOLD_DOUBLE 0.5
#define LOWER_ALPHA_DOUBLE 0.2



typedef struct HashTableDouble {
    long long size;       // velkost tabulky
    long long prime;
    long long used; // pocet obsadenych slotov, vratane vymazanych
    long long count; // pocet skutocne vlozenych klucov (bez vymazanych)
    long long *table;     // pole pre ulozenie klucov
} HashTableDouble;

HashTableDouble* create_table_double(long long size);
void insert_double(HashTableDouble *ht, long long key);
long long search_double(HashTableDouble *ht, long long key);
void delete_key_double(HashTableDouble *ht, long long key);
void free_table_double(HashTableDouble *ht);
void rehash_double(HashTableDouble *ht, double factor);
long long find_prime(long long n);
long long hashd_1(long long key, long long size);
long long hashd_2(long long key, long long prime);
long long double_hash(long long key, long long i, long long size, long long prime);




#ifdef __cplusplus
}
#endif

#endif