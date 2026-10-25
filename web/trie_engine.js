/**
 * ================================================================================
 * TRIE & PREDICTIVE TEXT ENGINE (Client-side Data Structures in ES6)
 * ================================================================================
 * Core Data Structures:
 * 1. Trie (Prefix Tree) with 26-way / dynamic character branching
 * 2. Binary Max-Heap for Top-K frequency-weighted priority ranking
 * 3. Dynamic Programming Levenshtein Distance & Trie Branch Pruning
 * ================================================================================
 */

class TrieNode {
    constructor() {
        this.children = new Map();
        this.isEndOfWord = false;
        this.frequency = 0;
        this.word = null;
    }
}

class MaxHeap {
    constructor() {
        this.heap = [];
    }

    push(item) {
        this.heap.push(item);
        this._siftUp(this.heap.length - 1);
    }

    pop() {
        if (this.heap.length === 0) return null;
        const top = this.heap[0];
        const last = this.heap.pop();
        if (this.heap.length > 0) {
            this.heap[0] = last;
            this._siftDown(0);
        }
        return top;
    }

    size() {
        return this.heap.length;
    }

    _compare(a, b) {
        // Primary: lower edit distance is better (if present)
        if (a.distance !== undefined && b.distance !== undefined) {
            if (a.distance !== b.distance) {
                return b.distance - a.distance; // higher distance has lower priority
            }
        }
        // Secondary: higher frequency is better
        return a.frequency - b.frequency;
    }

    _siftUp(index) {
        let parent = Math.floor((index - 1) / 2);
        while (index > 0 && this._compare(this.heap[index], this.heap[parent]) > 0) {
            [this.heap[index], this.heap[parent]] = [this.heap[parent], this.heap[index]];
            index = parent;
            parent = Math.floor((index - 1) / 2);
        }
    }

    _siftDown(index) {
        const length = this.heap.length;
        while (true) {
            let largest = index;
            const left = 2 * index + 1;
            const right = 2 * index + 2;

            if (left < length && this._compare(this.heap[left], this.heap[largest]) > 0) {
                largest = left;
            }
            if (right < length && this._compare(this.heap[right], this.heap[largest]) > 0) {
                largest = right;
            }
            if (largest === index) break;

            [this.heap[index], this.heap[largest]] = [this.heap[largest], this.heap[index]];
            index = largest;
        }
    }
}

class TrieEngine {
    constructor() {
        this.root = new TrieNode();
        this.totalWords = 0;
        this.totalNodes = 1;
    }

    _cleanWord(word) {
        if (!word) return "";
        return word.toLowerCase().replace(/[^a-z]/g, "");
    }

    insert(rawWord, frequency = 100) {
        const word = this._cleanWord(rawWord);
        if (!word) return false;

        let node = this.root;
        for (let i = 0; i < word.length; i++) {
            const char = word[i];
            if (!node.children.has(char)) {
                node.children.set(char, new TrieNode());
                this.totalNodes++;
            }
            node = node.children.get(char);
        }

        if (!node.isEndOfWord) {
            this.totalWords++;
            node.isEndOfWord = true;
            node.frequency = frequency;
            node.word = rawWord.trim();
            return true;
        } else {
            node.frequency += frequency;
            if (rawWord && rawWord[0] === rawWord[0].toUpperCase()) {
                node.word = rawWord.trim();
            }
            return false;
        }
    }

    searchExact(rawWord) {
        const word = this._cleanWord(rawWord);
        if (!word) return { found: false, frequency: 0 };

        let node = this.root;
        for (let i = 0; i < word.length; i++) {
            const char = word[i];
            if (!node.children.has(char)) {
                return { found: false, frequency: 0 };
            }
            node = node.children.get(char);
        }

        return {
            found: node.isEndOfWord,
            frequency: node.isEndOfWord ? node.frequency : 0,
            word: node.word
        };
    }

    findPrefixNode(rawPrefix) {
        const prefix = this._cleanWord(rawPrefix);
        let node = this.root;
        for (let i = 0; i < prefix.length; i++) {
            const char = prefix[i];
            if (!node.children.has(char)) {
                return null;
            }
            node = node.children.get(char);
        }
        return node;
    }

    getCompletions(rawPrefix, maxResults = 5) {
        const startTime = performance.now();
        const prefix = this._cleanWord(rawPrefix);
        if (!prefix) {
            return {
                suggestions: [],
                timeMicroseconds: 0,
                matchedCount: 0,
                prefix: ""
            };
        }

        const prefixNode = this.findPrefixNode(prefix);
        if (!prefixNode) {
            const elapsed = Math.round((performance.now() - startTime) * 1000);
            return {
                suggestions: [],
                timeMicroseconds: elapsed,
                matchedCount: 0,
                prefix
            };
        }

        const heap = new MaxHeap();
        let matchedCount = 0;

        // DFS Traversal to collect all words under prefix
        const dfs = (node) => {
            if (node.isEndOfWord) {
                heap.push({
                    word: node.word,
                    frequency: node.frequency,
                    isExactMatch: node.word === prefix
                });
                matchedCount++;
            }
            for (const [char, child] of node.children.entries()) {
                dfs(child);
            }
        };

        dfs(prefixNode);

        // Extract top-K from Heap
        const suggestions = [];
        for (let i = 0; i < maxResults && heap.size() > 0; i++) {
            suggestions.push(heap.pop());
        }

        const elapsed = Math.round((performance.now() - startTime) * 1000);
        return {
            suggestions,
            timeMicroseconds: elapsed,
            matchedCount,
            prefix
        };
    }

