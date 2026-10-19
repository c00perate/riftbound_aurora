#include "deck.h"
#include <string.h>
#include <rng.h>

void deck_init_and_shuffle(Deck *d, RNG *rng) {
    CardType comp[DECK_SIZE];
    int idx = 0;
    for (int i = 0; i < 3; i++) {
        comp[idx++] = CARD_AURORA;
    }
    for (int i = 0; i < 3; i++) {
        comp[idx++] = CARD_PROMISING_FUTURE;
    }
    for (int i = 0; i < 3; i++) {
        comp[idx++] = CARD_MOBILIZE;
    }
    for (int i = 0; i < 3; i++) {
        comp[idx++] = CARD_CATALYST;
    }
    for (int i = 0; i < 3; i++) {
        comp[idx++] = CARD_CLAIRVOYANCE;
    }
    for (int i = 0; i < 3; i++) {
        comp[idx++] = CARD_DREDGE_UP;
    }
    for (int i = 0; i < 3; i++) {
        comp[idx++] = CARD_PLATEWYRM_EGG;
    }
    for (int i = 0; i < 18; i++) {
        comp[idx++] = CARD_BLANK;
    }
    shuffle_array(comp, DECK_SIZE, rng);
    memcpy(d->cards, comp, sizeof(comp));
    d->head = 0;
    d->count = DECK_SIZE;
}

int count_type_in_deck(const Deck *d, CardType type) {
    int c = 0;
    for (int i = 0; i < d->count; i++) {
        if (d->cards[(d->head + i) % DECK_SIZE] == type) {
            c++;
        }
    }
    return c;
}