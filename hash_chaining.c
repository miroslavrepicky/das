#include <stdio.h>
#include <stdlib.h>

#include "hash_chaining.h"



HashTable* create_table(int size){
    HashTable *ht = (HashTable*)malloc(sizeof(HashTable));
    ht->size = size;
    ht->table = (Node**)malloc(size * sizeof(Node*));
    for(int i = 0; i < size; i++){
        ht->table[i] = NULL;
    }
    return ht;
}

void insert(HashTable *ht, int key){
    int index = key % ht->size; // jednoduchá hashovací funkce
    Node *new_node = (Node*)malloc(sizeof(Node));
    new_node->key = key;
    new_node->next = ht->table[index]; // vkládáme na začátek řetězce
    ht->table[index] = new_node;
}

Node* search(HashTable *ht, int key){
    int index = key % ht->size;
    Node *current = ht->table[index];
    while(current != NULL){
        if(current->key == key){
            return current; // nalezen klíč
        }
        current = current->next;
    }
    return NULL; // klíč nenalezen
}

void delete_key(HashTable *ht, int key){
    int index = key % ht->size;
    Node *current = ht->table[index];
    Node *prev = NULL;
    while(current != NULL){
        if(current->key == key){
            if(prev == NULL){
                ht->table[index] = current->next; // mazání prvního uzlu v řetězci
            } else {
                prev->next = current->next; // mazání uzlu uprostřed nebo na konci řetězce
            }
            free(current);
            return;
        }
        prev = current;
        current = current->next;
    }
}




int main(){


    return 0;
}