    incrementFrequency(rawWord, delta = 50) {
        const word = this._cleanWord(rawWord);
        if (!word) return;

        let node = this.root;
        for (let i = 0; i < word.length; i++) {
            const char = word[i];
            if (!node.children.has(char)) {
                this.insert(word, delta);
                return;
            }
            node = node.children.get(char);
        }

        if (node.isEndOfWord) {
            node.frequency += delta;
        } else {
            node.isEndOfWord = true;
            node.frequency = delta;
            node.word = word;
            this.totalWords++;
        }
    }

    deleteWord(rawWord) {
        const word = this._cleanWord(rawWord);
        if (!word) return false;

        const _delete = (node, depth) => {
            if (depth === word.length) {
                if (!node.isEndOfWord) return false;
                node.isEndOfWord = false;
                node.frequency = 0;
                node.word = null;
                this.totalWords--;
                return node.children.size === 0;
            }

            const char = word[depth];
            if (!node.children.has(char)) return false;

            const childNode = node.children.get(char);
            const shouldDeleteChild = _delete(childNode, depth + 1);

            if (shouldDeleteChild) {
                node.children.delete(char);
                this.totalNodes--;
                return !node.isEndOfWord && node.children.size === 0;
            }
            return false;
        };

        return _delete(this.root, 0);
    }

    getStats() {
        let maxDepth = 0;
        const dfs = (node, depth) => {
            if (depth > maxDepth) maxDepth = depth;
            for (const child of node.children.values()) {
                dfs(child, depth + 1);
            }
        };
        dfs(this.root, 0);

        // Memory estimation: ~48 bytes per TrieNode + map entries
        const estimatedMemoryBytes = this.totalNodes * 64 + this.totalWords * 32;

        return {
            totalWords: this.totalWords,
            totalNodes: this.totalNodes,
            maxDepth,
            estimatedMemoryKB: (estimatedMemoryBytes / 1024).toFixed(2),
            estimatedMemoryMB: (estimatedMemoryBytes / (1024 * 1024)).toFixed(3)
        };
    }

    /**
     * Bounded Levenshtein Search via Trie Branch Pruning
     * Explores Trie nodes while maintaining a DP distance vector row.
     */
    getSpellCorrections(rawWord, maxDistance = 2, maxResults = 5) {
        const startTime = performance.now();
        const word = this._cleanWord(rawWord);
        if (!word) {
            return {
                suggestions: [],
                timeMicroseconds: 0,
                isCorrect: true
            };
        }

        // Exact match check first
        const exact = this.searchExact(word);
        if (exact.found) {
            const elapsed = Math.round((performance.now() - startTime) * 1000);
            return {
                suggestions: [{ word: exact.word, frequency: exact.frequency, distance: 0, confidence: 100 }],
                timeMicroseconds: elapsed,
                isCorrect: true
            };
        }

        const targetLen = word.length;
        const initialRow = [];
        for (let i = 0; i <= targetLen; i++) {
            initialRow[i] = i;
        }

        const candidates = [];

        // Recursive Trie DP Branch Search
        const searchRecursive = (node, letter, previousRow) => {
            const currentRow = [previousRow[0] + 1];

            for (let i = 1; i <= targetLen; i++) {
                const insertCost = currentRow[i - 1] + 1;
                const deleteCost = previousRow[i] + 1;
                const replaceCost = previousRow[i - 1] + (word[i - 1] === letter ? 0 : 1);

                currentRow[i] = Math.min(insertCost, deleteCost, replaceCost);
            }

            // If current node forms a word and distance is within limit
            if (currentRow[targetLen] <= maxDistance && node.isEndOfWord) {
                const dist = currentRow[targetLen];
                candidates.push({
                    word: node.word,
                    frequency: node.frequency,
                    distance: dist
                });
            }

            // Pruning: if the minimum element in currentRow exceeds maxDistance, prune branch
            const minRowVal = Math.min(...currentRow);
            if (minRowVal <= maxDistance) {
                for (const [nextChar, childNode] of node.children.entries()) {
                    searchRecursive(childNode, nextChar, currentRow);
                }
            }
        };

        for (const [char, childNode] of this.root.children.entries()) {
            searchRecursive(childNode, char, initialRow);
        }

        // Sort candidates by:
        // 1. Distance ascending
        // 2. Frequency descending
        // 3. Length difference ascending
        candidates.sort((a, b) => {
            if (a.distance !== b.distance) return a.distance - b.distance;
            if (b.frequency !== a.frequency) return b.frequency - a.frequency;
            return Math.abs(a.word.length - word.length) - Math.abs(b.word.length - word.length);
        });

        // Add confidence score (0 - 100%)
        const maxFreq = candidates.length > 0 ? Math.max(...candidates.map(c => c.frequency), 1) : 1;
        const suggestions = candidates.slice(0, maxResults).map(c => {
            const distPenalty = c.distance === 0 ? 100 : c.distance === 1 ? 80 : 50;
            const freqBonus = Math.min(20, Math.round((c.frequency / maxFreq) * 20));
            const confidence = Math.min(99, distPenalty + freqBonus);
            return {
                ...c,
                confidence
            };
        });

        const elapsed = Math.round((performance.now() - startTime) * 1000);
        return {
            suggestions,
            timeMicroseconds: elapsed,
            isCorrect: false
        };
    }

