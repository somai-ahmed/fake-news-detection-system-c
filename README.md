# 🔍fake news detection system 


A C application that models an information network as a **directed graph**, where each node is a web article and each arc `A → B` means article A cites article B. The program detects misinformation, simulates how fake news spreads through the network, and classifies articles by reliability score.

---

## 📁 Project Structure

```
FakeNewsDetector/
│
├── eltarticle.h / eltarticle.c   # ADT ELEMENT — article definition & operations
├── liste.h                        # ADT LIST interface (linked list declarations)
├── lstrp.h / lstrp.c              # Linked list implementation
├── graphe.h / graphe.c            # ADT grapheReseau — directed graph engine
├── fakesdb.h / fakesdb.c          # Suspicious keywords & fake phrase database
├── main.c                         # Interactive menu & entry point
└── DB.txt                         # Sample dataset (articles + citations)
```

### Dependency Diagram

```
         ┌─────────────────┐
         │     main.c      │
         │   (Menu / UI)   │
         └────────┬────────┘
                  │ uses
      ┌───────────┼───────────┐
      ▼           ▼           ▼
 ┌──────────┐ ┌───────────┐ ┌──────────┐
 │ graphe.h │ │ fakesdb.h │ │ lstrp.h  │
 │ graphe.c │ │ fakesdb.c │ │ lstrp.c  │
 └────┬─────┘ └───────────┘ └────┬─────┘
      │ uses                      │
      └─────────────┬─────────────┘
                    ▼
       ┌────────────────────────┐
       │   eltarticle.h / .c    │
       │  ADT ELEMENT (article) │
       └────────────────────────┘
                    │
                    ▼
         ┌──────────────────┐
         │      liste.h     │
         │  NOEUD / LISTE   │
         └──────────────────┘
```

---

## ⚙️ Build & Run

```bash
gcc main.c eltarticle.c lstrp.c graphe.c fakesdb.c -o fake_news_detector
./fake_news_detector
```

Compatible with **GCC** and **Clang** (C99 standard). On Windows, `<windows.h>` is conditionally included for `Sleep()` and UTF-8 console support.

---

## 🧩 Features

### Network Management
- Load a network from `DB.txt` (format `A` / `C` lines)
- Add or remove an article (node) with dynamic `realloc`
- Add or remove a citation (arc) between two articles
- Display the full graph with adjacency lists

### Search & Analysis
- List articles **cited by** a given source (outgoing neighbors)
- List articles **that cite** a given target (incoming neighbors)
- Identify **original sources** (articles citing no one)
- Identify **isolated articles** (no incoming, no outgoing citations)
- Find the **most cited article** (max in-degree)
- Sort articles by **publication date** (insertion sort)
- Find the **first article** to cite a given target

### Propagation Simulation (BFS)
- Simulate spread **level by level** from a source article
- Compute a **propagation score** for each reached article: `score_parent × (reliability / 100)`
- Display all articles **reachable** from a source
- Reconstruct the **chronological propagation chain**

### Fake News Detection
- Lexical analysis of article titles using two layers:
  - **Fake phrase base** (`BASE_FAKES`) → `+40 pts` suspicion per match
  - **Suspicious keywords** (`MOTS_SUSPECTS`) → `+10 pts` per match
- Formula: `score_fiabilite = max(0, 100 - score_suspicion)`
- Classify each article: `[SUSPECT]` / `[DOUTEUX]` / `[FIABLE]`
- List **suspect articles sorted by in-degree** (widest reach first)
- Alert + confirmation prompt before adding a suspect article

### Bonus Features
- **Simulate deletion** of an article — measure impact without actually removing it
- **Neutralize propagation** — cut a specific arc and compare BFS reach before/after
- **Suspicious citation chains** — find all pairs `(i, j)` where `i` cites `j` and both scores fall below a given threshold

---

## 🧠 Technical Design

### ADT ELEMENT — `eltarticle.h / .c`

Each article is stored as `articleStruct`, accessed via pointer type `ELEMENT`.

| Function | Description |
|---|---|
| `creerArticle()` | Allocates and zero-initializes an article (`malloc`) |
| `copierArticle()` | Deep copy — allocates a fully independent duplicate |
| `affecterArticle()` | Shallow copy — both pointers share the same memory |
| `comparerArticle()` | Chronological comparison by date/time fields |
| `detruireArticle()` | Frees the allocated memory |

> **Key distinction:** `copierArticle()` creates an independent copy; `affecterArticle()` makes two pointers share the same object. Mixing them causes **double-free** bugs.

---

### ADT LIST — `lstrp.c`

Singly-linked list with a head pointer and a length counter. Used both as an adjacency list in the graph and as a **FIFO queue** for BFS.

| Function | Description |
|---|---|
| `listeCreer()` | Allocates and initializes an empty list |
| `inserer(l, e, pos)` | Inserts an ELEMENT at position `pos` (1-based) |
| `supprimer(l, pos)` | Removes and frees the node at position `pos` |
| `recuperer(l, pos)` | Returns the ELEMENT at `pos` without removing it |
| `listeDetruire(l)` | Frees all nodes and the list struct |
| `listeCopier(l)` | Deep copy of the entire list |

