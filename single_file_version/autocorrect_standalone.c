/**
 * ============================================================================
 * PROJECT: AUTO-CORRECT & PREDICTIVE TEXT ENGINE IN C
 * DATA STRUCTURES COURSE PROJECT
 * ----------------------------------------------------------------------------
 * DATA STRUCTURES & ALGORITHMS:
 *   1. Trie (Prefix Tree) for O(L) instant prefix search and auto-completion.
 *   2. Priority Queue / Binary Heap for Top-K frequency ranking.
 *   3. Levenshtein Distance Dynamic Programming for edit-distance calculation.
 *   4. Bounded Trie-Search Pruning for rapid fuzzy matching.
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <time.h>

#define ALPHABET_SIZE 26
#define MAX_WORD_LEN 64
#define MAX_INPUT_LEN 256
#define MAX_SUGGESTIONS 10
#define DEFAULT_DICT_PATH "data/dictionary.txt"

#define MIN3(a, b, c) ((a) < (b) ? ((a) < (c) ? (a) : (c)) : ((b) < (c) ? (b) : (c)))

/* ============================================================================
 * 1. TRIE DATA STRUCTURE DEFINITIONS
 * ============================================================================ */

typedef struct TrieNode {
    struct TrieNode* children[ALPHABET_SIZE];
    bool is_end_of_word;
    int frequency;
    char* word;
} TrieNode;

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
    if (!node) return NULL;
    node->is_end_of_word = false;
    node->frequency = 0;
    node->word = NULL;
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        node->children[i] = NULL;
    }
    return node;
}

bool insert_word(TrieNode* root, const char* word, int frequency) {
    if (!root || !word || word[0] == '\0') return false;

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
            if (!crawl->children[index]) return NULL;
            crawl = crawl->children[index];
        }
    }
    return crawl;
}

bool search_exact_word(TrieNode* root, const char* word, int* out_freq) {
    if (!root || !word) return false;
    TrieNode* node = search_prefix(root, word);
    if (node && node->is_end_of_word) {
        if (out_freq) *out_freq = node->frequency;
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
        if (node->children[index]->word) free(node->children[index]->word);
        free(node->children[index]);
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
    if (current_depth > *max_depth) *max_depth = current_depth;
    if (root->is_end_of_word) (*total_words)++;

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

/* ============================================================================
 * 2. PRIORITY QUEUE / BINARY HEAP IMPLEMENTATION
 * ============================================================================ */

typedef struct {
    char word[MAX_WORD_LEN];
    int frequency;
    int distance;
} Suggestion;

typedef struct {
    Suggestion* data;
    int size;
    int capacity;
} SuggestionHeap;

static bool is_higher_priority(const Suggestion* a, const Suggestion* b) {
    if (a->distance != b->distance) return a->distance < b->distance;
    if (a->frequency != b->frequency) return a->frequency > b->frequency;
    return strcmp(a->word, b->word) < 0;
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
        } else break;
    }
}

static void sift_down(SuggestionHeap* h, int index) {
    while (index < h->size) {
        int left = 2 * index + 1;
        int right = 2 * index + 2;
        int best = index;

        if (left < h->size && is_higher_priority(&h->data[left], &h->data[best])) best = left;
        if (right < h->size && is_higher_priority(&h->data[right], &h->data[best])) best = right;

        if (best != index) {
            swap_suggestions(&h->data[index], &h->data[best]);
            index = best;
        } else break;
    }
}

void heap_push(SuggestionHeap* h, const char* word, int frequency, int distance) {
    if (!h || !word || word[0] == '\0') return;

    for (int i = 0; i < h->size; i++) {
        if (strcmp(h->data[i].word, word) == 0) {
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
        if (!new_data) return;
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
    if (out) *out = h->data[0];
    h->data[0] = h->data[h->size - 1];
    h->size--;
    if (h->size > 0) sift_down(h, 0);
    return true;
}

void free_heap(SuggestionHeap* h) {
    if (h && h->data) {
        free(h->data);
        h->data = NULL;
        h->size = 0;
        h->capacity = 0;
    }
}

/* ============================================================================
 * 3. SPELL CHECK & LEVENSHTEIN DYNAMIC PROGRAMMING
 * ============================================================================ */

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
                dp[i - 1][j] + 1,
                dp[i][j - 1] + 1,
                dp[i - 1][j - 1] + cost
            );
        }
    }
    return dp[len1][len2];
}

