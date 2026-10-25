/**
 * ================================================================================
 * AUTOCORRECT PRO (Modern SaaS Dashboard Application Controller)
 * With WhatsApp Shortcuts Expander, Capitalization & DS Engine
 * ================================================================================
 */

class AutoCorrectApp {
    constructor() {
        this.trie = new TrieEngine();
        this.keystrokesSaved = 0;
        this.predictionsCount = 1450;
        this.typoFixCount = 85;
        this.currentSuggestions = [];
        this.selectedSuggestionIndex = 0;
        this.bestGhostCompletion = "";
        this.activeWord = "";
        this.activeWordStart = 0;
        this.activeWordEnd = 0;

        // WhatsApp slang dictionary
        this.whatsappSlang = window.WHATSAPP_SLANG || {
            'idk': "I don't know",
            'brb': "Be right back",
            'tbh': "To be honest",
            'imo': "In my opinion",
            'imho': "In my humble opinion",
            'btw': "By the way",
            'rn': "Right now",
            'np': "No problem",
            'ttyl': "Talk to you later",
            'tysm': "Thank you so much",
            'ty': "Thank you",
            'thx': "Thank you",
            'pls': "Please",
            'plz': "Please",
            'wbu': "What about you?",
            'hru': "How are you?",
            'gm': "Good morning",
            'gn': "Good night",
            'tc': "Take care",
            'afaik': "As far as I know",
            'ofc': "Of course",
            'nvm': "Never mind",
            'wyd': "What are you doing?",
            'wfh': "Work from home",
            'omg': "Oh my god",
            'gtg': "Got to go",
            'fyi': "For your information",
            'asap': "As soon as possible",
            'dm': "Direct message",
            'aka': "Also known as",
            'diy': "Do it yourself"
        };

        this.recentFixes = [
            { typo: "computr", fix: "computer", dist: 1, time: "Just now" },
            { typo: "teh", fix: "the", dist: 1, time: "1m ago" },
            { typo: "algoritm", fix: "algorithm", dist: 1, time: "3m ago" },
            { typo: "idk", fix: "I don't know", dist: 0, time: "5m ago" }
        ];

        // SVG Pan & Zoom state
        this.zoom = 1;
        this.panX = 40;
        this.panY = 60;
        this.isPanning = false;
        this.startX = 0;
        this.startY = 0;

        this.init();
    }

    init() {
        this.loadDictionary();
        this.setupDOMReferences();
        this.setupEventListeners();
        this.setupSidebarNav();
        this.renderSlangGrid();
        this.updateTelemetry();
        this.renderRecentFixes();
        this.renderTrieTree("how");
        this.computeDPMatrix();
        this.populateDictTable();
    }

    loadDictionary() {
        if (window.DEFAULT_DICTIONARY && Array.isArray(window.DEFAULT_DICTIONARY)) {
            for (const [word, freq] of window.DEFAULT_DICTIONARY) {
                this.trie.insert(word, freq);
            }
        }
    }

    setupDOMReferences() {
        this.editor = document.getElementById("mainEditor");
        this.ghostOverlay = document.getElementById("ghostOverlay");
        this.suggestionsBar = document.getElementById("suggestionsBar");
        this.heapList = document.getElementById("heapRankingList");
        this.typoAlertBox = document.getElementById("typoAlertBox");
        this.typoWordSpan = document.getElementById("typoWordSpan");
        this.typoSuggestionsList = document.getElementById("typoSuggestionsList");

        this.breadcrumbCurrent = document.getElementById("breadcrumbCurrent");
        this.pageMainHeading = document.getElementById("pageMainHeading");
        this.pageSubHeading = document.getElementById("pageSubHeading");

        this.dashTotalWords = document.getElementById("dashTotalWords");
        this.dashPredictionsCount = document.getElementById("dashPredictionsCount");
        this.dashTypoFixCount = document.getElementById("dashTypoFixCount");
        this.dashLatency = document.getElementById("dashLatency");

        this.triePrefixInput = document.getElementById("triePrefixInput");
        this.btnRenderTrie = document.getElementById("btnRenderTrie");
        this.btnResetTrieZoom = document.getElementById("btnResetTrieZoom");
        this.trieSvg = document.getElementById("trieSvg");
        this.trieCanvasWrapper = document.getElementById("trieCanvasWrapper");

        this.matrixSourceInput = document.getElementById("matrixSourceInput");
        this.matrixTargetInput = document.getElementById("matrixTargetInput");
        this.btnComputeMatrix = document.getElementById("btnComputeMatrix");
        this.matrixTableWrapper = document.getElementById("matrixTableWrapper");
        this.matrixDistanceVal = document.getElementById("matrixDistanceVal");

        this.batchInputText = document.getElementById("batchInputText");
        this.btnRunBatchAnalyze = document.getElementById("btnRunBatchAnalyze");
        this.btnBatchReset = document.getElementById("btnBatchReset");
        this.batchDiffContent = document.getElementById("batchDiffContent");
        this.btnCopyBatch = document.getElementById("btnCopyBatch");

        this.newWordInput = document.getElementById("newWordInput");
        this.newWordFreq = document.getElementById("newWordFreq");
        this.btnAddWord = document.getElementById("btnAddWord");
        this.dictFilterInput = document.getElementById("dictFilterInput");
        this.dictTableBody = document.getElementById("dictTableBody");
        this.dictTableCount = document.getElementById("dictTableCount");

        this.sidebarSearchInput = document.getElementById("sidebarSearchInput");
        this.globalHeaderSearch = document.getElementById("globalHeaderSearch");
        this.allSlangGrid = document.getElementById("allSlangGrid");
        this.recentFixesTableBody = document.getElementById("recentFixesTableBody");
        this.toastContainer = document.getElementById("toastContainer");
    }

