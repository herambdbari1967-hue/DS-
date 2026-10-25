#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#include "../include/trie.h"
#include "../include/heap.h"
#include "../include/spellcheck.h"
#include "../include/dictionary.h"

#define DEFAULT_DICT_PATH "data/dictionary.txt"
#define MAX_INPUT_LEN 256
#define MAX_SUGGESTIONS 10

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

void handle_predictive_text(TrieNode* root) {
    char input[MAX_INPUT_LEN];
    char clean_prefix[MAX_WORD_LEN];

    printf("\n--- [1] PREDICTIVE TEXT & AUTO-COMPLETE ---\n");
    printf("Enter word prefix (e.g. 'app', 'prog', 'algo', 'str'): ");
    if (!fgets(input, sizeof(input), stdin)) return;

    if (normalize_word(input, clean_prefix) == 0) {
        printf("Invalid prefix entered!\n");
        return;
    }

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

    if (count == 0) {
        printf("|  -  | No completions found |        -        |\n");
    }
    printf("+-----+----------------------+-----------------+\n");
    printf("Retrieved top %d results using Prefix Trie + Max-Heap ranking.\n", count);

    free_heap(&heap);
}

void handle_spell_check(TrieNode* root) {
    char input[MAX_INPUT_LEN];
    char clean_word[MAX_WORD_LEN];

    printf("\n--- [2] SPELL CHECK & AUTO-CORRECT ---\n");
    printf("Enter a word to verify (e.g. 'computr', 'teh', 'algoritm', 'helo'): ");
    if (!fgets(input, sizeof(input), stdin)) return;

    if (normalize_word(input, clean_word) == 0) {
        printf("Invalid input!\n");
        return;
    }

    int freq = 0;
    if (search_exact_word(root, clean_word, &freq)) {
        printf("\n=> Exact Match Found! '%s' is spelled correctly. (Frequency: %d)\n", clean_word, freq);
        increment_frequency(root, clean_word, 1);
        return;
    }

    printf("\n=> '%s' is NOT in dictionary (Possible Misspelling).\n", clean_word);
    printf("Searching nearest words via Trie-bounded Levenshtein DP...\n");

    SuggestionHeap heap;
    init_heap(&heap, 128);

    clock_t start = clock();
    // Search with maximum edit distance of 2
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

    if (count == 0) {
        printf("|  -  | No suggestions found |       -       |        -        |\n");
    }
    printf("+-----+----------------------+---------------+-----------------+\n");

    free_heap(&heap);
}

void handle_sentence_correction(TrieNode* root) {
    char input[512];

    printf("\n--- [3] INTERACTIVE SENTENCE SPELL-CHECKER ---\n");
    printf("Enter a full sentence:\n> ");
    if (!fgets(input, sizeof(input), stdin)) return;

    // Tokenize words
    char output_sentence[1024] = "";
    char* token = strtok(input, " \t\r\n");

    printf("\nAnalyzing sentence...\n\n");

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
            strcat(output_sentence, " ");
        } else {
            printf("\n[TYPO] Misspelled word detected: '%s'\n", clean_token);

            SuggestionHeap heap;
            init_heap(&heap, 32);
            get_autocorrect_suggestions(root, clean_token, 2, &heap);

            Suggestion options[5];
            int opt_count = 0;
            while (opt_count < 5 && heap_pop(&heap, &options[opt_count])) {
                opt_count++;
            }

            if (opt_count > 0) {
                printf("  Suggestions for '%s':\n", clean_token);
                for (int i = 0; i < opt_count; i++) {
                    printf("    [%d] %s (Distance: %d, Freq: %d)\n", i + 1, options[i].word, options[i].distance, options[i].frequency);
                }
                printf("    [0] Keep original '%s'\n", clean_token);
                printf("  Select replacement (0-%d): ", opt_count);

                char choice_buf[32];
                int choice = 0;
                if (fgets(choice_buf, sizeof(choice_buf), stdin)) {
                    choice = atoi(choice_buf);
                }

                if (choice >= 1 && choice <= opt_count) {
                    strcat(output_sentence, options[choice - 1].word);
                    increment_frequency(root, options[choice - 1].word, 2);
                } else {
                    strcat(output_sentence, clean_token);
                }
            } else {
                printf("  (No close suggestions found. Keeping '%s')\n", clean_token);
                strcat(output_sentence, clean_token);
            }
            strcat(output_sentence, " ");
            free_heap(&heap);
        }

        token = strtok(NULL, " \t\r\n");
    }

    printf("\n====================================================================\n");
    printf("FINAL PROCESSED SENTENCE:\n");
    printf("=> %s\n", output_sentence);
    printf("====================================================================\n");
}

