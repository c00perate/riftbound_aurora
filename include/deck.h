#ifndef DECK_H
#define DECK_H

#include "config.h"

/* ---------------------- Deck (circular buffer) ---------------------- */
static inline CardType deck_draw(Deck *d) {
    CardType c = d->cards[d->head];
    d->head = (d->head + 1) % DECK_SIZE;
    d->count--;
    return c;
}
static inline void deck_put_bottom(Deck *d, CardType c) {
    int pos = (d->head + d->count) % DECK_SIZE;
    d->cards[pos] = c;
    d->count++;
}
static inline void deck_put_top(Deck *d, CardType c) {
    d->head = (d->head - 1 + DECK_SIZE) % DECK_SIZE;
    d->cards[d->head] = c;
    d->count++;
}
static inline void deck_peek(const Deck *d, int n, CardType *out) {
    for (int i = 0; i < n; i++) {
        out[i] = d->cards[(d->head + i) % DECK_SIZE];
    }
}
static inline void deck_remove_top(Deck *d, int n) {
    d->head = (d->head + n) % DECK_SIZE;
    d->count -= n;
}

void deck_init_and_shuffle(Deck *d, RNG *rng);
int count_type_in_deck(const Deck *d, CardType type);

#endif