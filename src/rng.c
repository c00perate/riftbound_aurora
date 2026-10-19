#include "rng.h"

void shuffle_array(CardType *arr, int n, RNG *rng) {
    for (int i = n - 1; i > 0; i--) {
        int j = rng_int(rng, i + 1);
        CardType tmp = arr[i]; arr[i] = arr[j]; arr[j] = tmp;
    }
}