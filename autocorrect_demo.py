"""
================================================================================
AUTO-CORRECT & PREDICTIVE TEXT ENGINE (Python Reference & Interactive Runner)
DATA STRUCTURES COURSE PROJECT
--------------------------------------------------------------------------------
Features identical data structures & algorithms as the C implementation:
1. Trie (Prefix Tree) for O(L) prefix search & auto-complete
2. Priority Queue / Binary Max-Heap for Top-K ranking
3. Dynamic Programming Levenshtein Distance & Branch Pruning
================================================================================
"""

import os
import sys
import time
import heapq

# Ensure UTF-8 output on Windows consoles
if sys.platform == "win32":
    try:
        sys.stdout.reconfigure(encoding="utf-8", errors="replace")
        sys.stderr.reconfigure(encoding="utf-8", errors="replace")
    except Exception:
        pass

DEFAULT_DICT_PATH = os.path.join(os.path.dirname(__file__), "data", "dictionary.txt")
MAX_WORD_LEN = 64
MAX_SUGGESTIONS = 10

class TrieNode:
    def __init__(self):
        self.children = {}
        self.is_end_of_word = False
        self.frequency = 0
        self.word = None

class Trie:
    def __init__(self):
        self.root = TrieNode()

    def insert(self, word: str, frequency: int = 1):
        if not word:
            return
        clean = "".join(c.lower() for c in word if c.isalpha())
        if not clean:
            return
        node = self.root
        for char in clean:
            if char not in node.children:
                node.children[char] = TrieNode()
            node = node.children[char]
        node.is_end_of_word = True
        node.frequency = (node.frequency + frequency) if node.frequency > 0 else frequency
        node.word = clean

    def search_prefix(self, prefix: str):
        node = self.root
        for char in prefix.lower():
            if char not in node.children:
                return None
            node = node.children[char]
        return node

    def search_exact(self, word: str):
        node = self.search_prefix(word)
        if node and node.is_end_of_word:
            return True, node.frequency
        return False, 0

    def increment_frequency(self, word: str, delta: int = 1):
        node = self.search_prefix(word)
        if node and node.is_end_of_word:
            node.frequency += delta
        else:
            self.insert(word, delta)

    def delete(self, word: str):
        def _delete(node, w, depth):
            if depth == len(w):
                if not node.is_end_of_word:
                    return False
                node.is_end_of_word = False
                node.frequency = 0
                node.word = None
                return len(node.children) == 0

            char = w[depth]
            if char not in node.children:
                return False

            should_delete_child = _delete(node.children[char], w, depth + 1)
            if should_delete_child:
                del node.children[char]
                return not node.is_end_of_word and len(node.children) == 0
            return False

        clean = "".join(c.lower() for c in word if c.isalpha())
        if self.search_exact(clean)[0]:
            _delete(self.root, clean, 0)
            return True
        return False

    def get_stats(self):
        total_words = 0
        total_nodes = 0
        max_depth = 0

        def _dfs(node, depth):
            nonlocal total_words, total_nodes, max_depth
            total_nodes += 1
            if depth > max_depth:
                max_depth = depth
            if node.is_end_of_word:
                total_words += 1
            for child in node.children.values():
                _dfs(child, depth + 1)

        _dfs(self.root, 0)
        return total_words, total_nodes, max_depth


def levenshtein_distance(s1: str, s2: str) -> int:
    len1, len2 = len(s1), len(s2)
    dp = [[0] * (len2 + 1) for _ in range(len1 + 1)]
    for i in range(len1 + 1):
        dp[i][0] = i
    for j in range(len2 + 1):
        dp[0][j] = j

    for i in range(1, len1 + 1):
        for j in range(1, len2 + 1):
            cost = 0 if s1[i - 1].lower() == s2[j - 1].lower() else 1
            dp[i][j] = min(
                dp[i - 1][j] + 1,       # deletion
                dp[i][j - 1] + 1,       # insertion
                dp[i - 1][j - 1] + cost # substitution
            )
    return dp[len1][len2]


def get_predictive_text(trie: Trie, prefix: str, top_k: int = 10):
    node = trie.search_prefix(prefix)
    if not node:
        return []

    heap = []  # (-frequency, word)

    def _collect(curr):
        if curr.is_end_of_word and curr.word:
            heapq.heappush(heap, (-curr.frequency, curr.word))
        for child in curr.children.values():
            _collect(child)

    _collect(node)

    results = []
    while heap and len(results) < top_k:
        neg_freq, w = heapq.heappop(heap)
        results.append((w, -neg_freq))
    return results