> **BFS queue pattern:** `inserer(file, e, listeTaille(file)+1)` enqueues at tail; `recuperer(file,1)` + `supprimer(file,1)` dequeues from head. O(n) per operation.

---

### ADT grapheReseau — `graphe.h / .c`

The graph holds three parallel arrays:

```
grapheReseau g:
┌─────────────────────────────────────────────────┐
│  V          → number of vertices                │
│  articles[] → array of ELEMENT pointers         │
│  adjList[]  → array of LISTs (adjacency lists)  │
│  degre_in[] → in-degree for each article        │
└─────────────────────────────────────────────────┘
```

Key implementation details:

- `ajouterArticle()` uses **secure `realloc`** with temporary pointers to avoid memory leaks on partial failure
- `supprimerArticle()` uses a **swap-with-last** strategy before shrinking — no holes left in the array
- `chargerGraphe()` does a **two-pass parse**: first counts `A` lines to determine `V`, then builds the graph
- `IndexParId()` is the internal O(V) lookup helper — maps an article `id` to its index in `articles[]`

---

### Detection Engine — `fakesdb.h / .c`

```c
score_suspicion = (nb_fake_phrases × 40) + (nb_suspect_words × 10)
score_fiabilite = max(0, 100 - score_suspicion)
```

| Score | Classification |
|---|---|
| ≥ 70 | ✅ `[FIABLE]` — reliable |
| 40 – 69 | 🟡 `[DOUTEUX]` — doubtful |
| < 40 | 🔴 `[SUSPECT]` — suspicious |

Detection is **case-insensitive** and handles underscore-separated titles (converted to spaces before matching).

---

## 📊 Complexity

| Operation | Time | Notes |
|---|---|---|
| `ajouterCitation()` | O(n) | Insert at end of adjacency list |
| `articlesCites()` | O(d_out) | Traverse source's adjacency list |
| `articlesCitants()` | O(V·d_out) | Scan all adjacency lists |
| `trierParDate()` | O(V²) | Insertion sort on `articles[]` |
| `simulerPropagation()` | O(V + E) | Full BFS |
| `analyserReseau()` | O(V·T) | T = max title length (~100 chars) |
| `neutraliserPropagation()` | O(V·(V+E)) | Two BFS passes + arc removal |

**Space:** O(V + E) for the graph; O(V) for BFS structures.

---

## 🧪 Test Cases

`DB.txt` contains **10 articles** and **14 citations**, including a **mutual citation** between articles 6 and 8 (cycle).

| Test | What it validates |
|---|---|
| Mutual citation (6 ↔ 8) | BFS `visited[]` array prevents infinite loops on cycles |
| Title analysis with suspect words | `alerte`, `secret`, `choc` → suspicion = 30 → `[DOUTEUX]` |
| Most-cited article | Article with highest `degre_in` correctly identified |
| Non-existent ID passed | Returns error message cleanly, no crash |
| Full array + `realloc` | `ajouterArticle()` grows the graph dynamically |

---

## 📁 `DB.txt` Format

```
# Comment line (ignored by parser)
A id "title_with_underscores" source reliability_score day month year hour minute
C source_id destination_id
```

**Example:**
```
A 0 "La_verite_sur_le_vaccin_alerte_secret_choc" page_TunisieActu 8 15 3 2025 9 0
A 3 "Communique_officiel_MinSante" MinistereSante 90 15 3 2025 7 0
C 0 3
```

---

## 🔮 Possible Improvements

- **Hash table** — O(1) article lookup by ID instead of O(V) linear scan
- **Persistence** — write modified graph back to `DB.txt` after add/remove operations
- **External keyword config** — load suspicious words from a file instead of hardcoding
- **Dijkstra** — measure propagation distance between articles
- **Simplified PageRank** — update reliability scores dynamically based on network topology
- **Unit tests** — integrate a C testing framework (Unity or CMocka)
- **Doxygen** — auto-generate API docs from `.h` comment headers

---

## 📚 Standard Libraries Used

| Library | Usage |
|---|---|
| `<stdio.h>` | I/O: `printf`, `scanf`, `fopen`, `fgets` |
| `<stdlib.h>` | Memory: `malloc`, `calloc`, `realloc`, `free` |
| `<string.h>` | Strings: `strcmp`, `strncpy`, `strstr` |
| `<ctype.h>` | Character handling: `tolower`, `isalpha` |
| `<unistd.h>` | `usleep` for animations (POSIX / Linux / macOS) |
| `<windows.h>` | `Sleep`, UTF-8 console setup (Windows, conditional) |

---

## ✅ Code Quality Practices

- Interface / implementation separation via `.h` / `.c` pairs
- Include guards (`#ifndef … #define … #endif`) in all headers
- Systematic `malloc()` and `fopen()` return value checks
- `strncpy()` with explicit null terminator to prevent buffer overflows
- `ELEMENT_VIDE` (`NULL`) constant for absent articles
- **Golden rule:** everything allocated by `creerArticle()` or `creerGraphe()` is freed by `detruireArticle()` or `detruireGraphe()`

---

*Fake news is not just a content problem — it's a graph problem. This project demonstrates how classical data structures can model and analyze real-world information networks.*