static void collect_predictive_words(TrieNode* node, SuggestionHeap* heap) {
    if (!node) return;
    if (node->is_end_of_word && node->word) {
        heap_push(heap, node->word, node->frequency, 0);
    }
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (node->children[i]) collect_predictive_words(node->children[i], heap);
    }
}

void get_predictive_text(TrieNode* root, const char* prefix, SuggestionHeap* heap) {
    if (!root || !prefix || !heap) return;
    TrieNode* prefix_node = search_prefix(root, prefix);
    if (!prefix_node) return;
    collect_predictive_words(prefix_node, heap);
}

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
        if (current_row[i] < min_val) min_val = current_row[i];
    }

    if (node->is_end_of_word && current_row[target_len] <= max_dist && node->word) {
        heap_push(heap, node->word, node->frequency, current_row[target_len]);
    }

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

    int initial_row[MAX_WORD_LEN + 1];
    for (int i = 0; i <= target_len; i++) initial_row[i] = i;

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

/* ============================================================================
 * 4. DICTIONARY LOADER & PERSISTENCE
 * ============================================================================ */

int normalize_word(const char* input, char* output) {
    if (!input || !output) return 0;
    int len = 0;
    for (int i = 0; input[i] != '\0' && len < MAX_WORD_LEN - 1; i++) {
        if (isalpha((unsigned char)input[i])) {
            output[len++] = (char)tolower((unsigned char)input[i]);
        }
    }
    output[len] = '\0';
    return len;
}

static const struct {
    const char* word;
    int freq;
} BUILTIN_VOCAB[] = {
    {"the", 1000}, {"be", 950}, {"to", 920}, {"of", 900}, {"and", 880},
    {"a", 870}, {"in", 850}, {"that", 820}, {"have", 800}, {"it", 770},
    {"for", 750}, {"not", 740}, {"on", 730}, {"with", 720}, {"he", 700},
    {"as", 690}, {"you", 680}, {"do", 670}, {"at", 660}, {"this", 650},
    {"algorithm", 400}, {"algorithms", 350}, {"array", 380}, {"arrays", 340},
    {"binary", 320}, {"tree", 390}, {"trees", 330}, {"trie", 450}, {"tries", 350},
    {"graph", 310}, {"graphs", 280}, {"heap", 360}, {"heaps", 300},
    {"stack", 370}, {"stacks", 310}, {"queue", 380}, {"queues", 320},
    {"structure", 420}, {"structures", 390}, {"data", 480}, {"database", 340},
    {"program", 440}, {"programming", 490}, {"programmer", 360}, {"progress", 300},
    {"function", 390}, {"functions", 350}, {"variable", 370}, {"pointer", 400},
    {"memory", 380}, {"search", 430}, {"searching", 360}, {"sort", 410},
    {"sorting", 400}, {"computer", 450}, {"predictive", 460}, {"autocorrect", 490},
    {"spellcheck", 440}, {"levenshtein", 390}, {"distance", 370}, {"dynamic", 380},
    {"apple", 350}, {"application", 410}, {"apply", 330}, {"complete", 360},
    {"hello", 450}, {"help", 420}, {"hero", 300}, {"college", 370},
    {"student", 410}, {"students", 380}, {"professor", 390}, {"project", 430}
};

int load_builtin_dictionary(TrieNode* root) {
    if (!root) return 0;
    int count = 0;
    int total = sizeof(BUILTIN_VOCAB) / sizeof(BUILTIN_VOCAB[0]);
    for (int i = 0; i < total; i++) {
        if (insert_word(root, BUILTIN_VOCAB[i].word, BUILTIN_VOCAB[i].freq)) count++;
    }
    return count;
}

int load_dictionary(TrieNode* root, const char* filepath) {
    if (!root) return 0;
    int loaded = 0;
    FILE* file = fopen(filepath, "r");
    if (!file) {
        return load_builtin_dictionary(root);
    }
    char line[128];
    while (fgets(line, sizeof(line), file)) {
        char word_raw[MAX_WORD_LEN];
        int freq = 1;
        int scanned = sscanf(line, "%63s %d", word_raw, &freq);
        if (scanned < 1) continue;
        if (scanned == 1) freq = 10;
        char clean_word[MAX_WORD_LEN];
        if (normalize_word(word_raw, clean_word) > 0) {
            if (insert_word(root, clean_word, freq)) loaded++;
        }
    }
    fclose(file);
    load_builtin_dictionary(root);
    return loaded;
}