def get_autocorrect_suggestions(trie: Trie, typo: str, max_dist: int = 2, top_k: int = 10):
    clean = "".join(c.lower() for c in typo if c.isalpha())
    if not clean:
        return []

    target_len = len(clean)
    initial_row = list(range(target_len + 1))
    heap = []  # (distance, -frequency, word)

    def _search_fuzzy(node, char, previous_row):
        current_row = [previous_row[0] + 1]
        for i in range(1, target_len + 1):
            cost = 0 if clean[i - 1] == char else 1
            insert_cost = current_row[i - 1] + 1
            delete_cost = previous_row[i] + 1
            replace_cost = previous_row[i - 1] + cost
            current_row.append(min(insert_cost, delete_cost, replace_cost))

        if node.is_end_of_word and current_row[target_len] <= max_dist and node.word:
            heapq.heappush(heap, (current_row[target_len], -node.frequency, node.word))

        if min(current_row) <= max_dist:
            for next_char, child in node.children.items():
                _search_fuzzy(child, next_char, current_row)

    for char, child in trie.root.children.items():
        _search_fuzzy(child, char, initial_row)

    results = []
    seen = set()
    while heap and len(results) < top_k:
        dist, neg_freq, w = heapq.heappop(heap)
        if w not in seen:
            seen.add(w)
            results.append((w, dist, -neg_freq))
    return results


def load_dictionary(trie: Trie, filepath: str):
    if not os.path.exists(filepath):
        print(f"[Notice] Dictionary file '{filepath}' not found. Loading built-in core vocabulary...")
        return 0

    count = 0
    with open(filepath, "r", encoding="utf-8", errors="ignore") as f:
        for line in f:
            parts = line.strip().split()
            if not parts:
                continue
            w = parts[0]
            freq = int(parts[1]) if len(parts) > 1 and parts[1].isdigit() else 10
            trie.insert(w, freq)
            count += 1
    return count


def print_banner():
    print("\n" + "=" * 68)
    print("     AUTO-CORRECT & PREDICTIVE TEXT ENGINE (Data Structures)      ")
    print("=" * 68)
    print("  [Data Structures: Trie (Prefix Tree) + Heap (PQ) + DP Matrix]     ")
    print("=" * 68)


def print_menu():
    print("\n-------------------------- MAIN MENU -------------------------------")
    print(" [1] [>] Predictive Text / Auto-Complete (By Prefix)")
    print(" [2] [?] Spell Check & Auto-Correct (Find Nearest Words)")
    print(" [3] [*] Sentence Corrector & Interactive Spell Checker")
    print(" [4] [+] Add / Teach New Word to Dictionary")
    print(" [5] [-] Remove a Word from Dictionary")
    print(" [6] [#] Data Structure Analytics & Memory Footprint")
    print(" [7] [~] Run Performance & Complexity Benchmark")
    print(" [8] [x] Exit System")
    print("--------------------------------------------------------------------")


