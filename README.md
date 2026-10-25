# ⚡ Auto-Correct & Predictive Text Engine in C

A production-grade, highly efficient **Auto-Correct and Predictive Text System** built in **C** using advanced Data Structures & Algorithms.

---

## 🌟 Key Features

1. **⚡ Instant Prefix-based Predictive Text / Auto-Complete**:
   - Types any prefix (e.g. `app`, `prog`, `algo`) and retrieves the top-ranked completions in **sub-microsecond** time.
   - Predictions are dynamically ranked by usage frequency using a **Priority Queue (Heap)**.
2. **🔍 Intelligent Spell-Checker & Auto-Correct**:
   - Flags misspelled words (e.g., `computr`, `teh`, `algoritm`).
   - Uses **Levenshtein Distance Dynamic Programming** bounded by Trie branch pruning to recommend nearest valid words.
3. **📝 Interactive Sentence Processor**:
   - Stream-processes entire sentences.
   - Highlights typos, prompts for replacement choices, and outputs a clean, corrected sentence.
4. **🧠 Dynamic Vocabulary Learning**:
   - Frequently typed and selected words automatically increase their ranking weight.
   - Add new custom words or remove words on the fly.
5. **📊 Data Structure Analytics & Microsecond Benchmarks**:
   - Displays real-time metrics: node count, Trie depth/height, heap memory usage (KB/MB), and lookup benchmarks.

---

## 📂 Project Architecture

```
AutoCorrect_PredictiveText/
├── include/
│   ├── trie.h          # Trie definition, node creation, insert, search, prefix traversal
│   ├── heap.h          # Priority queue / Min-Max Heap for ranking Top-K predictions
│   ├── spellcheck.h    # Levenshtein distance & Trie-based approximate string matching
│   └── dictionary.h    # Dictionary loader, file parser, and persistence
├── src/
│   ├── trie.c          # Trie logic implementation & memory cleanup
│   ├── heap.c          # Heap operations (push, pop, top-k extraction)
│   ├── spellcheck.c    # Dynamic programming edit distance & fuzzy Trie traversal
│   ├── dictionary.c    # File I/O for loading 10,000+ words with frequencies
│   └── main.c          # Interactive CLI with terminal UI & performance benchmark
├── data/
│   └── dictionary.txt  # Comprehensive word list with baseline word frequencies
├── single_file_version/
│   └── autocorrect_standalone.c # Standalone single-file version for quick submission / online IDEs
├── build.bat           # Windows 1-click compile script
├── run.bat             # 1-click execution script
├── Makefile            # Standard cross-platform Makefile
├── PROJECT_REPORT.md   # Complete academic submission report & Viva Q&A
└── README.md           # This file
```

---

## 🛠️ How to Launch and Run

### 🌐 Option 1: Interactive Modern Web Playground (Recommended)
Launch the rich visual playground with real-time predictive text editor, interactive Trie tree visualizer, Levenshtein DP matrix inspector, and batch paragraph spell-fixer:

- **1-Click Launch (Windows):** Double-click [`start_web.bat`](file:///c:/Users/hp/OneDrive/Desktop/PYTHON/AutoCorrect_PredictiveText/start_web.bat)
- **Via Python:**
  ```bash
  python start_web.py
  ```
- Open browser at `http://localhost:8000/index.html` (or open [`web/index.html`](file:///c:/Users/hp/OneDrive/Desktop/PYTHON/AutoCorrect_PredictiveText/web/index.html) directly).

### 🐍 Option 2: Python Interactive Demo & Reference
Run the full Python implementation in the terminal:
```bash
python autocorrect_demo.py
```

### ⚡ Option 3: Modular C Build
Using GCC (MinGW / Linux / macOS):
```bash
gcc -Wall -O2 -Iinclude src/trie.c src/heap.c src/spellcheck.c src/dictionary.c src/main.c -o autocorrect.exe
./autocorrect.exe
```
Or use the Makefile:
```bash
make
./autocorrect
```

### 📦 Option 4: Single-File C Standalone Version
If your submission requires a single `.c` file:
```bash
gcc -Wall -O2 single_file_version/autocorrect_standalone.c -o autocorrect_standalone.exe
./autocorrect_standalone.exe
```

---

## 📊 Big-O Complexity Summary

| Operation | Data Structure / Algorithm | Time Complexity | Space Complexity |
| :--- | :--- | :--- | :--- |
| **Word Insertion** | Trie | $\mathcal{O}(L)$ | $\mathcal{O}(L \times \Sigma)$ |
| **Exact Word Search** | Trie | $\mathcal{O}(L)$ | $\mathcal{O}(1)$ |
| **Prefix Auto-Complete** | Trie Traversal + Heap | $\mathcal{O}(L + M \log K)$ | $\mathcal{O}(M)$ |
| **Levenshtein Distance** | Dynamic Programming | $\mathcal{O}(L_1 \times L_2)$ | $\mathcal{O}(L_1 \times L_2)$ or $\mathcal{O}(L_2)$ |
| **Auto-Correct Search** | Pruned Trie + DP Row | $\mathcal{O}(\text{pruned nodes})$ | $\mathcal{O}(L_{\max})$ |

*Where $L$ = word length, $\Sigma = 26$ (alphabet size), $M$ = matched prefix words, $K$ = top suggestions, $N$ = total dictionary words.*