static void save_trie_dfs(TrieNode* node, FILE* file, int* count) {
    if (!node) return;
    if (node->is_end_of_word && node->word) {
        fprintf(file, "%s %d\n", node->word, node->frequency);
        (*count)++;
    }
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (node->children[i]) save_trie_dfs(node->children[i], file, count);
    }
}

int save_dictionary(TrieNode* root, const char* filepath) {
    if (!root || !filepath) return 0;
    FILE* file = fopen(filepath, "w");
    if (!file) return 0;
    int count = 0;
    save_trie_dfs(root, file, &count);
    fclose(file);
    return count;
}

/* ============================================================================
 * 5. USER INTERFACE & MAIN CONTROLLER
 * ============================================================================ */

void print_banner(void) {
    printf("\n");
    printf("====================================================================\n");
    printf("     AUTO-CORRECT & PREDICTIVE TEXT ENGINE (C Data Structures)      \n");
    printf("====================================================================\n");
    printf("  [Data Structures: Trie (Prefix Tree) + Heap (PQ) + DP Matrix]     \n");
    printf("====================================================================\n");
}

void print_menu(void) {
    printf("\n");
    printf("-------------------------- MAIN MENU -------------------------------\n");
    printf(" [1] ⚡ Predictive Text / Auto-Complete (By Prefix)\n");
    printf(" [2] 🔍 Spell Check & Auto-Correct (Find Nearest Words)\n");
    printf(" [3] 📝 Sentence Corrector & Interactive Spell Checker\n");
    printf(" [4] ➕ Add / Teach New Word to Dictionary\n");
    printf(" [5] ❌ Remove a Word from Dictionary\n");
    printf(" [6] 📊 Data Structure Analytics & Memory Footprint\n");
    printf(" [7] ⏱️  Run Performance & Complexity Benchmark\n");
    printf(" [8] 💾 Save Dictionary & Exit\n");
    printf("--------------------------------------------------------------------\n");
    printf(" Enter your choice (1-8): ");
}

