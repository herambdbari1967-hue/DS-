#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../include/dictionary.h"

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

// Built-in core vocabulary with balanced frequencies for reliable standalone execution
static const struct {
    const char* word;
    int freq;
} BUILTIN_WORDS[] = {
    // Top Common English Words
    {"the", 1000}, {"be", 950}, {"to", 920}, {"of", 900}, {"and", 880},
    {"a", 870}, {"in", 850}, {"that", 820}, {"have", 800}, {"i", 780},
    {"it", 770}, {"for", 750}, {"not", 740}, {"on", 730}, {"with", 720},
    {"he", 700}, {"as", 690}, {"you", 680}, {"do", 670}, {"at", 660},
    {"this", 650}, {"but", 640}, {"his", 630}, {"by", 620}, {"from", 610},
    {"they", 600}, {"we", 590}, {"say", 580}, {"her", 570}, {"she", 560},
    {"or", 550}, {"an", 540}, {"will", 530}, {"my", 520}, {"one", 510},
    {"all", 500}, {"would", 490}, {"there", 480}, {"their", 470}, {"what", 460},
    {"so", 450}, {"up", 440}, {"out", 430}, {"if", 420}, {"about", 410},
    {"who", 400}, {"get", 390}, {"which", 380}, {"go", 370}, {"me", 360},
    {"when", 350}, {"make", 340}, {"can", 330}, {"like", 320}, {"time", 310},
    {"no", 300}, {"just", 290}, {"him", 280}, {"know", 270}, {"take", 260},
    {"people", 250}, {"into", 240}, {"year", 230}, {"your", 220}, {"good", 210},
    {"some", 200}, {"could", 195}, {"them", 190}, {"see", 185}, {"other", 180},
    {"than", 175}, {"then", 170}, {"now", 165}, {"look", 160}, {"only", 155},
    {"come", 150}, {"its", 145}, {"over", 140}, {"think", 135}, {"also", 130},
    {"back", 125}, {"after", 120}, {"use", 115}, {"two", 110}, {"how", 105},
    {"our", 100}, {"work", 98}, {"first", 96}, {"well", 94}, {"way", 92},
    {"even", 90}, {"new", 88}, {"want", 86}, {"because", 84}, {"any", 82},
    {"these", 80}, {"give", 78}, {"day", 76}, {"most", 74}, {"us", 72},
    
    // Computer Science & Data Structures Vocab
    {"algorithm", 300}, {"algorithms", 280}, {"array", 290}, {"arrays", 270},
    {"binary", 260}, {"tree", 275}, {"trees", 250}, {"trie", 350}, {"tries", 280},
    {"graph", 240}, {"graphs", 220}, {"heap", 265}, {"heaps", 230},
    {"stack", 280}, {"stacks", 240}, {"queue", 285}, {"queues", 245},
    {"structure", 290}, {"structures", 270}, {"data", 340}, {"database", 250},
    {"program", 310}, {"programming", 330}, {"programmer", 260}, {"progress", 220},
    {"function", 290}, {"functions", 270}, {"variable", 260}, {"variables", 240},
    {"pointer", 280}, {"pointers", 250}, {"memory", 270}, {"allocation", 230},
    {"search", 310}, {"searching", 270}, {"sort", 290}, {"sorting", 280},
    {"quicksort", 220}, {"mergesort", 210}, {"traversal", 230}, {"recursion", 260},
    {"recursive", 240}, {"iterative", 230}, {"complexity", 270}, {"space", 250},
    {"time", 310}, {"performance", 260}, {"computer", 320}, {"computation", 240},
    {"predictive", 320}, {"prediction", 290}, {"autocorrect", 350}, {"correction", 280},
    {"spellcheck", 310}, {"spelling", 270}, {"levenshtein", 300}, {"distance", 280},
    {"dynamic", 290}, {"optimization", 260}, {"optimize", 250}, {"node", 300},
    {"nodes", 280}, {"root", 290}, {"leaf", 240}, {"children", 260}, {"parent", 250},
    
    // Words starting with 'app'
    {"apple", 280}, {"apples", 210}, {"application", 310}, {"applications", 260},
    {"apply", 250}, {"applied", 220}, {"apprentice", 150}, {"appreciate", 180},
    {"approach", 230}, {"appropriate", 190}, {"approval", 170}, {"approve", 180},
    
    // Words starting with 'comp'
    {"complete", 280}, {"completion", 240}, {"complex", 260}, {"compile", 270},
    {"compiler", 290}, {"computation", 230}, {"compare", 240}, {"comparison", 230},
    
    // Words for common spelling mistakes
    {"hello", 350}, {"help", 320}, {"hero", 220}, {"here", 310}, {"hear", 260},
    {"friend", 290}, {"friends", 260}, {"school", 280}, {"college", 270},
    {"student", 300}, {"students", 280}, {"professor", 290}, {"project", 320},
    {"homework", 220}, {"knowledge", 270}, {"science", 280}, {"language", 300},
    {"learning", 290}, {"system", 310}, {"software", 310}, {"engineering", 290}
};

int load_builtin_dictionary(TrieNode* root) {
    if (!root) return 0;
    int count = 0;
    int total = sizeof(BUILTIN_WORDS) / sizeof(BUILTIN_WORDS[0]);

    for (int i = 0; i < total; i++) {
        if (insert_word(root, BUILTIN_WORDS[i].word, BUILTIN_WORDS[i].freq)) {
            count++;
        }
    }
    return count;
}

int load_dictionary(TrieNode* root, const char* filepath) {
    if (!root) return 0;

    int loaded_count = 0;
    FILE* file = fopen(filepath, "r");

    if (!file) {
        printf("[Notice] Dictionary file '%s' not found on disk. Loading embedded core vocabulary...\n", filepath);
        return load_builtin_dictionary(root);
    }

    char line[128];
    while (fgets(line, sizeof(line), file)) {
        char word_raw[MAX_WORD_LEN];
        int freq = 1;

        // Parse line: supports "word frequency" or "word,frequency" or just "word"
        int scanned = sscanf(line, "%63s %d", word_raw, &freq);
        if (scanned < 1) continue;
        if (scanned == 1) freq = 10; // Default frequency

        char clean_word[MAX_WORD_LEN];
        if (normalize_word(word_raw, clean_word) > 0) {
            if (insert_word(root, clean_word, freq)) {
                loaded_count++;
            }
        }
    }

    fclose(file);

    // Also inject builtin CS and core words to ensure rich coverage
    load_builtin_dictionary(root);

    return loaded_count;
}

static void save_trie_words_dfs(TrieNode* node, FILE* file, int* count) {
    if (!node) return;

    if (node->is_end_of_word && node->word) {
        fprintf(file, "%s %d\n", node->word, node->frequency);
        (*count)++;
    }

    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (node->children[i]) {
            save_trie_words_dfs(node->children[i], file, count);
        }
    }
}

int save_dictionary(TrieNode* root, const char* filepath) {
    if (!root || !filepath) return 0;

    FILE* file = fopen(filepath, "w");
    if (!file) {
        fprintf(stderr, "Error: Could not open '%s' for writing!\n", filepath);
        return 0;
    }

    int count = 0;
    save_trie_words_dfs(root, file, &count);
    fclose(file);

    return count;
}
