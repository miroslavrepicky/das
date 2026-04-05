#include <iostream>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <ctime>


extern "C" {
    #include "hash_chaining.h"
    #include "two_three_tree.h"
}



//TODO testovanie vsetkych 4 algoritmov a ich casovej narocnosti pri roznej velkosti pre rozne operacie konkretne insert, search a delete.
//TODO porovnat casovu narocnost a vysledky pre kazdy algoritmus a napisat o tom kratku analyzu v readme.
//TODO vizualizacia vysledkov pomocou grafov, casova narocnost vs velkost dat, pre kazdy algoritmus a operaciu.





// pomocná funkcia na generovanie dát
std::vector<int> generate_data(long long n) {
    std::vector<int> data;
    data.reserve(n);

    for (long long i = 0; i < n; i++) {
        data.push_back(rand());
    }

    return data;
}

void run_test(long long n, double alpha) {
    long long m = (int)(n / alpha); // veľkosť tabuľky

    HashTable *ht = create_table(m);
    std::vector<int> data = generate_data(n);

    // 1. naplň tabuľku
    for (long long i = 0; i < n; i++) {
        insert(ht, data[i]);
    }

    // 2. meraj insert ďalších prvkov
    long long extra = 100000;

    auto start_insert = std::chrono::high_resolution_clock::now();

    for (long long i = 0; i < extra; i++) {
        insert(ht, rand());
    }

    auto end_insert = std::chrono::high_resolution_clock::now();
    double insert_time = std::chrono::duration<double>(end_insert - start_insert).count();

    // priemerný čas na 1 insert
    double avg_insert = insert_time / extra;

    // 🔹 SEARCH (existujúce prvky)
    auto start_search = std::chrono::high_resolution_clock::now();

    for (long long i = 0; i < n; i++) {
        search(ht, data[i]);
    }

    auto end_search = std::chrono::high_resolution_clock::now();
    double search_time = std::chrono::duration<double>(end_search - start_search).count();

    // 🔹 DELETE
    auto start_delete = std::chrono::high_resolution_clock::now();

    for (long long i = 0; i < n; i++) {
        delete_key(ht, data[i]);
    }

    auto end_delete = std::chrono::high_resolution_clock::now();
    double delete_time = std::chrono::duration<double>(end_delete - start_delete).count();

    // 🔹 výstup (CSV formát)
    std::cout << n << "," << alpha << ","
              << insert_time << ","
              << avg_insert << ","
              << search_time << ","
              << delete_time << std::endl;
    free_table(ht);
}

long long main() {
    srand(time(NULL));

    std::vector<int> sizes = {1000, 5000, 10000, 100000, 500000, 1000000, 5000000, 10000000, 50000000};
    std::vector<double> alphas = {0.5};

    std::cout << "n,alpha,insert,search,delete" << std::endl;

    for (long long n : sizes) {
        for (double alpha : alphas) {
            run_test(n, alpha);
        }
    }

    return 0;
}