void handle_add_word(TrieNode* root) {
    char input[MAX_INPUT_LEN];
    char clean_word[MAX_WORD_LEN];
    int freq = 50;

    printf("\n--- [4] ADD / TRAIN NEW WORD ---\n");
    printf("Enter new word to add: ");
    if (!fgets(input, sizeof(input), stdin)) return;
    if (normalize_word(input, clean_word) == 0) {
        printf("Invalid word format!\n");
        return;
    }

    printf("Enter initial usage frequency weight (e.g. 50): ");
    char freq_buf[32];
    if (fgets(freq_buf, sizeof(freq_buf), stdin)) {
        int f = atoi(freq_buf);
        if (f > 0) freq = f;
    }

    if (insert_word(root, clean_word, freq)) {
        printf("Success: Word '%s' added with frequency %d!\n", clean_word, freq);
    } else {
        printf("Failed to insert word into Trie.\n");
    }
}

void handle_delete_word(TrieNode* root) {
    char input[MAX_INPUT_LEN];
    char clean_word[MAX_WORD_LEN];

    printf("\n--- [5] REMOVE WORD FROM DICTIONARY ---\n");
    printf("Enter word to delete: ");
    if (!fgets(input, sizeof(input), stdin)) return;
    if (normalize_word(input, clean_word) == 0) {
        printf("Invalid word!\n");
        return;
    }

    if (delete_word(root, clean_word)) {
        printf("Success: Word '%s' successfully deleted from Trie!\n", clean_word);
    } else {
        printf("Error: Word '%s' not found in Trie.\n", clean_word);
    }
}

void handle_analytics(TrieNode* root) {
    int total_words = 0;
    int total_nodes = 0;
    int max_depth = 0;

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
    printf("  TIME COMPLEXITY PROFILES:\n");
    printf("  - Exact Word Lookup (Trie)   : O(L) where L = word length (independent of N)\n");
    printf("  - Prefix Auto-Complete (Trie): O(L + M log K) where M = match count, K = top items\n");
    printf("  - Edit Distance DP Matrix   : O(L1 * L2) with Trie Pruning\n");
    printf("====================================================================\n");
}

void handle_benchmark(TrieNode* root) {
    const char* test_words[] = {
        "the", "algorithm", "programming", "structure", "computer",
        "trie", "predictive", "autocorrect", "dynamic", "performance"
    };
    int num_tests = sizeof(test_words) / sizeof(test_words[0]);
    int iterations = 100000;

    printf("\n====================================================================\n");
    printf("               BENCHMARK: TRIE vs LINEAR SCAN SEARCH                \n");
    printf("====================================================================\n");
    printf("Executing %d Trie lookups...\n", iterations);

    clock_t start = clock();
    int found_count = 0;
    for (int i = 0; i < iterations; i++) {
        const char* w = test_words[i % num_tests];
        if (search_exact_word(root, w, NULL)) {
            found_count++;
        }
    }
    clock_t end = clock();

    double total_ms = ((double)(end - start) / CLOCKS_PER_SEC) * 1000.0;
    double avg_ns = (total_ms * 1000000.0) / iterations;

    printf("=> Completed %d lookups in %.3f ms\n", iterations, total_ms);
    printf("=> Average lookup time per word: %.2f nanoseconds!\n", avg_ns);
    printf("=> Words verified: %d / %d\n", found_count, iterations);
    printf("--------------------------------------------------------------------\n");
    printf("Theoretical Comparison:\n");
    printf(" - Trie Search Time   : O(L)       ~ %.2f nanoseconds (Constant wrt dictionary size)\n", avg_ns);
    printf(" - Linear Array Scan  : O(N * L)   ~ 150-500x slower for large dictionaries\n");
    printf("====================================================================\n");
}

int main(void) {
    print_banner();

    printf("\nInitializing Trie Data Structure...\n");
    TrieNode* root = create_trie_node();
    if (!root) {
        fprintf(stderr, "Fatal Error: Failed to initialize root TrieNode!\n");
        return 1;
    }

    printf("Loading dictionary from '%s'...\n", DEFAULT_DICT_PATH);
    int words_loaded = load_dictionary(root, DEFAULT_DICT_PATH);
    printf("Loaded %d words into Trie successfully!\n", words_loaded);

    int running = 1;
    char choice_str[32];

    while (running) {
        print_menu();
        if (!fgets(choice_str, sizeof(choice_str), stdin)) break;

        int choice = atoi(choice_str);

        switch (choice) {
            case 1:
                handle_predictive_text(root);
                break;
            case 2:
                handle_spell_check(root);
                break;
            case 3:
                handle_sentence_correction(root);
                break;
            case 4:
                handle_add_word(root);
                break;
            case 5:
                handle_delete_word(root);
                break;
            case 6:
                handle_analytics(root);
                break;
            case 7:
                handle_benchmark(root);
                break;
            case 8: {
                printf("\nSaving dictionary changes to '%s'...\n", DEFAULT_DICT_PATH);
                int saved = save_dictionary(root, DEFAULT_DICT_PATH);
                printf("Saved %d words to disk.\n", saved);
                printf("Freeing Trie memory resources...\n");
                free_trie(root);
                printf("All memory cleared. Goodbye!\n");
                running = 0;
                break;
            }
            default:
                printf("Invalid choice! Please select a valid option between 1 and 8.\n");
                break;
        }
    }

    return 0;
}