    setupSidebarNav() {
        const navButtons = document.querySelectorAll(".sidebar-nav-item");
        const navMeta = {
            "tab-dashboard": {
                bread: "Products Management",
                heading: "Products Management",
                sub: "Manage vocabulary, track prediction performance, and test live WhatsApp shortcuts."
            },
            "tab-slang": {
                bread: "WhatsApp Slang Expander",
                heading: "WhatsApp & Chat Slang Hub",
                sub: "36+ Instant chat abbreviations that auto-expand into full sentences."
            },
            "tab-trie": {
                bread: "Trie Visualizer",
                heading: "Trie (Prefix Tree) Hierarchy Explorer",
                sub: "Interactive 26-way prefix tree rendering active character branches."
            },
            "tab-matrix": {
                bread: "Levenshtein Matrix",
                heading: "Levenshtein DP Distance Matrix",
                sub: "Cell-by-cell dynamic programming matrix and optimal alignment traceback path."
            },
            "tab-batch": {
                bread: "Batch Spell-Fixer",
                heading: "Batch Sentence & Paragraph Spell-Fixer",
                sub: "Stream-processes text, resolves typos, and expands shortcuts simultaneously."
            },
            "tab-dict": {
                bread: "Vocabulary Base",
                heading: "Vocabulary & Knowledge Base",
                sub: "Explore and manage 10,652 words with dynamic frequency ranking."
            },
            "tab-analytics": {
                bread: "Big-O Analytics",
                heading: "Data Structures & Complexity Analytics",
                sub: "Theoretical proofs and runtime Big-O benchmarks for Trie, Heap, and DP."
            }
        };

        navButtons.forEach(btn => {
            btn.addEventListener("click", () => {
                const tabId = btn.dataset.tab;
                this.showTab(tabId, navMeta);
            });
        });
    }

    showTab(tabId, navMeta) {
        navMeta = navMeta || {
            "tab-dashboard": { bread: "Products Management", heading: "Products Management", sub: "Manage vocabulary and predictions." },
            "tab-slang": { bread: "WhatsApp Slang", heading: "WhatsApp Slang Hub", sub: "Chat abbreviations." },
            "tab-trie": { bread: "Trie Visualizer", heading: "Trie Visualizer", sub: "Explore prefix tree." },
            "tab-matrix": { bread: "Levenshtein Matrix", heading: "Levenshtein DP Matrix", sub: "Traceback matrix." },
            "tab-batch": { bread: "Batch Spell-Fixer", heading: "Batch Sentence Fixer", sub: "Fix paragraphs." },
            "tab-dict": { bread: "Vocabulary Base", heading: "Vocabulary Manager", sub: "Manage dictionary." },
            "tab-analytics": { bread: "Big-O Analytics", heading: "Complexity Analytics", sub: "Big-O proofs." }
        };

        document.querySelectorAll(".sidebar-nav-item").forEach(b => {
            b.classList.toggle("active", b.dataset.tab === tabId);
        });
        document.querySelectorAll(".tab-pane").forEach(p => {
            p.classList.toggle("active", p.id === tabId);
        });

        if (navMeta[tabId]) {
            if (this.breadcrumbCurrent) this.breadcrumbCurrent.textContent = navMeta[tabId].bread;
            if (this.pageMainHeading) this.pageMainHeading.textContent = navMeta[tabId].heading;
            if (this.pageSubHeading) this.pageSubHeading.textContent = navMeta[tabId].sub;
        }

        if (tabId === "tab-trie") {
            this.renderTrieTree(this.triePrefixInput.value || "how");
        }
    }

