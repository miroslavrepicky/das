#include <stdio.h>
#include <stdlib.h>

#include "hash_double.h"

HashTable* create_table(int size){
    HashTable *ht = (HashTable*)malloc(sizeof(HashTable));
    ht->size = size;
    ht->table = (int*)malloc(size * sizeof(int));
    for(int i = 0; i < size; i++){
        ht->table[i] = -1; // -1 znamená prázdný slot
    }
    return ht;
}

void insert(HashTable *ht, int key){
    int i = 0;
    int index;
    while(i < ht->size){
        index = DOUBLE_HASH(key, i);
        if(ht->table[index] == -1){ // našli jsme prázdný slot
            ht->table[index] = key;
            return;
        }
        i++;
    }
    printf("Hash table is full, cannot insert key %d\n", key);
}

int search(HashTable *ht, int key){
    int i = 0;
    int index;
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

void delete_key(HashTable *ht, int key){
    int i = 0;
    int index;
    while(i < ht->size){
        index = DOUBLE_HASH(key, i);
        if(ht->table[index] == -1){ // prázdný slot znamená, že klíč není v tabulce
            return; // klíč nenalezen
        }
        if(ht->table[index] == key){ // klíč nalezen
            ht->table[index] = -2; // -2 znamená smazaný slot
            return;
        }
        i++;
    }
}

int main() {
    HashTable *ht = create_table(HASH_SIZE);

    for(int i = 1; i <= 1000; i++){
        insert(ht, i);
    }

    printf("Search for 1: %d\n", search(ht, 1)); // should find
    printf("Search for 110: %d\n", search(ht, 110)); // should find
    printf("Search for 321: %d\n", search(ht, 321)); // should find
    printf("Search for 831: %d\n", search(ht, 831)); // should not find

    delete_key(ht, 11);
    printf("Search for 11 after deletion: %d\n", search(ht, 11)); // should not find

    free(ht->table);
    free(ht);
    return 0;
}


