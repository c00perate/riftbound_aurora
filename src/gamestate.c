#include "gamestate.h"
#include <deck.h>
#include <rng.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

extern int deckout;
extern int pf_exception;
extern int pf_resolved;

static void gain_card(GameState *gs, CardType t) {
    if (t == CARD_DREDGE_UP) {
        gs->dredge_activations += 2;
    }
    else {
        gs->hand_count[t]++;
    }
}

static bool pay_cost(GameState *gs, int cost, int recycle) {
    if (gs->ready_runes < recycle) {
        return false;
    }
    if (gs->ready_runes + gs->energy < cost) {
        return false;
    }
    int allowance = cost - recycle; 
    if (allowance < 0) {
        allowance = 0;
    }
    int energy_used = gs->energy < allowance ? gs->energy : allowance;
    int real_used = cost - energy_used;
    gs->energy -= energy_used;
    gs->ready_runes -= real_used;
    gs->exhausted_runes += (real_used - recycle);
    return true;
}

/* Exact hypergeometric P(at least one Aurora among the next look-L
 * cards), based on the ACTUAL remaining deck composition. */
static double prob_aurora_in_top5(const GameState *gs) {
    int N = gs->deck.count;
    if (N == 0) {
        return 0.0;
    }
    int L = N < 5 ? N : 5;
    int K = count_type_in_deck(&gs->deck, CARD_AURORA);
    if (K <= 0) {
        return 0.0;
    }
    if (N - K < L) {
        return 1.0;
    }
    double prob_zero = 1.0;
    for (int i = 0; i < L; i++) {
        prob_zero *= (double)(N - K - i) / (double)(N - i);
    }
    return 1.0 - prob_zero;
}

static void resolve_clairvoyance(GameState *gs, RNG *rng) {
    int n = gs->deck.count < 5 ? gs->deck.count : 5;
    if (n == 0) {
        return;
    }
    CardType peeked[5];
    deck_peek(&gs->deck, n, peeked);
    deck_remove_top(&gs->deck, n);
    CardType useful[5]; 
    int nu = 0;
    CardType blanks[5]; 
    int nb = 0;
    for (int i = 0; i < n; i++) {
        if (peeked[i] != CARD_AURORA) {
            blanks[nb++] = peeked[i];
        }
        else {
            useful[nu++] = peeked[i];
        }
    }
    shuffle_array(useful, nu, rng);
    shuffle_array(blanks, nb, rng);
    for (int i = 0; i < nb; i++) {
        deck_put_bottom(&gs->deck, blanks[i]);
    }
    /* insert in reverse so useful[0] ends up as the very top card */
    for (int i = nu - 1; i >= 0; i--) {
        deck_put_top(&gs->deck, useful[i]);
    }
    for (int i = 0; i < 2; i++) {
        if (gs->deck.count > 0) gain_card(gs, deck_draw(&gs->deck));
    }
}

static void resolve_promising_future(GameState *gs, RNG *rng, int depth, bool *success) {
    int n = gs->deck.count < 5 ? gs->deck.count : 5;
    if (n == 0) {
        return;
    }
    CardType peeked[5];
    deck_peek(&gs->deck, n, peeked);
    deck_remove_top(&gs->deck, n);

    static const CardType priority[7] = {
            CARD_AURORA, CARD_PROMISING_FUTURE, CARD_CLAIRVOYANCE, CARD_DREDGE_UP,
            CARD_CATALYST, CARD_MOBILIZE, CARD_PLATEWYRM_EGG 
    };
    int pick_idx = -1;
    for (int p = 0; p < 7 && pick_idx == -1; p++) {
        for (int i = 0; i < n; i++) {
            if (peeked[i] != priority[p]) {
                continue;
            }
            if (priority[p] == CARD_AURORA && ((gs->ready_runes + gs->exhausted_runes) < 2)) {
                continue;
            }
            if (priority[p] == CARD_PROMISING_FUTURE && ((gs->ready_runes + gs->exhausted_runes) < 1)) {
                continue;
            }
            pick_idx = i;
            break;
        }
    }
    CardType chosen = (pick_idx != -1) ? peeked[pick_idx] : CARD_BLANK;

    CardType remaining[5]; 
    int nr = 0;
    for (int i = 0; i < n; i++) {
        if (i != pick_idx) {
            remaining[nr++] = peeked[i];
        }
    }
    shuffle_array(remaining, nr, rng);
    for (int i = 0; i < nr; i++) {
        deck_put_bottom(&gs->deck, remaining[i]);
    }

    switch (chosen) {
        case CARD_AURORA:
            *success = true;
            break;
        case CARD_CATALYST:
            gs->exhausted_runes += 2;
            break;
        case CARD_MOBILIZE:
            gs->exhausted_runes += 1;
            break;
        case CARD_CLAIRVOYANCE:
            resolve_clairvoyance(gs, rng);
            break;
        case CARD_DREDGE_UP:
            gs->dredge_activations += 1; /* one use already spent for free */
            if (gs->deck.count > 0) {
                gain_card(gs, deck_draw(&gs->deck));
            }
            break;
        case CARD_PLATEWYRM_EGG:
            gs->eggs_total += 1;
            break;
        case CARD_PROMISING_FUTURE:
            if (depth < PF_MAX_DEPTH) {
                resolve_promising_future(gs, rng, depth + 1, success);
            }
            else {
                deckout++;
                break;
            }
            if (gs->exhausted_runes > 0) {
                gs->exhausted_runes -= 1;
            }
            else {
                gs->energy += 1;
                gs->ready_runes -= 1;
                pf_exception++;
            }
            break;
        default:
            break; /* nothing playable among the look; all go to bottom */
    }
}

