#include <stdio.h>
#include <stdlib.h>

#include "hash_double.h"

HashTableDouble* create_table(long long size){
    HashTableDouble *ht = (HashTableDouble*)malloc(sizeof(HashTableDouble));
    ht->size = size;
    ht->used = 0;
    ht->count = 0;
    ht->prime = find_prime(size - 1);
    ht->table = (int*)malloc(size * sizeof(int));
    for(long long i = 0; i < size; i++){
        ht->table[i] = -1; // -1 znamená prázdný slot
    }
    return ht;
}

void insert_double(HashTableDouble *ht, long long key){
    long long i = 0;
    long long index;
    while(i < ht->size){
        index = DOUBLE_HASH(key, i);
        index = double_hash(key, i, ht->size, ht->prime);
        if(ht->table[index] == -1){ // našli jsme prázdný slot
            ht->table[index] = key;
            ht->used++;
            ht->count++;
            if((double)ht->used / ht->size > ALPHA_THRESHOLD){
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
        index = DOUBLE_HASH(key, i);
        if(ht->table[index] == -1){ // prázdný slot znamená, že klíč není v tabulce
            return -1; // klíč nenalezen
        }
        if(ht->table[index] == key){ // klíč nalezen
            return index;
        }
        i++;
    }
    return -1; // klíč nenalezen po prohledání celé tabulky
}

void delete_key_double(HashTableDouble *ht, long long key){
    long long i = 0;
    long long index;
    while(i < ht->size){
        index = double_hash(key, i, ht->size, ht->prime);
        if(ht->table[index] == -1){ // prázdný slot znamená, že klíč není v tabulce
            return; // klíč nenalezen
        }
        if(ht->table[index] == key){ // klíč nalezen
            ht->table[index] = -2; // -2 znamená smazaný slot
            ht->count--;
            if((double)ht->count / ht->size < LOWER_ALPHA){
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
    long long new_size = find_prime((int)(old_size * factor));
    HashTableDouble *new_ht = create_table_double(new_size);
    new_ht->size = new_size;
    new_ht->prime = find_prime(new_size - 1);
    new_ht->used = 0;
    new_ht->count = 0;
    for(long long i = 0; i < old_size; i++){
        if(old_table[i] >= 0){ // iba skutocne kluce, nie prazdne alebo vymazane sloty
            insert_double(new_ht, old_table[i]);
        }
    }
    free(old_table);
    free(ht);
    *ht = *new_ht;
}


void free_table_double(HashTableDouble *ht){
    free(ht->table);
    free(ht);
}



long long main() {
    HashTableDouble *ht = create_table_double(10);

    for(long long i = 1; i <= 1000; i++){
        insert_double(ht, i);
    }

    printf("Search for 1: %lld\n", search_double(ht, 1)); // should find
    printf("Search for 110: %lld\n", search_double(ht, 110)); // should find
    printf("Search for 321: %lld\n", search_double(ht, 321)); // should find
    printf("Search for 831: %lld\n", search_double(ht, 831)); // should not find

    delete_key_double(ht, 11);
    printf("Search for 11 after deletion: %lld\n", search_double(ht, 11)); // should not find

    free(ht->table);
    free(ht);
    return 0;
}


