#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <stdbool.h>
#include "config.h"

/* ---------------------- Game state ---------------------- */
typedef struct {
    int ready_runes;
    int exhausted_runes;
    int energy;                 /* resets each turn */
    int eggs_total;
    int eggs_empowered;
    int dredge_activations;      /* pooled across all Dredge Up copies in hand */
    int hand_count[NUM_CARD_TYPES]; /* BLANK used for mulligan bookkeeping; DREDGE_UP unused (see dredge_activations) */
    Deck deck;
} GameState;

static inline bool can_cast_aurora(const GameState *gs) {
    return gs->hand_count[CARD_AURORA] > 0 &&
           gs->ready_runes >= 2 &&
           (gs->ready_runes + gs->energy) >= 9;
}

int simulate_trial(int start_runes, RNG *rng);

#endif