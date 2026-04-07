#include <stdio.h>
#include <stdlib.h>

#include "hash_chaining.h"



HashTable* create_table(long long size){
    HashTable *ht = (HashTable*)malloc(sizeof(HashTable));
    ht->size = size;
    ht->count = 0;
    ht->table = (Node**)malloc(size * sizeof(Node*));
    for(long long i = 0; i < size; i++){
        ht->table[i] = NULL;
    }
    return ht;
}

void insert(HashTable *ht, long long key){
    long long index = ((key % ht->size) + ht->size) % ht->size; // jednoduchá hashovacia funkcia
    Node *new_node = (Node*)malloc(sizeof(Node));
    new_node->key = key;
    new_node->next = ht->table[index]; // vkládáme na zaciatok reťazca
    ht->table[index] = new_node;
    ht->count++;
    if ((double)ht->count / ht->size > ALPHA_THRESHOLD)
    {
        rehash(ht, 2.0);
    }
    
}

Node* search(HashTable *ht, long long key){
    long long index = ((key % ht->size) + ht->size) % ht->size;
    Node *current = ht->table[index];
    while(current != NULL){
        if(current->key == key){
            return current; // nalezen klíč
        }
        current = current->next;
    }
    return NULL; // klíč nenalezen
}

void delete_key(HashTable *ht, long long key){
    long long index = ((key % ht->size) + ht->size) % ht->size;
    Node *current = ht->table[index];
    Node *prev = NULL;
    while(current != NULL){
        if(current->key == key){
            if(prev == NULL){
                ht->table[index] = current->next; // mazanie prvného uzla v reťazci
            } else {
                prev->next = current->next; // mazanie uzla uprostred nebo na konci reťazca
            }
            free(current);
            ht->count--;
            if((double)ht->count / ht->size < LOWER_ALPHA){
                rehash(ht, 0.5);
            }
            return;
        }
        prev = current;
        current = current->next;
    }
}

void rehash(HashTable *ht, double factor){
    long long old_size = ht->size;
    Node **old_table = ht->table;

    long long new_size = (int)(old_size * factor);
    HashTable *new_ht = create_table(new_size);

    // Prenesieme všetky kľúče do novej tabuľky
    for (long long i = 0; i < old_size; i++) {
        Node *current = old_table[i];
        while (current != NULL) {
            insert(new_ht, current->key);
            current = current->next;
        }
    }
    // Uvoľníme uzly v starej tabuľke
    for (long long i = 0; i < old_size; i++) {
        Node *current = old_table[i];
        while (current != NULL) {
            Node *tmp = current;
            current = current->next;
            free(tmp);           // uvoľníme každý uzol
        }
    }

    free(old_table);             // uvoľníme pole bucketov

    // Prenesieme obsah new_ht do pôvodnej štruktúry
    ht->table = new_ht->table;
    ht->size  = new_ht->size;
    ht->count = new_ht->count;

    free(new_ht);                // uvoľníme len wrapper štruktúru, NIE table
}

void free_table(HashTable *ht) {
    if (ht == NULL) return;

    for (long long i = 0; i < ht->size; i++) {
        Node *current = ht->table[i];

        while (current != NULL) {
            Node *tmp = current;
            current = current->next;
            free(tmp);
        }
    }

    free(ht->table);
    free(ht);
}