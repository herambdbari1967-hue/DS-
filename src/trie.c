#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../include/trie.h"

// Helper function to safely duplicate a string (C99 portable)
static char* safe_strdup(const char* s) {
    if (!s) return NULL;
    size_t len = strlen(s) + 1;
    char* copy = (char*)malloc(len);
    if (copy) {
        memcpy(copy, s, len);
    }
    return copy;
}

TrieNode* create_trie_node(void) {
    TrieNode* node = (TrieNode*)malloc(sizeof(TrieNode));
    if (!node) {
        fprintf(stderr, "Error: Memory allocation failed for TrieNode!\n");
        return NULL;
    }
    node->is_end_of_word = false;
    node->frequency = 0;
    node->word = NULL;
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        node->children[i] = NULL;
    }
    return node;
}

bool insert_word(TrieNode* root, const char* word, int frequency) {
    if (!root || !word || word[0] == '\0') {
        return false;
    }

    TrieNode* crawl = root;
    char clean_word[MAX_WORD_LEN];
    int len = 0;

    for (int i = 0; word[i] != '\0' && len < MAX_WORD_LEN - 1; i++) {
        if (isalpha((unsigned char)word[i])) {
            char lower = (char)tolower((unsigned char)word[i]);
            clean_word[len++] = lower;
            int index = lower - 'a';

            if (!crawl->children[index]) {
                crawl->children[index] = create_trie_node();
                if (!crawl->children[index]) return false;
            }
            crawl = crawl->children[index];
        }
    }
    clean_word[len] = '\0';

    if (len == 0) return false;

    crawl->is_end_of_word = true;
    if (frequency > 0) {
        crawl->frequency = (crawl->frequency > 0) ? (crawl->frequency + frequency) : frequency;
    } else if (crawl->frequency == 0) {
        crawl->frequency = 1;
    }

    if (!crawl->word) {
        crawl->word = safe_strdup(clean_word);
    }

    return true;
}

TrieNode* search_prefix(TrieNode* root, const char* prefix) {
    if (!root || !prefix) return NULL;

    TrieNode* crawl = root;
    for (int i = 0; prefix[i] != '\0'; i++) {
        if (isalpha((unsigned char)prefix[i])) {
            int index = tolower((unsigned char)prefix[i]) - 'a';
            if (!crawl->children[index]) {
                return NULL;
            }
            crawl = crawl->children[index];
        }
    }
    return crawl;
}

bool search_exact_word(TrieNode* root, const char* word, int* out_freq) {
    if (!root || !word) return false;

    TrieNode* node = search_prefix(root, word);
    if (node && node->is_end_of_word) {
        if (out_freq) {
            *out_freq = node->frequency;
        }
        return true;
    }
    return false;
}

void increment_frequency(TrieNode* root, const char* word, int delta) {
    if (!root || !word || delta <= 0) return;

    TrieNode* node = search_prefix(root, word);
    if (node && node->is_end_of_word) {
        node->frequency += delta;
    } else {
        // If word not yet in trie, insert it with this frequency
        insert_word(root, word, delta);
    }
}

static bool has_children(TrieNode* node) {
    if (!node) return false;
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (node->children[i]) return true;
    }
    return false;
}

static bool delete_helper(TrieNode* node, const char* word, int depth) {
    if (!node) return false;

    if (depth == (int)strlen(word)) {
        if (!node->is_end_of_word) return false;

        node->is_end_of_word = false;
        node->frequency = 0;
        if (node->word) {
            free(node->word);
            node->word = NULL;
        }

        return !has_children(node);
    }

    int index = tolower((unsigned char)word[depth]) - 'a';
    if (index < 0 || index >= ALPHABET_SIZE) return false;

    if (delete_helper(node->children[index], word, depth + 1)) {
        free_trie(node->children[index]);
        node->children[index] = NULL;
        return (!node->is_end_of_word && !has_children(node));
    }

    return false;
}

bool delete_word(TrieNode* root, const char* word) {
    if (!root || !word || word[0] == '\0') return false;
    if (!search_exact_word(root, word, NULL)) return false;
    delete_helper(root, word, 0);
    return true;
}

void get_trie_stats(TrieNode* root, int* total_words, int* total_nodes, int* max_depth, int current_depth) {
    if (!root) return;

    (*total_nodes)++;
    if (current_depth > *max_depth) {
        *max_depth = current_depth;
    }

    if (root->is_end_of_word) {
        (*total_words)++;
    }

    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (root->children[i]) {
            get_trie_stats(root->children[i], total_words, total_nodes, max_depth, current_depth + 1);
        }
    }
}

void free_trie(TrieNode* root) {
    if (!root) return;

    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (root->children[i]) {
            free_trie(root->children[i]);
            root->children[i] = NULL;
        }
    }

    if (root->word) {
        free(root->word);
        root->word = NULL;
    }

    free(root);
}
