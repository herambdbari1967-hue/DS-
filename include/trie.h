#ifndef TRIE_H
#define TRIE_H

#include <stdbool.h>

#define ALPHABET_SIZE 26
#define MAX_WORD_LEN 64

/**
 * @brief Trie Node data structure for prefix-tree storage.
 */
typedef struct TrieNode {
    struct TrieNode* children[ALPHABET_SIZE];
    bool is_end_of_word;
    int frequency;          // Frequency count for predictive ranking
    char* word;             // Stored complete word at end of word for O(1) string retrieval
} TrieNode;

/**
 * @brief Creates and initializes a new Trie node.
 * @return Pointer to newly allocated TrieNode.
 */
TrieNode* create_trie_node(void);

/**
 * @brief Inserts a word with an initial frequency into the Trie.
 * @param root Pointer to the root TrieNode.
 * @param word String to insert (case-insensitive).
 * @param frequency Usage/popularity weight of the word.
 * @return true if inserted or updated successfully, false on error.
 */
bool insert_word(TrieNode* root, const char* word, int frequency);

/**
 * @brief Finds the TrieNode corresponding to the end of a prefix string.
 * @param root Pointer to root of Trie.
 * @param prefix Prefix string.
 * @return Pointer to TrieNode at end of prefix, or NULL if prefix doesn't exist.
 */
TrieNode* search_prefix(TrieNode* root, const char* prefix);

/**
 * @brief Checks if an exact word exists in the Trie.
 * @param root Pointer to root of Trie.
 * @param word Word to search.
 * @param out_freq Optional pointer to receive the word's frequency (can be NULL).
 * @return true if exact word is found, false otherwise.
 */
bool search_exact_word(TrieNode* root, const char* word, int* out_freq);

/**
 * @brief Increments the frequency weight of an existing word.
 * @param root Pointer to root of Trie.
 * @param word Word to update.
 * @param delta Amount to add to frequency.
 */
void increment_frequency(TrieNode* root, const char* word, int delta);

/**
 * @brief Deletes a word from the Trie.
 * @param root Pointer to root of Trie.
 * @param word Word to delete.
 * @return true if deleted, false if word not found.
 */
bool delete_word(TrieNode* root, const char* word);

/**
 * @brief Computes statistics of the Trie (node count, word count, depth).
 * @param root Pointer to root of Trie.
 * @param total_words Pointer to output total word count.
 * @param total_nodes Pointer to output total node count.
 * @param max_depth Pointer to output maximum tree depth.
 * @param current_depth Current depth during recursive traversal.
 */
void get_trie_stats(TrieNode* root, int* total_words, int* total_nodes, int* max_depth, int current_depth);

/**
 * @brief Frees all allocated memory in the Trie recursively.
 * @param root Pointer to root of Trie.
 */
void free_trie(TrieNode* root);

#endif // TRIE_H
