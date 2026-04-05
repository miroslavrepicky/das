#ifndef HASH_CHAINING_H
#define HASH_CHAINING_H

#ifdef __cplusplus
extern "C" {
#endif

#define ALPHA_THRESHOLD 0.5
#define LOWER_ALPHA 0.2

long long find_prime(long long n) {
    if (n <= 1) return 0;
    for (long long i = 2; i * i <= n; i++) {
        if (n % i == 0) return find_prime(n--);
    }
    return n;
}

long long hashd_1(long long key, long long size){
    return key % size;
}

long long hashd_2(long long key, long long prime){
    return prime - (key % prime);
}

long long double_hash(long long key, long long i, long long size, long long prime){
    return (hashd_1(key, size) + i * hashd_2(key, prime)) % size;
}

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



#ifdef __cplusplus
}
#endif

#endif