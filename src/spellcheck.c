#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../include/spellcheck.h"

#define MIN3(a, b, c) ((a) < (b) ? ((a) < (c) ? (a) : (c)) : ((b) < (c) ? (b) : (c)))

int levenshtein_distance(const char* s1, const char* s2) {
    if (!s1 || !s2) return 999;

    int len1 = (int)strlen(s1);
    int len2 = (int)strlen(s2);

    if (len1 == 0) return len2;
    if (len2 == 0) return len1;

    int dp[MAX_WORD_LEN + 1][MAX_WORD_LEN + 1];

    for (int i = 0; i <= len1; i++) dp[i][0] = i;
    for (int j = 0; j <= len2; j++) dp[0][j] = j;

    for (int i = 1; i <= len1; i++) {
        for (int j = 1; j <= len2; j++) {
            int cost = (tolower((unsigned char)s1[i - 1]) == tolower((unsigned char)s2[j - 1])) ? 0 : 1;
            dp[i][j] = MIN3(
                dp[i - 1][j] + 1,       // Deletion
                dp[i][j - 1] + 1,       // Insertion
                dp[i - 1][j - 1] + cost // Substitution
            );
        }
    }

    return dp[len1][len2];
}

bool is_valid_word(TrieNode* root, const char* word) {
    return search_exact_word(root, word, NULL);
}

// Recursive helper to collect all words starting from a prefix node
static void collect_predictive_words(TrieNode* node, SuggestionHeap* heap) {
    if (!node) return;

    if (node->is_end_of_word && node->word) {
        heap_push(heap, node->word, node->frequency, 0);
    }

    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (node->children[i]) {
            collect_predictive_words(node->children[i], heap);
        }
    }
}

void get_predictive_text(TrieNode* root, const char* prefix, SuggestionHeap* heap) {
    if (!root || !prefix || !heap) return;

    TrieNode* prefix_node = search_prefix(root, prefix);
    if (!prefix_node) return;

    collect_predictive_words(prefix_node, heap);
}

/**
 * @brief Recursive Trie traversal combined with Dynamic Programming for Levenshtein distance.
 * Prunes search branches where minimum edit distance in the DP row exceeds max_dist.
 */
static void search_trie_fuzzy(
    TrieNode* node,
    char current_char,
    const char* target,
    int target_len,
    const int* previous_row,
    int max_dist,
    SuggestionHeap* heap
) {
    int current_row[MAX_WORD_LEN + 1];
    current_row[0] = previous_row[0] + 1;

    int min_val = current_row[0];

    for (int i = 1; i <= target_len; i++) {
        int cost = (tolower((unsigned char)target[i - 1]) == tolower((unsigned char)current_char)) ? 0 : 1;
        int insert_cost = current_row[i - 1] + 1;
        int delete_cost = previous_row[i] + 1;
        int replace_cost = previous_row[i - 1] + cost;

        current_row[i] = MIN3(insert_cost, delete_cost, replace_cost);
        if (current_row[i] < min_val) {
            min_val = current_row[i];
        }
    }

    // If node completes a word and distance is within threshold, add to suggestions
    if (node->is_end_of_word && current_row[target_len] <= max_dist && node->word) {
        heap_push(heap, node->word, node->frequency, current_row[target_len]);
    }

    // Prune branches: only recurse if at least one entry in current_row <= max_dist
    if (min_val <= max_dist) {
        for (int i = 0; i < ALPHABET_SIZE; i++) {
            if (node->children[i]) {
                search_trie_fuzzy(
                    node->children[i],
                    (char)('a' + i),
                    target,
                    target_len,
                    current_row,
                    max_dist,
                    heap
                );
            }
        }
    }
}

void get_autocorrect_suggestions(TrieNode* root, const char* typo, int max_edit_distance, SuggestionHeap* heap) {
    if (!root || !typo || !heap) return;

    int target_len = (int)strlen(typo);
    if (target_len == 0 || target_len >= MAX_WORD_LEN) return;

    // Initial DP row for root (representing an empty prefix string)
    int initial_row[MAX_WORD_LEN + 1];
    for (int i = 0; i <= target_len; i++) {
        initial_row[i] = i;
    }

    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (root->children[i]) {
            search_trie_fuzzy(
                root->children[i],
                (char)('a' + i),
                typo,
                target_len,
                initial_row,
                max_edit_distance,
                heap
            );
        }
    }
}
