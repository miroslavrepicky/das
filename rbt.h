/**
 * rbt.h - Červeno-čierny strom (Red-Black Tree)
 *
 * Implementácia červeno-čierneho stromu v C99.
 * Podporované operácie: rbt_insert, rbt_search, rbt_delete.
 *
 * Vlastnosti červeno-čierneho stromu:
 *  1. Každý uzol je červený alebo čierny.
 *  2. Koreň je vždy čierny.
 *  3. Každý list (NIL sentinel) je čierny.
 *  4. Ak je uzol červený, obaja jeho synovia sú čierni.
 *  5. Pre každý uzol platí, že všetky cesty z neho do listov
 *     obsahujú rovnaký počet čiernych uzlov.
 */

#ifndef RBT_H
#define RBT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>  /* size_t */
#include <stdbool.h> /* bool   */

/* ------------------------------------------------------------------ */
/*  Farba uzla                                                          */
/* ------------------------------------------------------------------ */

typedef enum {
    RBT_RED   = 0,
    RBT_BLACK = 1
} rbt_color_t;

/* ------------------------------------------------------------------ */
/*  Uzol stromu                                                         */
/* ------------------------------------------------------------------ */

typedef struct rbt_node {
    long long              key;       /* uložená hodnota                     */
    rbt_color_t      color;     /* farba uzla                          */
    struct rbt_node *parent;    /* rodičovský uzol (NULL pre koreň)    */
    struct rbt_node *left;      /* ľavý syn                            */
    struct rbt_node *right;     /* pravý syn                           */
} rbt_node_t;

/* ------------------------------------------------------------------ */
/*  Strom                                                               */
/* ------------------------------------------------------------------ */

typedef struct {
    rbt_node_t *root;   /* koreň stromu                                */
    rbt_node_t *nil;    /* sentinel (zdieľaný NIL uzol)                */
    size_t      size;   /* počet uzlov                                 */
} rbt_tree_t;

/* ------------------------------------------------------------------ */
/*  Životný cyklus                                                      */
/* ------------------------------------------------------------------ */

/**
 * rbt_create - Alokuje a inicializuje nový prázdny strom.
 *
 * Vracia ukazovateľ na strom, alebo NULL pri chybe alokácie.
 */
rbt_tree_t *rbt_create(void);

/**
 * rbt_destroy - Uvoľní všetky uzly aj samotný strom.
 *
 * Po návrate je ukazovateľ neplatný; nastavte ho na NULL.
 */
void rbt_destroy(rbt_tree_t *tree);

/* ------------------------------------------------------------------ */
/*  Hlavné operácie                                                     */
/* ------------------------------------------------------------------ */

/**
 * rbt_insert - Vloží kľúč do stromu.
 *
 * Ak kľúč už existuje, funkcia nevloží duplikát a vráti false.
 * Vracia true pri úspešnom vložení, false pri duplikáte alebo chybe.
 */
bool rbt_insert(rbt_tree_t *tree, long long key);

/**
 * rbt_search - Hľadá uzol s daným kľúčom.
 *
 * Vracia ukazovateľ na uzol, alebo NULL ak kľúč neexistuje.
 * Vrátený ukazovateľ je platný, kým sa strom nemení.
 */
rbt_node_t *rbt_search(const rbt_tree_t *tree, long long key);

/**
 * rbt_delete - Odstráni uzol s daným kľúčom zo stromu.
 *
 * Vracia true ak bol kľúč nájdený a odstránený, false ak neexistoval.
 */
bool rbt_delete(rbt_tree_t *tree, long long key);

/* ------------------------------------------------------------------ */
/*  Pomocné / informačné funkcie                                        */
/* ------------------------------------------------------------------ */

/**
 * rbt_size - Počet uzlov v strome.
 */
size_t rbt_size(const rbt_tree_t *tree);

/**
 * rbt_is_empty - Vráti true ak je strom prázdny.
 */
bool rbt_is_empty(const rbt_tree_t *tree);

/**
 * rbt_min - Uzol s najmenším kľúčom, alebo NULL pre prázdny strom.
 */
rbt_node_t *rbt_min(const rbt_tree_t *tree);

/**
 * rbt_max - Uzol s najväčším kľúčom, alebo NULL pre prázdny strom.
 */
rbt_node_t *rbt_max(const rbt_tree_t *tree);

/**
 * rbt_prlong long - Vypíše strom do stdout (in-order) pre ladenie.
 */
void rbt_print(const rbt_tree_t *tree);


#ifdef __cplusplus
}
#endif

#endif /* RBT_H */