    setupEventListeners() {
        // Main Editor events
        this.editor.addEventListener("input", () => this.handleEditorInput());
        this.editor.addEventListener("keydown", (e) => this.handleEditorKeydown(e));
        this.editor.addEventListener("click", () => this.handleEditorInput());
        this.editor.addEventListener("keyup", (e) => {
            if (["ArrowLeft", "ArrowRight"].includes(e.key)) {
                this.handleEditorInput();
            }
        });

        // Searches
        const handleSearch = (input) => {
            if (!input) return;
            input.addEventListener("keydown", (e) => {
                if (e.key === "Enter") {
                    const val = input.value.trim();
                    if (val) {
                        this.setSampleText(val);
                        this.showTab("tab-dashboard");
                        this.showToast(`Loaded "${val}" into editor!`, "info");
                    }
                }
            });
        };
        handleSearch(this.sidebarSearchInput);
        handleSearch(this.globalHeaderSearch);

        // Trie Visualizer events
        this.btnRenderTrie.addEventListener("click", () => {
            this.renderTrieTree(this.triePrefixInput.value.trim());
        });
        this.triePrefixInput.addEventListener("keydown", (e) => {
            if (e.key === "Enter") {
                this.renderTrieTree(this.triePrefixInput.value.trim());
            }
        });
        this.btnResetTrieZoom.addEventListener("click", () => {
            this.zoom = 1;
            this.panX = 40;
            this.panY = 60;
            this.renderTrieTree(this.triePrefixInput.value.trim());
        });

        // Canvas Pan & Zoom
        this.trieCanvasWrapper.addEventListener("mousedown", (e) => {
            this.isPanning = true;
            this.startX = e.clientX - this.panX;
            this.startY = e.clientY - this.panY;
        });
        window.addEventListener("mousemove", (e) => {
            if (!this.isPanning) return;
            this.panX = e.clientX - this.startX;
            this.panY = e.clientY - this.startY;
            const g = this.trieSvg.querySelector("g.viewport");
            if (g) {
                g.setAttribute("transform", `translate(${this.panX}, ${this.panY}) scale(${this.zoom})`);
            }
        });
        window.addEventListener("mouseup", () => {
            this.isPanning = false;
        });
        this.trieCanvasWrapper.addEventListener("wheel", (e) => {
            e.preventDefault();
            const delta = e.deltaY > 0 ? 0.9 : 1.1;
            this.zoom = Math.max(0.3, Math.min(3, this.zoom * delta));
            const g = this.trieSvg.querySelector("g.viewport");
            if (g) {
                g.setAttribute("transform", `translate(${this.panX}, ${this.panY}) scale(${this.zoom})`);
            }
        }, { passive: false });

        // DP Matrix events
        this.btnComputeMatrix.addEventListener("click", () => this.computeDPMatrix());
        this.matrixSourceInput.addEventListener("keydown", (e) => {
            if (e.key === "Enter") this.computeDPMatrix();
        });
        this.matrixTargetInput.addEventListener("keydown", (e) => {
            if (e.key === "Enter") this.computeDPMatrix();
        });

        // Batch Analyzer
        this.btnRunBatchAnalyze.addEventListener("click", () => this.runBatchAnalyze());
        this.btnBatchReset.addEventListener("click", () => {
            this.batchInputText.value = "idk hows India doing. Teh algoritm for computr programing is extremly effishent when using a trie data structur. brb";
            this.runBatchAnalyze();
        });
        this.btnCopyBatch.addEventListener("click", () => {
            const cleanText = this.batchDiffContent.innerText.replace(/[\n\r]+/g, " ");
            navigator.clipboard.writeText(cleanText).then(() => {
                this.showToast("Corrected text copied to clipboard!", "success");
            });
        });

        // Dict Manager events
        this.btnAddWord.addEventListener("click", () => this.handleAddWord());
        this.dictFilterInput.addEventListener("input", () => this.populateDictTable());
    }

