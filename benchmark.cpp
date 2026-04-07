/**
 * benchmark.cpp — Testovaci harness pre 4 datove struktury
 *
 * Kompilovanie:
 *   g++ -O2 -std=c++17 -o benchmark benchmark.cpp
 *
 * Vystup: results/summary.csv + results/SCENAR.csv
 */

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iostream>
#include <numeric>
#include <random>
#include <string>
#include <vector>
//#include <filesystem>
#include <direct.h>

extern "C" {
#include "hash_chaining.h"
#include "hash_double.h"
#include "rbt.h"
#include "two_three_tree.h"
}

/* ================================================================ */
/*  Nastavenia                                                        */
/* ================================================================ */

static const std::vector<long long> N_VALUES = {10000LL, 25000LL, 50000LL, 100000LL, 250000LL ,500000LL, 750000LL, 1000000LL, 2500000LL, 5000000LL};

/* Pocet opakovani pre povodne scenare (bulk meranie) */
static constexpr int       REPEATS   = 4;

/*
 * Pocet opakovani pre scenare cistej zlozitosti (SC_*).
 * Kazda iteracia zmera blok K operacii a vydeli K.
 */
static constexpr int       REPEATS_SC = 50;

/*
 * Velkost bloku pre SC scenare.
 * Meriame K operacii naraz (struktura ide z n na n+K),
 * co je pri velkom n zanedbatelne (~0.1% zmena).
 * K musi byt dostatocne velke aby cas bloku presiel rozlisenie hodin (~100ns).
 * K=1000 je rozumny kompromis.
 */
static constexpr long long SC_BLOCK   = 1000;

static constexpr long long INIT_SIZE  = 16;

/* ================================================================ */
/*  Generatory klucov                                               */
/* ================================================================ */

std::vector<long long> sequential_keys(long long n) {
    std::vector<long long> v(n);
    std::iota(v.begin(), v.end(), 1LL);
    return v;
}

std::vector<long long> random_keys(long long n, unsigned seed = 42) {
    std::mt19937_64 rng(seed);
    std::uniform_int_distribution<long long> dist(1LL, n * 10LL);
    std::vector<long long> v;
    v.reserve(n * 2);
    while ((long long)v.size() < n * 2) v.push_back(dist(rng));
    std::sort(v.begin(), v.end());
    v.erase(std::unique(v.begin(), v.end()), v.end());
    if ((long long)v.size() < n) {
        // fallback: pad with sequential negatives (guaranteed unique)
        for (long long i = 1; (long long)v.size() < n; ++i)
            v.push_back(-i);
    }
    v.resize(n);
    std::shuffle(v.begin(), v.end(), std::mt19937_64(seed + 1));
    return v;
}

std::vector<long long> collision_keys(long long n, long long stride) {
    std::vector<long long> v(n);
    for (long long i = 0; i < n; ++i) v[i] = (i + 1) * stride;
    return v;
}

/* ================================================================ */
/*  Casovac — povodny (bulk)                                          */
/* ================================================================ */

using Clock = std::chrono::high_resolution_clock;
using Ns    = std::chrono::nanoseconds;

struct Stats { double mean_ns, stddev_ns; };

Stats bench(long long n_ops,
            std::function<void()> setup,
            std::function<void()> fn) {
    std::vector<double> s;
    s.reserve(REPEATS);
    for (int r = 0; r < REPEATS; ++r) {
        setup();
        auto t0 = Clock::now();
        fn();
        auto t1 = Clock::now();
        s.push_back((double)std::chrono::duration_cast<Ns>(t1 - t0).count()
                    / (double)n_ops);
    }
    double mean = std::accumulate(s.begin(), s.end(), 0.0) / REPEATS;
    double var  = 0.0;
    for (double x : s) var += (x - mean) * (x - mean);
    return {mean, std::sqrt(var / REPEATS)};
}

/* ================================================================ */
/*  Casovac — blokovy (pre cistu zlozitost)                           */
/*                                                                    */
/*  Priebeh jednej iteracie:                                          */
/*    1. setup() — naplni strukturu n prvkami (nemeria sa)            */
/*    2. fn()    — vykona SC_BLOCK operacii (MERIA SA)                */
/*    vysledok  = celkovy_cas / SC_BLOCK  →  cas jednej operacie      */
/*                                                                    */
/*  Struktura ma pocas merania velkost ~n (ide z n na n+SC_BLOCK      */
/*  pri inserte, alebo zostava n pri search). Pri n >> SC_BLOCK       */
/*  je tato zmena zanedbatelna (<0.1%).                               */
/* ================================================================ */

