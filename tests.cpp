extern "C" {
    double run_test(int n);
}

#include <iostream>

//TODO testovanie vsetkych 4 algoritmov a ich casovej narocnosti pri roznej velkosti pre rozne operacie konkretne insert, search a delete.
//TODO porovnat casovu narocnost a vysledky pre kazdy algoritmus a napisat o tom kratku analyzu v readme.
//TODO vizualizacia vysledkov pomocou grafov, casova narocnost vs velkost dat, pre kazdy algoritmus a operaciu.

int main() {
    std::cout << run_test(1000) << std::endl;
}