    /**
     * Build dynamic hierarchy tree for visual D3 / SVG graph representation
     */
    getVisualSubtree(rawPrefix = "", maxDepth = 4, maxChildrenPerNode = 6) {
        const prefix = this._cleanWord(rawPrefix);

        const buildNode = (node, charLabel, depth, fullWord) => {
            const result = {
                name: charLabel || "ROOT",
                word: node.word || fullWord || "",
                frequency: node.frequency,
                isEnd: node.isEndOfWord,
                childrenCount: node.children.size,
                children: []
            };

            if (depth < maxDepth) {
                let count = 0;
                // Sort children by frequency or alphabetical
                const sortedChildren = Array.from(node.children.entries()).sort((a, b) => {
                    return (b[1].frequency || 0) - (a[1].frequency || 0);
                });

                for (const [ch, child] of sortedChildren) {
                    if (count >= maxChildrenPerNode) break;
                    result.children.push(buildNode(child, ch, depth + 1, (fullWord || "") + ch));
                    count++;
                }
            }

            return result;
        };

        // If prefix is provided, find prefix node, else start at root
        let rootToRender = this.root;
        let startWord = "";
        if (prefix.length > 0) {
            const found = this.findPrefixNode(prefix);
            if (found) {
                rootToRender = found;
                startWord = prefix;
            }
        }

        return {
            tree: buildNode(rootToRender, prefix ? prefix : "ROOT", 0, startWord),
            activePrefix: prefix
        };
    }

    /**
     * Compute full Levenshtein Matrix with back-pointer path trace
     */
    static getDetailedLevenshteinMatrix(s1, s2) {
        const str1 = (s1 || "").toLowerCase();
        const str2 = (s2 || "").toLowerCase();
        const m = str1.length;
        const n = str2.length;

        const dp = Array.from({ length: m + 1 }, () => Array(n + 1).fill(0));
        const ops = Array.from({ length: m + 1 }, () => Array(n + 1).fill(""));

        for (let i = 0; i <= m; i++) {
            dp[i][0] = i;
            ops[i][0] = i === 0 ? "START" : "DELETE";
        }
        for (let j = 0; j <= n; j++) {
            dp[0][j] = j;
            if (j > 0) ops[0][j] = "INSERT";
        }

        for (let i = 1; i <= m; i++) {
            for (let j = 1; j <= n; j++) {
                if (str1[i - 1] === str2[j - 1]) {
                    dp[i][j] = dp[i - 1][j - 1];
                    ops[i][j] = "MATCH";
                } else {
                    const deleteCost = dp[i - 1][j] + 1;
                    const insertCost = dp[i][j - 1] + 1;
                    const replaceCost = dp[i - 1][j - 1] + 1;

                    const minCost = Math.min(deleteCost, insertCost, replaceCost);
                    dp[i][j] = minCost;

                    if (minCost === replaceCost) {
                        ops[i][j] = "REPLACE";
                    } else if (minCost === insertCost) {
                        ops[i][j] = "INSERT";
                    } else {
                        ops[i][j] = "DELETE";
                    }
                }
            }
        }

        // Traceback optimal path
        const path = [];
        let r = m;
        let c = n;
        while (r > 0 || c > 0) {
            path.push({ row: r, col: c, op: ops[r][c], cost: dp[r][c] });
            if (r > 0 && c > 0 && (ops[r][c] === "MATCH" || ops[r][c] === "REPLACE")) {
                r--;
                c--;
            } else if (c > 0 && ops[r][c] === "INSERT") {
                c--;
            } else if (r > 0 && ops[r][c] === "DELETE") {
                r--;
            } else {
                break;
            }
        }
        path.push({ row: 0, col: 0, op: "START", cost: 0 });
        path.reverse();

        return {
            s1: str1,
            s2: str2,
            matrix: dp,
            operations: ops,
            distance: dp[m][n],
            path
        };
    }
}

// Attach to window
if (typeof window !== "undefined") {
    window.TrieEngine = TrieEngine;
    window.TrieNode = TrieNode;
    window.MaxHeap = MaxHeap;
}