Stats bench_block(std::function<void()> setup,
                  std::function<void()> fn) {
    std::vector<double> s;
    s.reserve(REPEATS_SC);
    for (int r = 0; r < REPEATS_SC; ++r) {
        setup();
        auto t0 = Clock::now();
        fn();
        auto t1 = Clock::now();
        s.push_back((double)std::chrono::duration_cast<Ns>(t1 - t0).count()
                    / (double)SC_BLOCK);
    }
    double mean = std::accumulate(s.begin(), s.end(), 0.0) / REPEATS_SC;
    double var  = 0.0;
    for (double x : s) var += (x - mean) * (x - mean);
    return {mean, std::sqrt(var / REPEATS_SC)};
}

/* ================================================================ */
/*  Wrappers — jednotne API                                           */
/* ================================================================ */

struct HC {
    HashTable *h = nullptr;
    void init()               { h = create_table(INIT_SIZE); }
    void destroy()            { if (h) { free_table(h); h = nullptr; } }
    void insert(long long k)  { ::insert(h, k); }
    bool search(long long k)  { return ::search(h, k) != nullptr; }
    void remove(long long k)  { delete_key(h, k); }
};

struct HD {
    HashTableDouble *h = nullptr;
    void init()               { h = create_table_double(INIT_SIZE); }
    void destroy()            { if (h) { free_table_double(h); h = nullptr; } }
    void insert(long long k)  { insert_double(h, k); }
    bool search(long long k)  { return search_double(h, k) != -1; }
    void remove(long long k)  { delete_key_double(h, k); }
};

struct RBT {
    rbt_tree_t *t = nullptr;
    void init()               { t = rbt_create(); }
    void destroy()            { if (t) { rbt_destroy(t); t = nullptr; } }
    void insert(long long k)  { rbt_insert(t, k); }
    bool search(long long k)  { return rbt_search(t, k) != nullptr; }
    void remove(long long k)  { rbt_delete(t, k); }
};

struct T23 {
    T23Tree t;
    T23() { tree23_init(&t); }
    void init()               { tree23_init(&t); }
    void destroy()            { tree23_free(&t); }
    void insert(long long k)  { tree23_insert(&t, k); }
    bool search(long long k)  { return tree23_search(&t, k); }
    void remove(long long k)  { tree23_delete(&t, k); }
};

/* ================================================================ */
/*  Vysledky                                                          */
/* ================================================================ */

struct BenchResult {
    std::string scenario;
    std::string structure;
    long long   n;
    double      mean_ns;
    double      stddev_ns;
};

static std::vector<BenchResult> g_results;

void record(const std::string &sc, const std::string &ds, long long n, Stats s) {
    BenchResult br;
    br.scenario  = sc;
    br.structure = ds;
    br.n         = n;
    br.mean_ns   = s.mean_ns;
    br.stddev_ns = s.stddev_ns;
    g_results.push_back(br);
    std::printf("  [%-12s] n=%8lld  %9.3f ns/op  (stddev %.3f)\n",
                ds.c_str(), (long long)n, s.mean_ns, s.stddev_ns);
}

/* ================================================================ */
/*  Makro: spusti scenar pre vsetky 4 DS (bulk)                       */
/* ================================================================ */

#define RUN4(sc, n, SETUP, OP) \
do { \
    { HC  ds; record(sc,"hash_chain",  n, bench(n,[&]{ds.destroy();ds.init();SETUP},[&]{OP})); ds.destroy(); } \
    { HD  ds; record(sc,"hash_double", n, bench(n,[&]{ds.destroy();ds.init();SETUP},[&]{OP})); ds.destroy(); } \
    { RBT ds; record(sc,"rbt",         n, bench(n,[&]{ds.destroy();ds.init();SETUP},[&]{OP})); ds.destroy(); } \
    { T23 ds; record(sc,"t23",         n, bench(n,[&]{ds.destroy();ds.init();SETUP},[&]{OP})); ds.destroy(); } \
} while(0)