int simulate_trial(int start_runes, RNG *rng) {
    GameState gs;
    memset(&gs, 0, sizeof(gs));
    deck_init_and_shuffle(&gs.deck, rng);

    for (int i = 0; i < 4; i++) {
        if (gs.deck.count > 0) gain_card(&gs, deck_draw(&gs.deck));
    }
    
    // can improve mulligans by digging only for aurora
    int to_mull = gs.hand_count[CARD_BLANK] < 2 ? gs.hand_count[CARD_BLANK] : 2;
    for (int i = 0; i < to_mull; i++) {
        gs.hand_count[CARD_BLANK]--;
        deck_put_bottom(&gs.deck, CARD_BLANK);
    }
    for (int i = 0; i < to_mull; i++) {
        if (gs.deck.count > 0) gain_card(&gs, deck_draw(&gs.deck));
    }
    gs.ready_runes = start_runes;
    gs.exhausted_runes = 0;
    bool exhaustedEgg;

    for (int turn = 1; turn <= MAX_TURN; turn++) {
        if (turn > 1) {
            gs.ready_runes += gs.exhausted_runes;
            gs.exhausted_runes = 0;
            gs.ready_runes += 2;
        }
        if (gs.deck.count > 0) {
            gain_card(&gs, deck_draw(&gs.deck));
        }
        gs.energy = gs.eggs_total + 2*gs.eggs_empowered;

        if (can_cast_aurora(&gs)) {
            return turn;
        }

        bool success = false;
        bool changed = true;
        exhaustedEgg = false;

        while (changed) {
            changed = false;
            if (gs.hand_count[CARD_CLAIRVOYANCE] > 0 && pay_cost(&gs, 7, 0)) {
                gs.hand_count[CARD_CLAIRVOYANCE]--;
                resolve_clairvoyance(&gs, rng);
                changed = true;
                if (can_cast_aurora(&gs)) { success = true; break; }
                continue;
            }
            if (gs.hand_count[CARD_PROMISING_FUTURE] > 0) {
                double p = prob_aurora_in_top5(&gs);
                if (p >= PF_THRESHOLD && pay_cost(&gs, 5, 1)) {
                    gs.hand_count[CARD_PROMISING_FUTURE]--;
                    resolve_promising_future(&gs, rng, 0, &success);
                    pf_resolved++;
                    changed = true;
                    if (success || can_cast_aurora(&gs)) { 
                        success = true; 
                        break; 
                    }
                    continue;
                }
            }
            if (gs.hand_count[CARD_PLATEWYRM_EGG] > 0 && gs.eggs_total == 0 && pay_cost(&gs, 3, 0)) {
                gs.hand_count[CARD_PLATEWYRM_EGG]--;
                gs.eggs_total += 1;
                if (pay_cost(&gs, 1, 0)) {
                    exhaustedEgg = false;
                }
                else {
                    exhaustedEgg = true;
                }
                changed = true;
                continue;
            }
            if (gs.eggs_empowered < gs.eggs_total && exhaustedEgg == false && pay_cost(&gs, 1, 0)) {
                gs.eggs_empowered += 1;
                changed = true;
                continue;
            }
            if (gs.hand_count[CARD_CATALYST] > 0 && pay_cost(&gs, 4, 0)) {
                gs.hand_count[CARD_CATALYST]--;
                gs.exhausted_runes += 2;
                changed = true;
                continue;
            }
            if (gs.hand_count[CARD_MOBILIZE] > 0 && pay_cost(&gs, 2, 0)) {
                gs.hand_count[CARD_MOBILIZE]--;
                gs.exhausted_runes += 1;
                changed = true;
                continue;
            }
            if (gs.dredge_activations > 0 && pay_cost(&gs, 2, 0)) {
                gs.dredge_activations--;
                if (gs.deck.count > 0) gain_card(&gs, deck_draw(&gs.deck));
                changed = true;
                if (can_cast_aurora(&gs)) { 
                    success = true; 
                    break; 
                }
                continue;
            }
        }
        if (success || can_cast_aurora(&gs)) {
            return turn;
        }
    }
    return -1;
}