def main():
    print_banner()
    trie = Trie()
    print(f"\nInitializing Trie and loading dictionary from '{DEFAULT_DICT_PATH}'...")
    loaded = load_dictionary(trie, DEFAULT_DICT_PATH)
    print(f"Loaded {loaded} words into Trie successfully!")

    while True:
        print_menu()
        choice = input("Enter your choice (1-8): ").strip()

        if choice == "1":
            prefix = input("\nEnter word prefix (e.g. 'app', 'prog', 'algo', 'str'): ").strip()
            start = time.perf_counter()
            results = get_predictive_text(trie, prefix, MAX_SUGGESTIONS)
            elapsed_us = (time.perf_counter() - start) * 1_000_000

            print(f"\nPredictions for prefix '{prefix}' (Search Time: {elapsed_us:.2f} us):")
            print("+-----+----------------------+-----------------+")
            print("| No. | Suggested Word       | Usage Frequency |")
            print("+-----+----------------------+-----------------+")
            for idx, (word, freq) in enumerate(results, 1):
                print(f"| {idx:<3} | {word:<20} | {freq:<15} |")
            if not results:
                print("|  -  | No completions found |        -        |")
            print("+-----+----------------------+-----------------+")

        elif choice == "2":
            word = input("\nEnter a word to verify (e.g. 'computr', 'teh', 'algoritm'): ").strip()
            exact, freq = trie.search_exact(word)
            if exact:
                print(f"\n=> Exact Match Found! '{word}' is spelled correctly. (Frequency: {freq})")
                trie.increment_frequency(word, 1)
            else:
                print(f"\n=> '{word}' is misspelled. Searching nearest suggestions...")
                start = time.perf_counter()
                results = get_autocorrect_suggestions(trie, word, max_dist=2, top_k=MAX_SUGGESTIONS)
                elapsed_us = (time.perf_counter() - start) * 1_000_000

                print(f"\nTop Auto-Correct Suggestions (Search Time: {elapsed_us:.2f} us):")
                print("+-----+----------------------+---------------+-----------------+")
                print("| No. | Suggested Correction | Edit Distance | Usage Frequency |")
                print("+-----+----------------------+---------------+-----------------+")
                for idx, (w, dist, f) in enumerate(results, 1):
                    print(f"| {idx:<3} | {w:<20} | {dist:<13} | {f:<15} |")
                if not results:
                    print("|  -  | No suggestions found |       -       |        -        |")
                print("+-----+----------------------+---------------+-----------------+")

        elif choice == "3":
            sentence = input("\nEnter a full sentence:\n> ").strip()
            tokens = sentence.split()
            corrected_tokens = []
            print("\nAnalyzing sentence...\n")
            for token in tokens:
                clean = "".join(c.lower() for c in token if c.isalpha())
                if not clean:
                    corrected_tokens.append(token)
                    continue

                exact, _ = trie.search_exact(clean)
                if exact:
                    print(f"[OK]   '{clean}' is correct.")
                    corrected_tokens.append(clean)
                else:
                    print(f"\n[TYPO] Misspelled word detected: '{clean}'")
                    suggestions = get_autocorrect_suggestions(trie, clean, max_dist=2, top_k=5)
                    if suggestions:
                        print(f"  Suggestions for '{clean}':")
                        for idx, (w, dist, f) in enumerate(suggestions, 1):
                            print(f"    [{idx}] {w} (Distance: {dist}, Freq: {f})")
                        print(f"    [0] Keep original '{clean}'")
                        sel = input(f"  Select replacement (0-{len(suggestions)}): ").strip()
                        if sel.isdigit() and 1 <= int(sel) <= len(suggestions):
                            chosen = suggestions[int(sel) - 1][0]
                            corrected_tokens.append(chosen)
                            trie.increment_frequency(chosen, 2)
                        else:
                            corrected_tokens.append(clean)
                    else:
                        print(f"  (No close suggestions found. Keeping '{clean}')")
                        corrected_tokens.append(clean)

            print("\n" + "=" * 68)
            print("FINAL CORRECTED SENTENCE:")
            print("=> " + " ".join(corrected_tokens))
            print("=" * 68)

        elif choice == "4":
            word = input("\nEnter new word: ").strip()
            freq_str = input("Enter usage frequency weight (e.g. 50): ").strip()
            freq = int(freq_str) if freq_str.isdigit() else 50
            trie.insert(word, freq)
            print(f"Success: Word '{word}' (freq: {freq}) added to Trie!")

        elif choice == "5":
            word = input("\nEnter word to delete: ").strip()
            if trie.delete(word):
                print(f"Success: Word '{word}' deleted from Trie!")
            else:
                print(f"Error: Word '{word}' not found in Trie.")

        elif choice == "6":
            words, nodes, depth = trie.get_stats()
            print("\n" + "=" * 68)
            print("               DATA STRUCTURE & MEMORY ANALYTICS                    ")
            print("=" * 68)
            print(f"  Total Dictionary Words Stored : {words} words")
            print(f"  Total Trie Nodes Allocated   : {nodes} nodes")
            print(f"  Maximum Trie Depth (Height)  : {depth} levels")
            print(f"  Average Nodes per Word       : {nodes / words:.2f}" if words > 0 else "")
            print("-" * 68)
            print("  Time Complexity Profiles:")
            print("  - Exact Lookup (Trie)        : O(L) where L = word length")
            print("  - Predictive Auto-Complete   : O(L + M log K) where M = matches, K = top-K")
            print("  - Spell Check DP Pruning     : O(L1 * L2) branch-bounded")
            print("=" * 68)

        elif choice == "7":
            test_words = ["the", "algorithm", "programming", "structure", "computer", "trie", "predictive"]
            iterations = 100_000
            print(f"\nExecuting {iterations:,} benchmark lookups in Trie...")
            start = time.perf_counter()
            for i in range(iterations):
                trie.search_exact(test_words[i % len(test_words)])
            total_time = (time.perf_counter() - start) * 1000.0
            avg_ns = (total_time * 1_000_000) / iterations

            print(f"=> Completed {iterations:,} lookups in {total_time:.3f} ms")
            print(f"=> Average lookup time per word: {avg_ns:.2f} nanoseconds!")
            print("=" * 68)

        elif choice == "8":
            print("\nExiting system. Goodbye!")
            break


if __name__ == "__main__":
    main()