/*
 * Makro: spusti SC scenar pre vsetky 4 DS (blokove meranie)
 *
 * SETUP — naplni strukturu n prvkami (nemeria sa)
 * OP    — blok SC_BLOCK operacii (meria sa), vysledok = cas / SC_BLOCK
 */
#define RUN4_BLOCK(sc, n, SETUP, OP) \
do { \
    { HC  ds; record(sc,"hash_chain",  n, bench_block([&]{ds.destroy();ds.init();SETUP},[&]{OP})); ds.destroy(); } \
    { HD  ds; record(sc,"hash_double", n, bench_block([&]{ds.destroy();ds.init();SETUP},[&]{OP})); ds.destroy(); } \
    { RBT ds; record(sc,"rbt",         n, bench_block([&]{ds.destroy();ds.init();SETUP},[&]{OP})); ds.destroy(); } \
    { T23 ds; record(sc,"t23",         n, bench_block([&]{ds.destroy();ds.init();SETUP},[&]{OP})); ds.destroy(); } \
} while(0)

/* ================================================================ */
/*  Povodne scenare (S1–S8)                                           */
/* ================================================================ */

void s1_sequential_insert() {
    std::puts("\n--- S1: Sequential insert ---");
    for (long long n : N_VALUES) {
        auto keys = sequential_keys(n);
        RUN4("S1_seq_insert", n, {}, { for (auto k : keys) ds.insert(k); });
    }
}

void s2_random_insert() {
    std::puts("\n--- S2: Random insert ---");
    for (long long n : N_VALUES) {
        auto keys = random_keys(n);
        RUN4("S2_rand_insert", n, {}, { for (auto k : keys) ds.insert(k); });
    }
}

void s3_search_hit() {
    std::puts("\n--- S3: Search hit (100% existing keys) ---");
    for (long long n : N_VALUES) {
        auto keys = random_keys(n);
        auto look = keys;
        std::shuffle(look.begin(), look.end(), std::mt19937_64(99));
        RUN4("S3_search_hit", n,
             { for (auto k : keys) ds.insert(k); },
             { for (auto k : look) ds.search(k); });
    }
}

void s4_search_miss() {
    std::puts("\n--- S4: Search miss (0% existing keys) ---");
    for (long long n : N_VALUES) {
        auto keys = random_keys(n, 42);
        std::vector<long long> miss(n);
        // Zaporne hodnoty — garantovane nie su v DS (vkladame kladne)
        for (long long i = 0; i < n; ++i) miss[i] = -(i + 1);
        RUN4("S4_search_miss", n,
             { for (auto k : keys) ds.insert(k); },
             { for (auto k : miss) ds.search(k); });
    }
}

void s5_delete_all() {
    std::puts("\n--- S5: Delete all ---");
    for (long long n : N_VALUES) {
        auto keys = random_keys(n);
        auto del  = keys;
        std::shuffle(del.begin(), del.end(), std::mt19937_64(77));
        RUN4("S5_delete_all", n,
             { for (auto k : keys) ds.insert(k); },
             { for (auto k : del)  ds.remove(k); });
    }
}

void s6_mixed() {
    std::puts("\n--- S6: Mixed workload (70% search / 20% insert / 10% delete) ---");
    for (long long n : N_VALUES) {
        auto base = random_keys(n, 42);
        struct Op { int type; long long key; };
        std::vector<Op> ops;
        ops.reserve(n);
        std::mt19937_64 rng(55);
        std::uniform_int_distribution<int> d(0, 9);
        long long extra = n * 20LL + 1;
        for (long long i = 0; i < n; ++i) {
            int r = d(rng);
            if      (r < 7) ops.push_back({0, base[i % n]});
            else if (r < 9) ops.push_back({1, extra++});
            else            ops.push_back({2, base[i % n]});
        }
        RUN4("S6_mixed", n,
             { for (auto k : base) ds.insert(k); },
             { for (auto &o : ops) {
                   if      (o.type == 0) ds.search(o.key);
                   else if (o.type == 1) ds.insert(o.key);
                   else                 ds.remove(o.key);
               }
             });
    }
}

void s7_hash_worst() {
    std::puts("\n--- S7: Hash worst-case (all keys -> same slot) ---");
    for (long long n : N_VALUES) {
        if (n > 100000LL) {
            std::printf("  [skip] n=%lld -- too slow for worst-case\n", n);
            continue;
        }
        auto keys = collision_keys(n, 1009LL);
        RUN4("S7_hash_worst", n, {}, { for (auto k : keys) ds.insert(k); });
    }
}