int main(void) {
    print_banner();

    TrieNode* root = create_trie_node();
    if (!root) {
        fprintf(stderr, "Fatal Error: Memory allocation failed for Trie.\n");
        return 1;
    }

    printf("Loading dictionary from '%s'...\n", DEFAULT_DICT_PATH);
    int words_loaded = load_dictionary(root, DEFAULT_DICT_PATH);
    printf("Successfully initialized Trie with %d words!\n", words_loaded);

    int running = 1;
    char choice_str[32];

    while (running) {
        print_menu();
        if (!fgets(choice_str, sizeof(choice_str), stdin)) break;

        int choice = atoi(choice_str);

        if (choice == 1) {
            char input[MAX_INPUT_LEN];
            char clean_prefix[MAX_WORD_LEN];
            printf("\nEnter word prefix (e.g. 'app', 'prog', 'algo', 'str'): ");
            if (fgets(input, sizeof(input), stdin) && normalize_word(input, clean_prefix) > 0) {
                SuggestionHeap heap;
                init_heap(&heap, 128);

                clock_t start = clock();
                get_predictive_text(root, clean_prefix, &heap);
                clock_t end = clock();
                double elapsed_us = ((double)(end - start) / CLOCKS_PER_SEC) * 1000000.0;

                printf("\nPredictions for prefix '%s' (Search Time: %.2f microseconds):\n", clean_prefix, elapsed_us);
                printf("+-----+----------------------+-----------------+\n");
                printf("| No. | Suggested Word       | Usage Frequency |\n");
                printf("+-----+----------------------+-----------------+\n");

                int count = 0;
                Suggestion s;
                while (count < MAX_SUGGESTIONS && heap_pop(&heap, &s)) {
                    printf("| %-3d | %-20s | %-15d |\n", count + 1, s.word, s.frequency);
                    count++;
                }
                if (count == 0) printf("|  -  | No completions found |        -        |\n");
                printf("+-----+----------------------+-----------------+\n");
                free_heap(&heap);
            }
        }
        else if (choice == 2) {
            char input[MAX_INPUT_LEN];
            char clean_word[MAX_WORD_LEN];
            printf("\nEnter a word to verify (e.g. 'computr', 'teh', 'algoritm', 'helo'): ");
            if (fgets(input, sizeof(input), stdin) && normalize_word(input, clean_word) > 0) {
                int freq = 0;
                if (search_exact_word(root, clean_word, &freq)) {
                    printf("\n=> Exact Match Found! '%s' is spelled correctly. (Frequency: %d)\n", clean_word, freq);
                    increment_frequency(root, clean_word, 1);
                } else {
                    printf("\n=> '%s' is misspelled. Searching nearest suggestions...\n", clean_word);
                    SuggestionHeap heap;
                    init_heap(&heap, 128);

                    clock_t start = clock();
                    get_autocorrect_suggestions(root, clean_word, 2, &heap);
                    clock_t end = clock();
                    double elapsed_us = ((double)(end - start) / CLOCKS_PER_SEC) * 1000000.0;

                    printf("\nTop Auto-Correct Suggestions (Search Time: %.2f microseconds):\n", elapsed_us);
                    printf("+-----+----------------------+---------------+-----------------+\n");
                    printf("| No. | Suggested Correction | Edit Distance | Usage Frequency |\n");
                    printf("+-----+----------------------+---------------+-----------------+\n");

                    int count = 0;
                    Suggestion s;
                    while (count < MAX_SUGGESTIONS && heap_pop(&heap, &s)) {
                        printf("| %-3d | %-20s | %-13d | %-15d |\n", count + 1, s.word, s.distance, s.frequency);
                        count++;
                    }
                    if (count == 0) printf("|  -  | No suggestions found |       -       |        -        |\n");
                    printf("+-----+----------------------+---------------+-----------------+\n");
                    free_heap(&heap);
                }
            }
        }
        else if (choice == 3) {
            char input[512];
            printf("\nEnter a full sentence:\n> ");
            if (fgets(input, sizeof(input), stdin)) {
                char output_sentence[1024] = "";
                char* token = strtok(input, " \t\r\n");
                printf("\nAnalyzing sentence...\n");

                while (token != NULL) {
                    char clean_token[MAX_WORD_LEN];
                    int len = normalize_word(token, clean_token);

                    if (len == 0) {
                        strcat(output_sentence, token);
                        strcat(output_sentence, " ");
                        token = strtok(NULL, " \t\r\n");
                        continue;
                    }

                    if (search_exact_word(root, clean_token, NULL)) {
                        printf("[OK]   '%s' is correct.\n", clean_token);
                        strcat(output_sentence, clean_token);
                    } else {
                        printf("\n[TYPO] Misspelled word detected: '%s'\n", clean_token);
                        SuggestionHeap heap;
                        init_heap(&heap, 32);
                        get_autocorrect_suggestions(root, clean_token, 2, &heap);

                        Suggestion options[5];
                        int opt_count = 0;
                        while (opt_count < 5 && heap_pop(&heap, &options[opt_count])) opt_count++;

                        if (opt_count > 0) {
                            printf("  Suggestions for '%s':\n", clean_token);
                            for (int i = 0; i < opt_count; i++) {
                                printf("    [%d] %s (Distance: %d, Freq: %d)\n", i + 1, options[i].word, options[i].distance, options[i].frequency);
                            }
                            printf("    [0] Keep original '%s'\n", clean_token);
                            printf("  Select replacement (0-%d): ", opt_count);

                            char choice_buf[32];
                            int c = 0;
                            if (fgets(choice_buf, sizeof(choice_buf), stdin)) c = atoi(choice_buf);

                            if (c >= 1 && c <= opt_count) {
                                strcat(output_sentence, options[c - 1].word);
                                increment_frequency(root, options[c - 1].word, 2);
                            } else {
                                strcat(output_sentence, clean_token);
                            }
                        } else {
                            strcat(output_sentence, clean_token);
                        }
                        free_heap(&heap);
                    }
                    strcat(output_sentence, " ");
                    token = strtok(NULL, " \t\r\n");
                }

                printf("\n====================================================================\n");
                printf("FINAL CORRECTED SENTENCE:\n=> %s\n", output_sentence);
                printf("====================================================================\n");
            }
        }
        else if (choice == 4) {
            char input[MAX_INPUT_LEN];
            char clean_word[MAX_WORD_LEN];
            printf("\nEnter new word: ");
            if (fgets(input, sizeof(input), stdin) && normalize_word(input, clean_word) > 0) {
                printf("Enter usage frequency weight (e.g. 50): ");
                char freq_buf[32];
                int freq = 50;
                if (fgets(freq_buf, sizeof(freq_buf), stdin)) {
                    int f = atoi(freq_buf);
                    if (f > 0) freq = f;
                }
                insert_word(root, clean_word, freq);
                printf("Success: Word '%s' (freq: %d) added to Trie!\n", clean_word, freq);
            }
        }
        else if (choice == 5) {
            char input[MAX_INPUT_LEN];
            char clean_word[MAX_WORD_LEN];
            printf("\nEnter word to delete: ");
            if (fgets(input, sizeof(input), stdin) && normalize_word(input, clean_word) > 0) {
                if (delete_word(root, clean_word)) {
                    printf("Success: Word '%s' deleted from Trie!\n", clean_word);
                } else {
                    printf("Error: Word '%s' not found in Trie.\n", clean_word);
                }
            }
        }
        else if (choice == 6) {
            int total_words = 0, total_nodes = 0, max_depth = 0;
            get_trie_stats(root, &total_words, &total_nodes, &max_depth, 0);
            size_t node_memory_bytes = (size_t)total_nodes * sizeof(TrieNode);
            double memory_kb = (double)node_memory_bytes / 1024.0;

            printf("\n====================================================================\n");
            printf("               DATA STRUCTURE & MEMORY ANALYTICS                    \n");
            printf("====================================================================\n");
            printf("  Total Dictionary Words Stored : %d words\n", total_words);
            printf("  Total Trie Nodes Allocated   : %d nodes\n", total_nodes);
            printf("  Maximum Trie Depth (Height)  : %d levels\n", max_depth);
            printf("  Size of single TrieNode      : %zu bytes\n", sizeof(TrieNode));
            printf("  Total Heap Memory (Trie)     : %.2f KB (%.3f MB)\n", memory_kb, memory_kb / 1024.0);
            printf("  Average Nodes per Word       : %.2f\n", total_words > 0 ? (double)total_nodes / total_words : 0.0);
            printf("--------------------------------------------------------------------\n");
            printf("  Time Complexity Profiles:\n");
            printf("  - Exact Lookup (Trie)        : O(L) where L = word length\n");
            printf("  - Predictive Auto-Complete   : O(L + M log K) where M = matches, K = top-K\n");
            printf("  - Spell Check DP Pruning     : O(L1 * L2) branch-bounded\n");
            printf("====================================================================\n");
        }
        else if (choice == 7) {
            const char* test_words[] = {"the", "algorithm", "programming", "structure", "computer", "trie", "predictive"};
            int num_tests = sizeof(test_words) / sizeof(test_words[0]);
            int iterations = 100000;

            printf("\nExecuting %d benchmark lookups in Trie...\n", iterations);
            clock_t start = clock();
            int found = 0;
            for (int i = 0; i < iterations; i++) {
                if (search_exact_word(root, test_words[i % num_tests], NULL)) found++;
            }
            clock_t end = clock();

            double total_ms = ((double)(end - start) / CLOCKS_PER_SEC) * 1000.0;
            double avg_ns = (total_ms * 1000000.0) / iterations;

            printf("=> Completed %d lookups in %.3f ms\n", iterations, total_ms);
            printf("=> Average lookup time per word: %.2f nanoseconds!\n", avg_ns);
            printf("=> Theoretical linear scan is ~150x-500x slower for large vocabularies.\n");
        }
        else if (choice == 8) {
            printf("\nSaving dictionary changes to '%s'...\n", DEFAULT_DICT_PATH);
            int saved = save_dictionary(root, DEFAULT_DICT_PATH);
            printf("Saved %d words to disk.\n", saved);
            printf("Freeing Trie memory...\n");
            free_trie(root);
            printf("All memory cleared. Goodbye!\n");
            running = 0;
        }
    }

    return 0;
}
