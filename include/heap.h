#ifndef HEAP_H
#define HEAP_H

#include "trie.h"

/**
 * @brief Represents a single candidate suggestion with ranking parameters.
 */
typedef struct {
    char word[MAX_WORD_LEN];
    int frequency;  // Higher frequency = higher priority
    int distance;   // Levenshtein edit distance (0 for exact prefix matches; lower = higher priority)
} Suggestion;

/**
 * @brief Priority Queue (Binary Max/Min Heap) for ranking candidate words.
 * Comparison Criteria:
 * 1. Smaller edit distance has higher priority.
 * 2. If edit distances are equal, higher frequency has higher priority.
 * 3. If both are equal, alphabetical order is used.
 */
typedef struct {
    Suggestion* data;
    int size;
    int capacity;
} SuggestionHeap;

/**
 * @brief Initializes the SuggestionHeap with a given capacity.
 * @param h Pointer to heap.
 * @param capacity Maximum number of suggestions to store.
 */
void init_heap(SuggestionHeap* h, int capacity);

/**
 * @brief Inserts a suggestion into the heap and maintains heap property.
 * @param h Pointer to heap.
 * @param word Suggested word string.
 * @param frequency Usage frequency.
 * @param distance Edit distance.
 */
void heap_push(SuggestionHeap* h, const char* word, int frequency, int distance);

/**
 * @brief Extracts the highest-priority suggestion from the heap.
 * @param h Pointer to heap.
 * @param out Pointer to output Suggestion.
 * @return true if an item was extracted, false if heap is empty.
 */
bool heap_pop(SuggestionHeap* h, Suggestion* out);

/**
 * @brief Checks if the heap is empty.
 */
bool heap_is_empty(const SuggestionHeap* h);

/**
 * @brief Clears all items in the heap.
 */
void heap_clear(SuggestionHeap* h);

/**
 * @brief Frees internal dynamically allocated memory of the heap.
 */
void free_heap(SuggestionHeap* h);

#endif // HEAP_H