void s8_sorted_insert() {
    std::puts("\n--- S8: Sorted insert (stress test for trees) ---");
    for (long long n : N_VALUES) {
        auto keys = sequential_keys(n);
        RUN4("S8_sorted_insert", n, {}, { for (auto k : keys) ds.insert(k); });
    }
}

/* ================================================================ */
/*  Scenare cistej casovej zlozitosti (SC_*)                          */
/*                                                                    */
/*  Kazdy scenar meria cas JEDNEJ operacie odvodeny z bloku           */
/*  SC_BLOCK operacii na strukture velkosti ~n.                       */
/*                                                                    */
/*  Postup:                                                           */
/*    setup:  vloz n prvkov  (struktura ma velkost n)                 */
/*    meras:  vykonaj SC_BLOCK operacii                               */
/*    cas/op: celkovy_cas / SC_BLOCK                                  */
/*                                                                    */
/*  Pri n >> SC_BLOCK je zmena velkosti struktury pocas merania       */
/*  zanedbatelna. Vykreslenim ns/op vs n ziskas tvar krivky           */
/*  z ktoreho odcitas casovu zlozitost:                               */
/*    konstanta → O(1),  log(n) → O(log n),  linearna → O(n)         */
/* ================================================================ */

/*
 * SC_insert — cas vlozenia jedneho noveho prvku do struktury s n prvkami.
 *
 * setup:  vloz n nahodnych klucov
 * meras:  vloz SC_BLOCK novych jedinecnych klucov (zaporne = zarucene unikatne)
 *
 * Struktura ide z n na n+SC_BLOCK, co je pri n >= 5000 zmena < 0.1% (pri SC_BLOCK=1000 iba pri n=5000 je to 20%, ale stale omnoho lepsi odhad nez povodny pristup kde struktura ide z 0 na n).
 * Pre spravnost merania pri malych n mozeme prijat tuto aproximaciu.
 */
void sc_insert() {
    std::puts("\n--- SC_insert: insert na strukture velkosti ~n (blok SC_BLOCK operacii) ---");
    for (long long n : N_VALUES) {
        auto keys = random_keys(n, 42);
        long long next_key = 0;  /* pocitadlo pre zaporne unikatne kluce */
        RUN4_BLOCK("SC_insert", n,
            /* SETUP */ {
                next_key = 0;
                for (auto k : keys) ds.insert(k);
            },
            /* OP (SC_BLOCK insertov) */ {
                for (long long i = 0; i < SC_BLOCK; ++i)
                    ds.insert(-(++next_key));
            }
        );
    }
}

/*
 * SC_search_hit — cas vyhladania existujuceho prvku v strukture s n prvkami.
 *
 * setup:  vloz n klucov
 * meras:  vyhladaj SC_BLOCK klucov zo vlozenych (rotujeme cez zoznam)
 *
 * Search nemeni strukturu — velkost zostava presne n pocas celeho merania.
 */
void sc_search_hit() {
    std::puts("\n--- SC_search_hit: search (hit) na strukture velkosti n (blok SC_BLOCK operacii) ---");
    for (long long n : N_VALUES) {
        auto keys = random_keys(n, 42);
        long long idx = 0;
        RUN4_BLOCK("SC_search_hit", n,
            /* SETUP */ {
                idx = 0;
                for (auto k : keys) ds.insert(k);
            },
            /* OP (SC_BLOCK searchov) */ {
                for (long long i = 0; i < SC_BLOCK; ++i)
                    ds.search(keys[(idx++) % n]);
            }
        );
    }
}

/*
 * SC_search_miss — cas vyhladania neexistujuceho prvku v strukture s n prvkami.
 *
 * setup:  vloz n kladnych klucov
 * meras:  hladaj SC_BLOCK zapornych klucov (garantovane nie su v strukture)
 *
 * Search nemeni strukturu — velkost zostava presne n.
 */
