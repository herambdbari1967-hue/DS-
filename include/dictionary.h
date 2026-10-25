#ifndef DICTIONARY_H
#define DICTIONARY_H

#include "trie.h"

/**
 * @brief Normalizes a raw input string into a clean lowercase alphabetic word.
 * Removes leading/trailing spaces, converts uppercase to lowercase, ignores punctuation.
 * @param input Raw word or token.
 * @param output Destination buffer for cleaned word.
 * @return Length of normalized word, or 0 if string contains no alphabetic characters.
 */
int normalize_word(const char* input, char* output);

/**
 * @brief Loads words and frequencies from a dictionary file into the Trie.
 * If file does not exist, initializes with a built-in core English vocabulary.
 * @param root Pointer to Trie root.
 * @param filepath Path to dictionary text file.
 * @return Number of words loaded into the Trie.
 */
int load_dictionary(TrieNode* root, const char* filepath);

/**
 * @brief Saves the current Trie dictionary and frequencies back to a file.
 * @param root Pointer to Trie root.
 * @param filepath Destination file path.
 * @return Number of words saved.
 */
int save_dictionary(TrieNode* root, const char* filepath);

/**
 * @brief Loads a rich built-in embedded dictionary in case external file is missing.
 * @param root Pointer to Trie root.
 * @return Number of built-in words loaded.
 */
int load_builtin_dictionary(TrieNode* root);

#endif // DICTIONARY_H
