#ifndef SPELLCHECK_H
#define SPELLCHECK_H

#include "trie.h"
#include "heap.h"

/**
 * @brief Computes standard Levenshtein Edit Distance between two strings using DP.
 * Calculates minimum operations (insert, delete, replace) required.
 * @param s1 First string.
 * @param s2 Second string.
 * @return Minimum edit distance (non-negative integer).
 */
int levenshtein_distance(const char* s1, const char* s2);

/**
 * @brief Gathers predictive text auto-completions for a given prefix.
 * @param root Pointer to Trie root.
 * @param prefix Prefix string typed by the user.
 * @param heap Output Priority Queue to store ranked suggestions.
 */
void get_predictive_text(TrieNode* root, const char* prefix, SuggestionHeap* heap);

/**
 * @brief Finds auto-correction suggestions for a misspelled word using Trie DP search.
 * Prunes Trie branches that exceed max_edit_distance for high performance.
 * @param root Pointer to Trie root.
 * @param typo Misspelled target word.
 * @param max_edit_distance Maximum allowed edit distance (typically 1 or 2).
 * @param heap Output Priority Queue to store ranked suggestions.
 */
void get_autocorrect_suggestions(TrieNode* root, const char* typo, int max_edit_distance, SuggestionHeap* heap);

/**
 * @brief Checks if a word is valid in the dictionary.
 * @param root Pointer to Trie root.
 * @param word Word to verify.
 * @return true if valid dictionary word, false otherwise.
 */
bool is_valid_word(TrieNode* root, const char* word);

#endif // SPELLCHECK_H