void sc_search_miss() {
    std::puts("\n--- SC_search_miss: search (miss) na strukture velkosti n (blok SC_BLOCK operacii) ---");
    for (long long n : N_VALUES) {
        auto keys = random_keys(n, 42);
        long long next_miss = 0;
        RUN4_BLOCK("SC_search_miss", n,
            /* SETUP */ {
                next_miss = 0;
                for (auto k : keys) ds.insert(k);
            },
            /* OP (SC_BLOCK searchov) */ {
                for (long long i = 0; i < SC_BLOCK; ++i)
                    ds.search(-(++next_miss));
            }
        );
    }
}

/*
 * SC_delete — cas vymazania jedneho prvku zo struktury s n prvkami.
 *
 * setup:  vloz n klucov + SC_BLOCK extra klucov (zaporne, unikatne)
 * meras:  vymas SC_BLOCK extra klucov
 *
 * Struktura ide z n+SC_BLOCK na n, cize pocas merania ma velkost ~n.
 * Symetricke k SC_insert — insert ide z n na n+K, delete ide z n+K na n.
 */
void sc_delete() {
    std::puts("\n--- SC_delete: delete na strukture velkosti ~n (blok SC_BLOCK operacii) ---");
    for (long long n : N_VALUES) {
        auto keys = random_keys(n, 42);
        std::vector<long long> extra(SC_BLOCK);
        for (long long i = 0; i < SC_BLOCK; ++i) extra[i] = -(i + 1);
        RUN4_BLOCK("SC_delete", n,
            /* SETUP */ {
                for (auto k : keys)  ds.insert(k);
                for (auto k : extra) ds.insert(k);  /* vloz extra prvky ktore budeme mazat */
            },
            /* OP (SC_BLOCK deletov) */ {
                for (auto k : extra) ds.remove(k);
            }
        );
    }
}

/* ================================================================ */
/*  Export CSV                                                        */
/* ================================================================ */

void export_csv() {
    // std::filesystem::create_directories("results");
    _mkdir("results");
    // Sumarny subor
    {
        std::ofstream f("results/summary.csv");
        f << "scenario,structure,n,time_per_op_ns,stddev_ns\n";
        for (auto &r : g_results)
            f << r.scenario  << ","
              << r.structure << ","
              << r.n         << ","
              << r.mean_ns   << ","
              << r.stddev_ns << "\n";
    }
    // Subor pre kazdy scenar
    std::vector<std::string> seen;
    for (auto &r : g_results) {
        bool already = false;
        for (auto &s : seen) if (s == r.scenario) { already = true; break; }
        if (already) continue;
        seen.push_back(r.scenario);
        std::ofstream f("results/" + r.scenario + ".csv");
        f << "structure,n,time_per_op_ns,stddev_ns\n";
        for (auto &x : g_results)
            if (x.scenario == r.scenario)
                f << x.structure << ","
                  << x.n         << ","
                  << x.mean_ns   << ","
                  << x.stddev_ns << "\n";
    }
    std::puts("\nVysledky ulozene do adresara results/");
}

/* ================================================================ */
/*  main                                                              */
/* ================================================================ */

int main() {
    std::puts("=== Benchmark datovych struktur ===");
    std::printf("Opakovania (bulk):  %d\n", REPEATS);
    std::printf("Opakovania (SC):    %d  x  blok %lld ops\n", REPEATS_SC, SC_BLOCK);
    std::printf("n hodnoty:  ");
    for (auto n : N_VALUES) std::printf("%lld ", n);
    std::puts("");

    s1_sequential_insert(); printf("S1 OK\n"); fflush(stdout);
    s2_random_insert();     printf("S2 OK\n"); fflush(stdout);
    s3_search_hit();        printf("S3 OK\n"); fflush(stdout);
    s4_search_miss();       printf("S4 OK\n"); fflush(stdout);
    s5_delete_all();        printf("S5 OK\n"); fflush(stdout);
    s6_mixed();             printf("S6 OK\n"); fflush(stdout);
    s7_hash_worst();        printf("S7 OK\n"); fflush(stdout);

    // s8_sorted_insert();     printf("S8 OK\n"); fflush(stdout);

    sc_insert();            printf("SC_insert OK\n");      fflush(stdout);
    sc_search_hit();        printf("SC_search_hit OK\n");  fflush(stdout);
    sc_search_miss();       printf("SC_search_miss OK\n"); fflush(stdout);
    sc_delete();            printf("SC_delete OK\n");      fflush(stdout);

    export_csv();
    std::puts("\nHotovo.");
    return 0;
}