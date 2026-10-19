#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

/* ---------------------- Config ---------------------- */
#define DECK_SIZE 39
#define MAX_TURN 7
#define PF_THRESHOLD 0.42 
#define PF_MAX_DEPTH 3      

typedef enum {
    CARD_BLANK = 0,
    CARD_AURORA,
    CARD_PROMISING_FUTURE,
    CARD_MOBILIZE,
    CARD_CATALYST,
    CARD_CLAIRVOYANCE,
    CARD_DREDGE_UP,
    CARD_PLATEWYRM_EGG,
    NUM_CARD_TYPES
} CardType;

typedef struct { 
    uint64_t state; 
} RNG;

typedef struct {
    CardType cards[DECK_SIZE];
    int head;
    int count;
} Deck;

#endif