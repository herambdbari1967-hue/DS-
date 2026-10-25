#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/heap.h"

static bool is_higher_priority(const Suggestion* a, const Suggestion* b) {
    if (a->distance != b->distance) {
        return a->distance < b->distance; // Smaller edit distance first
    }
    if (a->frequency != b->frequency) {
        return a->frequency > b->frequency; // Higher usage frequency first
    }
    return strcmp(a->word, b->word) < 0;    // Alphabetical tie-breaker
}

static void swap_suggestions(Suggestion* a, Suggestion* b) {
    Suggestion temp = *a;
    *a = *b;
    *b = temp;
}

void init_heap(SuggestionHeap* h, int capacity) {
    if (!h) return;
    if (capacity <= 0) capacity = 64;
    h->data = (Suggestion*)malloc(sizeof(Suggestion) * capacity);
    h->size = 0;
    h->capacity = capacity;
}

static void sift_up(SuggestionHeap* h, int index) {
    while (index > 0) {
        int parent = (index - 1) / 2;
        if (is_higher_priority(&h->data[index], &h->data[parent])) {
            swap_suggestions(&h->data[index], &h->data[parent]);
            index = parent;
        } else {
            break;
        }
    }
}

static void sift_down(SuggestionHeap* h, int index) {
    while (index < h->size) {
        int left = 2 * index + 1;
        int right = 2 * index + 2;
        int best = index;

        if (left < h->size && is_higher_priority(&h->data[left], &h->data[best])) {
            best = left;
        }
        if (right < h->size && is_higher_priority(&h->data[right], &h->data[best])) {
            best = right;
        }

        if (best != index) {
            swap_suggestions(&h->data[index], &h->data[best]);
            index = best;
        } else {
            break;
        }
    }
}

void heap_push(SuggestionHeap* h, const char* word, int frequency, int distance) {
    if (!h || !word || word[0] == '\0') return;

    // Check if word already exists in the heap to avoid duplicates
    for (int i = 0; i < h->size; i++) {
        if (strcmp(h->data[i].word, word) == 0) {
            // Keep the one with better ranking
            if (distance < h->data[i].distance || 
               (distance == h->data[i].distance && frequency > h->data[i].frequency)) {
                h->data[i].distance = distance;
                h->data[i].frequency = frequency;
                sift_up(h, i);
                sift_down(h, i);
            }
            return;
        }
    }

    if (h->size >= h->capacity) {
        int new_capacity = h->capacity * 2;
        Suggestion* new_data = (Suggestion*)realloc(h->data, sizeof(Suggestion) * new_capacity);
        if (!new_data) {
            fprintf(stderr, "Warning: Heap memory realloc failed!\n");
            return;
        }
        h->data = new_data;
        h->capacity = new_capacity;
    }

    strncpy(h->data[h->size].word, word, MAX_WORD_LEN - 1);
    h->data[h->size].word[MAX_WORD_LEN - 1] = '\0';
    h->data[h->size].frequency = frequency;
    h->data[h->size].distance = distance;

    sift_up(h, h->size);
    h->size++;
}

bool heap_pop(SuggestionHeap* h, Suggestion* out) {
    if (!h || h->size <= 0) return false;

    if (out) {
        *out = h->data[0];
    }

    h->data[0] = h->data[h->size - 1];
    h->size--;

    if (h->size > 0) {
        sift_down(h, 0);
    }

    return true;
}

bool heap_is_empty(const SuggestionHeap* h) {
    return (!h || h->size == 0);
}

void heap_clear(SuggestionHeap* h) {
    if (h) {
        h->size = 0;
    }
}

void free_heap(SuggestionHeap* h) {
    if (h && h->data) {
        free(h->data);
        h->data = NULL;
        h->size = 0;
        h->capacity = 0;
    }
}