    getCurrentWordContext() {
        const text = this.editor.value;
        const cursor = this.editor.selectionStart;

        // Boundary around cursor (including apostrophe)
        let start = cursor;
        while (start > 0 && /[a-zA-Z']/.test(text[start - 1])) {
            start--;
        }

        let end = cursor;
        while (end < text.length && /[a-zA-Z']/.test(text[end])) {
            end++;
        }

        const rawPrefix = text.slice(start, cursor);
        const fullWord = text.slice(start, end);

        // Check if word is start of sentence (at index 0 or after '.', '!', '?', or '\n')
        let isSentenceStart = true;
        if (start > 0) {
            const preceding = text.slice(0, start).trimEnd();
            if (preceding.length > 0) {
                const lastChar = preceding[preceding.length - 1];
                isSentenceStart = ['.', '!', '?', '\n'].includes(lastChar);
            }
        }

        return {
            prefix: rawPrefix,
            fullWord,
            start,
            end,
            cursor,
            isSentenceStart
        };
    }

    handleEditorInput() {
        const ctx = this.getCurrentWordContext();
        const rawPrefix = ctx.prefix;
        this.activeWord = rawPrefix;
        this.activeWordStart = ctx.start;
        this.activeWordEnd = ctx.end;

        if (!rawPrefix || rawPrefix.length === 0) {
            this.clearSuggestions();
            this.hideTypoAlert();
            this.updateGhostOverlay("");
            return;
        }

        const startTime = performance.now();
        const cleanKey = rawPrefix.toLowerCase().replace(/'/g, "");

        // 1. Check if input matches a WhatsApp slang shortcut (e.g. idk -> I don't know)
        let slangExpansion = this.whatsappSlang[cleanKey];

        const compRes = this.trie.getCompletions(rawPrefix, 5);
        const elapsed = Math.round((performance.now() - startTime) * 1000);
        if (this.dashLatency) this.dashLatency.textContent = `${elapsed} µs`;

        let suggestions = compRes.suggestions;

        // If it is an exact slang abbreviation, put it at the very top of predictions!
        if (slangExpansion) {
            const formattedSlang = ctx.isSentenceStart ? (slangExpansion.charAt(0).toUpperCase() + slangExpansion.slice(1)) : slangExpansion;
            suggestions = [
                { word: formattedSlang, frequency: 10000, isSlang: true, rawSlang: rawPrefix },
                ...suggestions.filter(s => s.word.toLowerCase() !== slangExpansion.toLowerCase())
            ];
        }

        this.currentSuggestions = suggestions;
        this.selectedSuggestionIndex = 0;

        this.renderSuggestions(suggestions, rawPrefix, ctx.isSentenceStart);
        this.renderHeapRanking(suggestions);

        if (suggestions.length > 0) {
            this.hideTypoAlert();
            this.updateSelectedGhost(ctx.isSentenceStart);
        } else {
            this.bestGhostCompletion = "";
            this.updateGhostOverlay("");
            this.checkAndShowTypo(ctx.fullWord || rawPrefix, ctx.isSentenceStart);
        }
    }

    updateSelectedGhost(isSentenceStart = false) {
        if (!this.currentSuggestions || this.currentSuggestions.length === 0) {
            this.bestGhostCompletion = "";
            this.updateGhostOverlay("");
            return;
        }

        const selectedItem = this.currentSuggestions[this.selectedSuggestionIndex];
        if (!selectedItem) return;

        const rawPrefix = this.activeWord;
        const formattedWord = this.formatCaseMatch(rawPrefix, selectedItem.word, isSentenceStart);

        if (formattedWord.toLowerCase().startsWith(rawPrefix.toLowerCase()) && formattedWord.length >= rawPrefix.length) {
            this.bestGhostCompletion = formattedWord;
            const remainingSuffix = formattedWord.slice(rawPrefix.length);
            this.updateGhostOverlay(remainingSuffix);
        } else {
            this.bestGhostCompletion = formattedWord;
            this.updateGhostOverlay("");
        }
    }

    handleEditorKeydown(e) {
        const ctx = this.getCurrentWordContext();

        if (this.currentSuggestions && this.currentSuggestions.length > 0) {
            // Down Arrow or Right Arrow -> Next Suggestion
            if (e.key === "ArrowDown" || (e.key === "ArrowRight" && this.editor.selectionStart === this.editor.selectionEnd && this.editor.selectionStart === this.activeWordEnd)) {
                e.preventDefault();
                this.selectedSuggestionIndex = (this.selectedSuggestionIndex + 1) % this.currentSuggestions.length;
                this.renderSuggestions(this.currentSuggestions, this.activeWord, ctx.isSentenceStart);
                this.updateSelectedGhost(ctx.isSentenceStart);
                return;
            }

            // Up Arrow -> Previous Suggestion
            if (e.key === "ArrowUp") {
                e.preventDefault();
                this.selectedSuggestionIndex = (this.selectedSuggestionIndex - 1 + this.currentSuggestions.length) % this.currentSuggestions.length;
                this.renderSuggestions(this.currentSuggestions, this.activeWord, ctx.isSentenceStart);
                this.updateSelectedGhost(ctx.isSentenceStart);
                return;
            }

            // Tab or Enter -> Accept highlighted suggestion
            if (e.key === "Tab" || (e.key === "Enter" && !e.shiftKey)) {
                e.preventDefault();
                const chosen = this.currentSuggestions[this.selectedSuggestionIndex];
                if (chosen) {
                    this.acceptSuggestion(chosen.word);
                    return;
                }
            }

            // Direct Number shortcuts 1-5 (Alt/Ctrl)
            if (["1", "2", "3", "4", "5"].includes(e.key) && (e.ctrlKey || e.altKey)) {
                const index = parseInt(e.key, 10) - 1;
                if (this.currentSuggestions[index]) {
                    e.preventDefault();
                    this.acceptSuggestion(this.currentSuggestions[index].word);
                    return;
                }
            }
        }

        // Space commits word & auto-expands WhatsApp slang, auto-capitalizes 'i' -> 'I' and first letter after '.'
        if (e.key === " ") {
            const cleanKey = ctx.prefix.toLowerCase().replace(/'/g, "");
            
            // 1. WhatsApp slang auto-expansion on space!
            if (this.whatsappSlang[cleanKey]) {
                e.preventDefault();
                const expansion = this.whatsappSlang[cleanKey];
                const formatted = ctx.isSentenceStart ? (expansion.charAt(0).toUpperCase() + expansion.slice(1)) : expansion;
                this.acceptSuggestion(formatted);
                return;
            }

            // 2. Standalone 'i' -> 'I'
            if (ctx.prefix.toLowerCase() === "i") {
                const text = this.editor.value;
                this.editor.value = text.slice(0, ctx.start) + "I" + text.slice(ctx.cursor);
            } 
            // 3. Auto-capitalize first letter after full stop
            else if (ctx.isSentenceStart && ctx.prefix.length > 0 && ctx.prefix[0] === ctx.prefix[0].toLowerCase()) {
                const text = this.editor.value;
                const capitalized = ctx.prefix.charAt(0).toUpperCase() + ctx.prefix.slice(1);
                this.editor.value = text.slice(0, ctx.start) + capitalized + text.slice(ctx.cursor);
            } else if (ctx.prefix && ctx.prefix.length > 2) {
                this.trie.incrementFrequency(ctx.prefix.toLowerCase(), 10);
            }
        }
    }

    formatCaseMatch(template, word, isSentenceStart = false) {
        if (!word) return "";

        // Proper nouns or fixed contractions (e.g. "I'm", "I'll", "India", "Bharat")
        if (word === "I" || word.startsWith("I'") || (word[0] === word[0].toUpperCase() && word.slice(1) === word.slice(1).toLowerCase())) {
            return word;
        }

        // If user typed all uppercase (e.g. "HOWS", "IDK")
        if (template && template === template.toUpperCase() && template.length > 1) {
            return word.toUpperCase();
        }

        // If sentence start or user typed first letter capital
        if (isSentenceStart || (template && template[0] === template[0].toUpperCase())) {
            return word.charAt(0).toUpperCase() + word.slice(1);
        }

        return word;
    }

    acceptSuggestion(rawWord) {
        const text = this.editor.value;
        const ctx = this.getCurrentWordContext();
        const matchedCaseWord = this.formatCaseMatch(ctx.prefix, rawWord, ctx.isSentenceStart);

        const before = text.slice(0, ctx.start);
        const after = text.slice(ctx.end);
        const newText = before + matchedCaseWord + " " + after;

        const savedCount = Math.max(0, matchedCaseWord.length - ctx.prefix.length);
        this.keystrokesSaved += savedCount;
        this.predictionsCount++;

        if (this.dashPredictionsCount) this.dashPredictionsCount.textContent = this.predictionsCount.toLocaleString();

        this.editor.value = newText;
        const newCursorPos = ctx.start + matchedCaseWord.length + 1;
        this.editor.selectionStart = newCursorPos;
        this.editor.selectionEnd = newCursorPos;
        this.editor.focus();

        // Boost frequency in Trie
        this.trie.incrementFrequency(rawWord.toLowerCase(), 30);

        this.handleEditorInput();
        this.showToast(`Completed: "${matchedCaseWord}" (+${savedCount} keys saved)`, "info");
    }

    renderSuggestions(suggestions, prefix, isSentenceStart = false) {
        if (!suggestions || suggestions.length === 0) {
            this.suggestionsBar.innerHTML = `<span style="color: var(--text-dim); font-size: 0.85rem;">No prefix completions for "${prefix}". Check typo suggestions below.</span>`;
            return;
        }

        let html = "";
        suggestions.forEach((item, index) => {
            const isSelected = index === this.selectedSuggestionIndex;
            const hotkey = index + 1;
            const formattedWord = this.formatCaseMatch(prefix, item.word, isSentenceStart);
            const prefixDisplay = formattedWord.slice(0, prefix.length);
            const suffixDisplay = formattedWord.slice(prefix.length);

            html += `
                <div class="suggestion-pill ${isSelected ? 'selected' : ''}" onclick="app.selectAndAccept(${index})" title="Press Tab or Enter to choose (or use ↓ / ↑ arrow keys)">
                    <span class="pill-key">${hotkey}</span>
                    <span><strong>${prefixDisplay}</strong>${suffixDisplay}</span>
                    ${item.isSlang ? '<span class="pill-slang-badge"><i class="fab fa-whatsapp"></i> Slang</span>' : `<span class="pill-freq">★ ${item.frequency}</span>`}
                    ${isSelected ? '<span class="pill-select-indicator"><i class="fas fa-check"></i> Tab</span>' : ''}
                </div>
            `;
        });

        this.suggestionsBar.innerHTML = html;
    }

    selectAndAccept(index) {
        if (this.currentSuggestions[index]) {
            this.selectedSuggestionIndex = index;
            this.acceptSuggestion(this.currentSuggestions[index].word);
        }
    }

    renderHeapRanking(suggestions) {
        if (!suggestions || suggestions.length === 0) {
            this.heapList.innerHTML = `<div style="color: var(--text-dim); font-size: 0.85rem; text-align: center; padding: 20px 0;">Type a prefix in the editor to populate the Heap</div>`;
            return;
        }

        const maxFreq = Math.max(...suggestions.map(s => s.frequency), 1);
        let html = "";
        suggestions.forEach((item, idx) => {
            const pct = Math.min(100, Math.max(15, Math.round((item.frequency / maxFreq) * 100)));
            html += `
                <div class="heap-item" onclick="app.acceptSuggestion('${item.word}')">
                    <span class="heap-rank">#${idx + 1}</span>
                    <span class="heap-word">${item.word}</span>
                    <div class="heap-bar-container">
                        <div class="heap-bar-fill" style="width: ${pct}%;"></div>
                    </div>
                    <span class="heap-score">${item.frequency}</span>
                </div>
            `;
        });

        this.heapList.innerHTML = html;
    }

    updateGhostOverlay(suffix) {
        if (!suffix) {
            this.ghostOverlay.innerHTML = "";
            return;
        }

        const text = this.editor.value;
        const cursor = this.editor.selectionStart;
        const before = text.slice(0, cursor);

        this.ghostOverlay.innerHTML = `${this.escapeHtml(before)}<span class="ghost-text">${this.escapeHtml(suffix)}</span>`;
    }

    checkAndShowTypo(word, isSentenceStart = false) {
        if (!word || word.length < 2) {
            this.hideTypoAlert();
            return;
        }

        const spellRes = this.trie.getSpellCorrections(word, 2, 5);
        if (spellRes.isCorrect) {
            this.hideTypoAlert();
            return;
        }

        if (spellRes.suggestions.length > 0) {
            this.typoWordSpan.textContent = word;
            let html = "";
            spellRes.suggestions.forEach(cand => {
                const formattedCandidate = this.formatCaseMatch(word, cand.word, isSentenceStart);
                html += `
                    <button class="correction-btn" onclick="app.replaceTypo('${word}', '${formattedCandidate}', ${cand.distance})">
                        <i class="fas fa-check" style="color: var(--accent-emerald);"></i>
                        <strong>${formattedCandidate}</strong>
                        <span class="correction-dist">d=${cand.distance}</span>
                        <span class="correction-conf">${cand.confidence}%</span>
                    </button>
                `;
            });
            this.typoSuggestionsList.innerHTML = html;
            this.typoAlertBox.classList.add("active");
        } else {
            this.hideTypoAlert();
        }
    }

    replaceTypo(oldWord, newWord, distance = 1) {
        const text = this.editor.value;
        const regex = new RegExp(`\\b${oldWord}\\b`, "i");
        this.editor.value = text.replace(regex, newWord);

        this.typoFixCount++;
        if (this.dashTypoFixCount) this.dashTypoFixCount.textContent = this.typoFixCount;

        // Add to recent fixes feed
        this.recentFixes.unshift({
            typo: oldWord,
            fix: newWord,
            dist: distance,
            time: "Just now"
        });
        if (this.recentFixes.length > 5) this.recentFixes.pop();
        this.renderRecentFixes();

        // Frequency reward
        this.trie.incrementFrequency(newWord.toLowerCase(), 30);
        this.showToast(`Replaced "${oldWord}" → "${newWord}"`, "success");
        this.hideTypoAlert();
        this.handleEditorInput();
    }

    renderRecentFixes() {
        if (!this.recentFixesTableBody) return;
        let html = "";
        this.recentFixes.forEach(item => {
            html += `
                <tr>
                    <td style="color: #e11d48; font-weight: 700;">${this.escapeHtml(item.typo)}</td>
                    <td>➔ <strong style="color: #059669;">${this.escapeHtml(item.fix)}</strong></td>
                    <td><span class="correction-dist">d=${item.dist}</span></td>
                    <td style="text-align: right; color: var(--text-dim); font-size: 0.75rem;">${item.time}</td>
                </tr>
            `;
        });
        this.recentFixesTableBody.innerHTML = html;
    }

    renderSlangGrid() {
        if (!this.allSlangGrid) return;
        let html = "";
        for (const [slang, expansion] of Object.entries(this.whatsappSlang)) {
            html += `
                <div style="background: #ffffff; border: 1px solid var(--border-subtle); padding: 12px 16px; border-radius: var(--radius-sm); display: flex; justify-content: space-between; align-items: center; box-shadow: var(--shadow-sm);">
                    <div>
                        <strong style="color: #9333ea; font-family: 'JetBrains Mono', monospace; font-size: 0.95rem;">${slang}</strong>
                        <div style="font-size: 0.8rem; color: var(--text-main); margin-top: 2px;">${expansion}</div>
                    </div>
                    <button class="slang-chip-btn" style="padding: 4px 10px; font-size: 0.72rem;" onclick="app.testSlang('${slang}')">
                        Try
                    </button>
                </div>
            `;
        }
        this.allSlangGrid.innerHTML = html;
    }

    testSlang(slang) {
        this.showTab("tab-dashboard");
        this.setSampleText(slang);
        this.showToast(`Tested slang "${slang}" in editor!`, "info");
    }

    setEditorFilter(category) {
        document.querySelectorAll(".filter-pill-tab").forEach(tab => tab.classList.remove("active"));
        if (event && event.target) event.target.classList.add("active");

        if (category === "slang") {
            this.setSampleText("idk");
        } else if (category === "india") {
            this.setSampleText("India");
        } else if (category === "tech") {
            this.setSampleText("prog");
        } else if (category === "typos") {
            this.setSampleText("computr");
        } else {
            this.clearEditor();
        }
    }

    hideTypoAlert() {
        this.typoAlertBox.classList.remove("active");
    }

    clearSuggestions() {
        this.suggestionsBar.innerHTML = `<span style="color: var(--text-dim); font-size: 0.85rem;">Type letters to see instant completions...</span>`;
        this.heapList.innerHTML = `<div style="color: var(--text-dim); font-size: 0.85rem; text-align: center; padding: 20px 0;">Type a prefix in the editor to populate the Heap</div>`;
        this.currentSuggestions = [];
        this.bestGhostCompletion = "";
    }

    setSampleText(text) {
        this.editor.value = text;
        this.editor.focus();
        this.editor.selectionStart = text.length;
        this.editor.selectionEnd = text.length;
        this.handleEditorInput();
    }

    clearEditor() {
        this.editor.value = "";
        this.handleEditorInput();
        this.editor.focus();
    }

    // =========================================================================
    // DYNAMIC SVG TRIE VISUALIZER
    // =========================================================================
    renderTrieTree(prefix = "how") {
        const visualData = this.trie.getVisualSubtree(prefix, 4, 5);
        const meta = document.getElementById("trieRenderMeta");
        if (meta) {
            meta.textContent = `Showing active branch for prefix: "${prefix || 'ROOT'}"`;
        }

        const svg = this.trieSvg;
        svg.innerHTML = "";

        const viewport = document.createElementNS("http://www.w3.org/2000/svg", "g");
        viewport.classList.add("viewport");
        viewport.setAttribute("transform", `translate(${this.panX}, ${this.panY}) scale(${this.zoom})`);
        svg.appendChild(viewport);

        const linksGroup = document.createElementNS("http://www.w3.org/2000/svg", "g");
        const nodesGroup = document.createElementNS("http://www.w3.org/2000/svg", "g");
        viewport.appendChild(linksGroup);
        viewport.appendChild(nodesGroup);

        const levelHeight = 90;
        const nodeRadius = 18;

        const assignPositions = (node, depth, left, right) => {
            const x = (left + right) / 2;
            const y = depth * levelHeight + 30;
            node.x = x;
            node.y = y;

            if (node.children && node.children.length > 0) {
                const step = (right - left) / node.children.length;
                node.children.forEach((child, idx) => {
                    const childLeft = left + idx * step;
                    const childRight = childLeft + step;
                    assignPositions(child, depth + 1, childLeft, childRight);
                });
            }
        };

        const canvasWidth = Math.max(900, Math.pow(visualData.tree.childrenCount || 4, 3) * 35);
        assignPositions(visualData.tree, 0, 0, canvasWidth);

        const drawTree = (node) => {
            if (node.children) {
                node.children.forEach(child => {
                    const path = document.createElementNS("http://www.w3.org/2000/svg", "path");
                    const dx = child.x - node.x;
                    const dy = child.y - node.y;
                    const d = `M ${node.x} ${node.y} C ${node.x} ${node.y + dy * 0.5}, ${child.x} ${node.y + dy * 0.5}, ${child.x} ${child.y}`;
                    path.setAttribute("d", d);
                    path.setAttribute("class", "node-link");
                    linksGroup.appendChild(path);

                    drawTree(child);
                });
            }

            const circle = document.createElementNS("http://www.w3.org/2000/svg", "circle");
            circle.setAttribute("cx", node.x);
            circle.setAttribute("cy", node.y);
            circle.setAttribute("r", nodeRadius);
            circle.setAttribute("class", `node-circle ${node.isEnd ? 'is-end' : 'normal'}`);

            circle.addEventListener("click", () => {
                if (node.word) {
                    this.showTab("tab-dashboard");
                    this.setSampleText(node.word);
                    this.showToast(`Loaded word "${node.word}" into playground!`, "info");
                }
            });

            const text = document.createElementNS("http://www.w3.org/2000/svg", "text");
            text.setAttribute("x", node.x);
            text.setAttribute("y", node.y);
            text.setAttribute("class", "node-text");
            text.textContent = node.name;

            if (node.isEnd) {
                const title = document.createElementNS("http://www.w3.org/2000/svg", "title");
                title.textContent = `Word: ${node.word} | Freq: ${node.frequency}`;
                circle.appendChild(title);
            }

            nodesGroup.appendChild(circle);
            nodesGroup.appendChild(text);
        };

        drawTree(visualData.tree);
    }

    // =========================================================================
    // LEVENSHTEIN DP MATRIX CALCULATOR
    // =========================================================================
    computeDPMatrix() {
        const s1 = this.matrixSourceInput.value.trim().toLowerCase() || "computr";
        const s2 = this.matrixTargetInput.value.trim().toLowerCase() || "computer";

        const res = TrieEngine.getDetailedLevenshteinMatrix(s1, s2);
        if (this.matrixDistanceVal) this.matrixDistanceVal.textContent = res.distance;

        const m = res.matrix;
        const pathSet = new Set(res.path.map(p => `${p.row},${p.col}`));

        let html = `<table class="dp-table"><thead><tr><th></th><th>ε</th>`;
        for (let j = 0; j < s2.length; j++) {
            html += `<th>${s2[j].toUpperCase()}</th>`;
        }
        html += `</tr></thead><tbody>`;

        for (let i = 0; i <= s1.length; i++) {
            html += `<tr><th>${i === 0 ? 'ε' : s1[i - 1].toUpperCase()}</th>`;
            for (let j = 0; j <= s2.length; j++) {
                const isPath = pathSet.has(`${i},${j}`);
                const op = res.operations[i][j];
                
                html += `
                    <td class="${isPath ? 'cell-path' : ''}" title="Operation: ${op} | Row: ${i}, Col: ${j}">
                        <span class="dp-cell-val">${m[i][j]}</span>
                        <span class="dp-cell-op">${op}</span>
                    </td>
                `;
            }
            html += `</tr>`;
        }
        html += `</tbody></table>`;

        this.matrixTableWrapper.innerHTML = html;
    }

    // =========================================================================
    // BATCH SENTENCE & PARAGRAPH SPELL-FIXER
    // =========================================================================
    runBatchAnalyze() {
        const text = this.batchInputText.value;
        if (!text.trim()) {
            this.batchDiffContent.innerHTML = "Please enter some text to analyze.";
            return;
        }

        const tokens = text.match(/[a-zA-Z']+|[^a-zA-Z']+/g) || [];
        let outputHtml = "";
        let correctedCount = 0;

        for (const token of tokens) {
            if (/^[a-zA-Z']+$/.test(token)) {
                const cleanKey = token.toLowerCase().replace(/'/g, "");
                
                // 1. WhatsApp slang expansion in batch
                if (this.whatsappSlang[cleanKey]) {
                    const expanded = this.whatsappSlang[cleanKey];
                    outputHtml += `<span class="diff-typo">${this.escapeHtml(token)}</span> <span class="diff-fixed">${this.escapeHtml(expanded)}</span>`;
                    correctedCount++;
                    continue;
                }

                const searchRes = this.trie.searchExact(token);
                if (searchRes.found) {
                    const formatted = this.formatCaseMatch(token, searchRes.word || token);
                    outputHtml += `<span style="color: var(--text-main); font-weight: 600;">${this.escapeHtml(formatted)}</span>`;
                } else {
                    const spell = this.trie.getSpellCorrections(token, 2, 1);
                    if (spell.suggestions.length > 0) {
                        const best = this.formatCaseMatch(token, spell.suggestions[0].word);
                        outputHtml += `<span class="diff-typo">${this.escapeHtml(token)}</span> <span class="diff-fixed">${this.escapeHtml(best)}</span>`;
                        correctedCount++;
                    } else {
                        outputHtml += `<span class="diff-typo">${this.escapeHtml(token)}</span>`;
                    }
                }
            } else {
                outputHtml += this.escapeHtml(token);
            }
        }

        this.batchDiffContent.innerHTML = outputHtml;
        this.showToast(`Batch complete: fixed & expanded ${correctedCount} words!`, "success");
    }

    // =========================================================================
    // VOCABULARY & DICTIONARY MANAGER
    // =========================================================================
    handleAddWord() {
        const word = this.newWordInput.value.trim();
        const freq = parseInt(this.newWordFreq.value, 10) || 9500;

        if (!word || !/^[a-zA-Z']+$/.test(word)) {
            this.showToast("Please enter a valid word.", "warn");
            return;
        }

        const isNew = this.trie.insert(word, freq);
        this.showToast(isNew ? `Added word "${word}" to dictionary!` : `Updated frequency for "${word}"!`, "success");

        this.newWordInput.value = "";
        this.updateTelemetry();
        this.populateDictTable();
    }

    deleteDictWord(word) {
        this.trie.deleteWord(word);
        this.showToast(`Deleted "${word}" from dictionary.`, "info");
        this.updateTelemetry();
        this.populateDictTable();
    }

    populateDictTable() {
        const filter = (this.dictFilterInput.value || "").trim().toLowerCase();
        let entries = [];

        const collect = (node) => {
            if (node.isEndOfWord && (!filter || (node.word && node.word.toLowerCase().includes(filter)))) {
                entries.push({ word: node.word, freq: node.frequency });
            }
            for (const child of node.children.values()) {
                collect(child);
            }
        };
        collect(this.trie.root);

        entries.sort((a, b) => b.freq - a.freq);

        if (this.dictTableCount) {
            this.dictTableCount.textContent = `Showing ${entries.length.toLocaleString()} entries`;
        }

        const slice = entries.slice(0, 100);
        let html = "";
        slice.forEach((entry, idx) => {
            html += `
                <tr>
                    <td style="color: var(--text-dim); font-weight: 700;">${idx + 1}</td>
                    <td style="font-weight: 800; color: var(--text-main);">${entry.word}</td>
                    <td style="color: var(--accent-emerald); font-weight: 700;">★ ${entry.freq}</td>
                    <td>
                        <button class="btn-secondary-pill" style="padding: 2px 8px; font-size: 0.72rem; color: #ef4444;" onclick="app.deleteDictWord('${entry.word}')">
                            <i class="fas fa-trash"></i>
                        </button>
                    </td>
                </tr>
            `;
        });

        if (entries.length === 0) {
            html = `<tr><td colspan="4" style="text-align: center; color: var(--text-dim); padding: 24px;">No words match your filter.</td></tr>`;
        }

        if (this.dictTableBody) {
            this.dictTableBody.innerHTML = html;
        }
    }

    openAddWordModal() {
        this.showTab("tab-dict");
        if (this.newWordInput) this.newWordInput.focus();
    }

    exportDictionary() {
        let entries = [];
        const collect = (node) => {
            if (node.isEndOfWord) entries.push(`${node.word} ${node.frequency}`);
            for (const child of node.children.values()) collect(child);
        };
        collect(this.trie.root);

        const blob = new Blob([entries.join("\n")], { type: "text/plain" });
        const url = URL.createObjectURL(blob);
        const a = document.createElement("a");
        a.href = url;
        a.download = "dictionary_export.txt";
        a.click();
        URL.revokeObjectURL(url);
        this.showToast(`Exported ${entries.length.toLocaleString()} words to dictionary_export.txt`, "success");
    }

    updateTelemetry() {
        const stats = this.trie.getStats();
        if (this.dashTotalWords) this.dashTotalWords.textContent = stats.totalWords.toLocaleString();
    }

    showToast(message, type = "info") {
        const toast = document.createElement("div");
        toast.className = `toast ${type}`;
        const icon = type === "success" ? "fa-circle-check" : type === "warn" ? "fa-triangle-exclamation" : "fa-circle-info";
        toast.innerHTML = `<i class="fas ${icon}"></i> <span>${message}</span>`;
        
        this.toastContainer.appendChild(toast);
        setTimeout(() => {
            toast.style.opacity = "0";
            toast.style.transform = "translateX(100%)";
            toast.style.transition = "all 0.3s ease";
            setTimeout(() => toast.remove(), 300);
        }, 3000);
    }

    escapeHtml(str) {
        if (!str) return "";
        return str
            .replace(/&/g, "&amp;")
            .replace(/</g, "&lt;")
            .replace(/>/g, "&gt;")
            .replace(/"/g, "&quot;")
            .replace(/'/g, "&#039;");
    }
}

// Instantiate on page load
document.addEventListener("DOMContentLoaded", () => {
    window.app = new AutoCorrectApp();
});
