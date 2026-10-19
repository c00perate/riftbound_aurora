#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include "config.h"
#include "rng.h"
#include "gamestate.h"

int pf_exception, pf_resolved, deckout;

int main(int argc, char **argv) {
    long num_trials = (argc > 1) ? atol(argv[1]) : 1000000L;

    RNG rng;
    rng.state = (uint64_t)time(NULL) ^ 0x9E3779B97F4A7C15ULL ^ (uint64_t)(uintptr_t)&rng;
    for (int i = 0; i < 32; i++) {
        rng_next(&rng); /* warm up */
    }

    long hist2[MAX_TURN + 2]; 
    memset(hist2, 0, sizeof(hist2));
    long hist3[MAX_TURN + 2]; 
    memset(hist3, 0, sizeof(hist3));

    for (long i = 0; i < num_trials; i++) {
        int t2 = simulate_trial(2, &rng);
        hist2[t2 == -1 ? MAX_TURN + 1 : t2]++;

        int t3 = simulate_trial(3, &rng);
        hist3[t3 == -1 ? MAX_TURN + 1 : t3]++;
    }

    printf("Trials per scenario: %ld   (deck=%d, max_turn=%d, PF_THRESHOLD=%.2f)\n\n",
           num_trials, DECK_SIZE, MAX_TURN, PF_THRESHOLD);

    printf("%4s | %10s %10s %10s | %10s %10s %10s\n",
           "Turn", "P(this)2", "Cum%2", "", "P(this)3", "Cum%3", "");
    printf("     | %10s %10s %10s | %10s %10s %10s\n",
           "start=2", "start=2", "", "start=3", "start=3", "");
    printf("-----+----------------------------------+--------------------------\n");

    double cum2 = 0, cum3 = 0;
    for (int t = 1; t <= MAX_TURN; t++) {
        double p2 = (double)hist2[t] / (double)num_trials;
        double p3 = (double)hist3[t] / (double)num_trials;
        cum2 += p2; cum3 += p3;
        printf("%4d | %9.3f%% %9.3f%%            | %9.3f%% %9.3f%%\n",
               t, p2 * 100.0, cum2 * 100.0, p3 * 100.0, cum3 * 100.0);
    }
    printf("\nNever by turn %d: start=2 -> %.3f%%   start=3 -> %.3f%%\n",
           MAX_TURN,
           100.0 * (double)hist2[MAX_TURN + 1] / (double)num_trials,
           100.0 * (double)hist3[MAX_TURN + 1] / (double)num_trials);
    
    printf("PF with no exhausted runes: %d\n", pf_exception);
    printf("Activated PF: %d\n", pf_resolved);
    printf("Deckout: %d\n", deckout);
    return 0;
}