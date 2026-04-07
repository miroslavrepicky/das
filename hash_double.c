#include <stdio.h>
#include <stdlib.h>

#include "hash_double.h"

HashTableDouble* create_table_double(long long size){
    HashTableDouble *ht = (HashTableDouble*)malloc(sizeof(HashTableDouble));
    ht->size = size;
    ht->used = 0;
    ht->count = 0;
    ht->prime = find_prime(size - 1);
    ht->table = (long long*)malloc(size * sizeof(long long));
    for(long long i = 0; i < size; i++){
        ht->table[i] = -1; // -1 znamená prázdny slot
    }
    return ht;
}

void insert_double(HashTableDouble *ht, long long key){
    long long i = 0;
    long long index;
    while(i < ht->size){
        index = double_hash(key, i, ht->size, ht->prime);
        if(ht->table[index] == -1){ // našli sme prázdny slot
            ht->table[index] = key;
            ht->used++;
            ht->count++;
            if((double)ht->used / ht->size > ALPHA_THRESHOLD_DOUBLE){
                rehash_double(ht, 2.0);
            }
            return;
        }
        i++;
    }
    printf("Hash table is full, cannot insert key %lld\n", key);
}

long long search_double(HashTableDouble *ht, long long key){
    long long i = 0;
    long long index;
    while(i < ht->size){
        index = double_hash(key, i, ht->size, ht->prime);
        
        if(ht->table[index] == -1){ // prázdny slot znamená, že kluc nie je v tabulke
            return -1; // klíč nenalezen
        }
        if(ht->table[index] == key){ // kluc nenajdeny
            return index;
        }
        i++;
    }
    return -1; // kluc nenajdeny po prehladani celej tabulky
}

void delete_key_double(HashTableDouble *ht, long long key){
    long long i = 0;
    long long index;
    while(i < ht->size){
        index = double_hash(key, i, ht->size, ht->prime);
        if(ht->table[index] == -1){ // prázdny slot znamená, že kluc nie je v tabulke
            return; // kluc nenajdeny
        }
        if(ht->table[index] == key){ // kluc najdeny
            ht->table[index] = -2; // -2 znamená zmazaný slot
            ht->count--;
            if((double)ht->count / ht->size < LOWER_ALPHA_DOUBLE){
                rehash_double(ht, 0.5);
            }
            return;
        }
        i++;
    }
}

void rehash_double(HashTableDouble *ht, double factor){
    long long old_size = ht->size;
    long long *old_table = ht->table;
    long long new_size = find_prime((long long)(old_size * factor));

    // Vytvoríme dočasnú novú tabuľku
    HashTableDouble *new_ht = create_table_double(new_size);

    for(long long i = 0; i < old_size; i++){
        if(old_table[i] >= 0){
            insert_double(new_ht, old_table[i]);
        }
    }

    // Uvoľníme starý interný buffer
    free(old_table);

    // Skopírujeme obsah new_ht do ht)
    ht->size  = new_ht->size;
    ht->used  = new_ht->used;
    ht->count = new_ht->count;
    ht->prime = new_ht->prime;
    ht->table = new_ht->table;

    // Uvoľníme iba wrapper štruktúru new_ht, nie jej table (tú teraz vlastní ht)
    free(new_ht);
}

long long find_prime(long long n) {
    if (n <= 1) return 2;  // minimálna bezpečná prvočíslo
    if (n == 2) return 2;
    if (n % 2 == 0) return find_prime(n - 1); // preskočiť párne
    for (long long i = 2; i * i <= n; i++) {
        if (n % i == 0) return find_prime(n - 1);
    }
    return n;
}

long long hashd_1(long long key, long long size){
    return ((key % size) + size) % size;  // vždy nezáporné
}

long long hashd_2(long long key, long long prime){
    return prime - (((key % prime) + prime) % prime);
}

long long double_hash(long long key, long long i, long long size, long long prime){
    return (hashd_1(key, size) + i * hashd_2(key, prime)) % size;
}


void free_table_double(HashTableDouble *ht){
    free(ht->table);
    free(ht);
}



// int main() {
//     HashTableDouble *ht = create_table_double(10);

//     for(long long i = 1; i <= 1000; i++){
//         insert_double(ht, i);
//         printf("Inserted %lld, load factor: %.2f\n", i, (double)ht->used / ht->size);
//     }

//     for(long long i = 1000; i <= 10000; i++){
//         if(search_double(ht, i) == -1){
//             printf("Error: key %lld not found!\n", i);
//         }
//     }

//     printf("Search for 1: %lld\n", search_double(ht, 1)); // should find
//     printf("Search for 110: %lld\n", search_double(ht, 110)); // should find
//     printf("Search for 321: %lld\n", search_double(ht, 321)); // should find
//     printf("Search for 831: %lld\n", search_double(ht, 831)); // should not find

//     delete_key_double(ht, 11);
//     printf("Search for 11 after deletion: %lld\n", search_double(ht, 11)); // should not find

//     free(ht->table);
//     free(ht);
//     return 0;
// }


