# 🛠️ Implementation Notes — `graphe.c`

> Internal logic and design decisions for every function in the graph engine.  
> Use this as a reference when reading, extending, or debugging the code.

---

## 1. `creerGraphe(int V)`

**Goal:** Create an empty graph capable of holding `V` articles.

**Logic:**
- Allocate memory for the `grapheReseau` struct (`malloc`)
- Validate allocation (`!= NULL`), print red error on failure
- Set `g->V = V`
- Allocate three parallel arrays:
  - `g->articles`  → `calloc(V, sizeof(ELEMENT))`
  - `g->adjList`   → `calloc(V, sizeof(LISTE))`
  - `g->degre_in`  → `calloc(V, sizeof(int))`
- Validate each allocation; free previously allocated ones on failure (avoid leaks)
- For each `i` from `0` to `V-1`: `g->adjList[i] = listeCreer()` (each vertex starts with an empty adjacency list)
- Return `g`

**Guard:** if `V <= 0` or any `malloc` fails → return `NULL` with red error message.

---

## 2. `chargerGraphe(const char *nomfichier)`

**Goal:** Read a `.txt` file and build the graph in memory.

**Logic:**
- Open the file in read mode (`fopen`) → validate `!= NULL`
- Read all lines into a string array (`fgets` loop)
- **Pass 1:** count lines starting with `'A'` → this is `V` (number of vertices)
- Call `creerGraphe(V)` to allocate the structure
- **Pass 2:** re-scan lines:
  - `'A'` line → `sscanf` to extract: `id`, `title`, `source`, `score`, `day`, `month`, `year`, `hour`, `minute` → call `creerArticle()`, fill fields, store in `g->articles[id]`
  - `'C'` line → `sscanf` to extract `src`, `dest` → call `ajouterCitation(g, src, dest)`
  - `'#'` line → skip (comment)
- Close file (`fclose`)
- Return `g`

---

## 3. `detruireGraphe(grapheReseau g)`

**Goal:** Free all memory allocated by the graph.

**Logic:**
- Validate `g != NULL`
- For each `i` from `0` to `g->V - 1`:
  - `listeDetruire(g->adjList[i])` — free adjacency list
  - `detruireArticle(g->articles[i])` — free article
- `free(g->adjList)`
- `free(g->articles)`
- `free(g->degre_in)`
- `free(g)`

> **Order matters:** free contents BEFORE freeing the arrays that held them.

---

## 4. `ajouterArticle(grapheReseau g, ELEMENT art)`

**Goal:** Add a new article to the graph (grow the array dynamically).

**Logic:**
- Validate `g != NULL` and `art != ELEMENT_VIDE`
- Validate `art->id >= 0`
- Check for duplicate ID via `IndexParId` → yellow warning if found
- `nouvelleTaille = g->V + 1`
- Secure `realloc` using temporary pointers:
  ```c
  tmp_articles = realloc(g->articles, nouvelleTaille * sizeof(ELEMENT))
  tmp_adjList  = realloc(g->adjList,  nouvelleTaille * sizeof(LISTE))
  tmp_degre_in = realloc(g->degre_in, nouvelleTaille * sizeof(int))
  ```
- Validate each `realloc != NULL`; print error and return `0` on failure
- Update the graph pointers
- `copierArticle(&g->articles[g->V], art)` — deep copy into new slot
- `g->adjList[g->V]  = listeCreer()`
- `g->degre_in[g->V] = 0`
- `g->V = nouvelleTaille`
- Return `1` (success)

---

## 5. `supprimerArticle(grapheReseau g, int idArt)`

**Goal:** Remove an article and all its incoming/outgoing citations.

**Logic:**
- Validate `g != NULL`
- Find `i = IndexParId(g, idArt)` → if `-1`: error message
- **Remove all incoming arcs** pointing to `idArt`:
  - For each `j` from `0` to `g->V - 1` (skip `j == i`):
    - Walk `g->adjList[j]` node by node with a position counter
    - If `courant->info->id == idArt` → `supprimer(adjList[j], pos)` (do NOT increment `pos` after deletion — positions shift)
    - Else: increment `pos`, advance `courant`
- `listeDetruire(g->adjList[i])` — destroy outgoing list
- **Swap with last** to avoid holes:
  ```c
  g->articles[i] = g->articles[dernierIndex]
  g->adjList[i]  = g->adjList[dernierIndex]
  g->degre_in[i] = g->degre_in[dernierIndex]
  ```
  Then recalculate `degre_in[i]` for the swapped article
- `g->V--`
- `realloc` to shrink memory
- Return `1` (success)

---

## 6. `ajouterCitation(grapheReseau g, int idSrc, int idDest)`

**Goal:** Add a directed arc `idSrc → idDest`.

**Logic:**
- Validate `g != NULL`
- `indexSrc  = IndexParId(g, idSrc)`  → if `-1`: error
- `indexDest = IndexParId(g, idDest)` → if `-1`: error
- Check for duplicate: walk `g->adjList[indexSrc]`; if any node has `id == idDest` → yellow "already exists" → return `0`
- `inserer(g->adjList[indexSrc], g->articles[indexDest], position_end)`
- `g->degre_in[indexDest]++`
- Return `1` (success)

---

## 7. `supprimerCitation(grapheReseau g, int idSrc, int idDest)`

**Goal:** Remove the directed arc `idSrc → idDest`.

**Logic:**
- Validate `g != NULL`
- Resolve `indexSrc` and `indexDest` via `IndexParId`
- Check `adjList[indexSrc]` is not empty
- Walk `adjList[indexSrc]` with a position counter `pos = 1`:
  - Each node: `courant->info` holds the article, `courant->suivant` is the next pointer
  - If `courant->info->id == idDest`:
    - `supprimer(g->adjList[indexSrc], pos)`
    - `g->degre_in[indexDest]--`
    - Return `1`
  - Else: `courant = courant->suivant`, `pos++`
- If end of list without match → yellow "not found" → return `0`

---

## 8. `afficherGraphe(grapheReseau g)`

**Goal:** Display all articles and their citations.

**Logic:**
- Validate `g != NULL`
- For each `i` from `0` to `g->V - 1`:
  - Display `g->articles[i]` info (title, id, source, score, date)
  - If `adjList[i]` is empty → print "(does not cite any article)"
  - Else walk `adjList[i]` and print each cited article (`--> title`)

---

## 9. `articlesCites(grapheReseau g, int idSrc)`

**Goal:** Display all articles cited by `idSrc`.

**Logic:**
- Validate `g != NULL`
- `idx = IndexParId(g, idSrc)` → if `-1`: error
- Walk `g->adjList[idx]` from head:
  - For each `courant != NULL`: display `courant->info` fields; advance `courant`

---

## 10. `articlesCitants(grapheReseau g, int idDest)`

**Goal:** Display all articles that cite `idDest`.

**Logic:**
- Validate `g != NULL`
- `idxDest = IndexParId(g, idDest)` → if `-1`: error
- For each `i` from `0` to `g->V - 1` (skip `i == idxDest`):
  - Walk `g->adjList[i]`
  - If any node has `courant->info->id == idDest` → display `g->articles[i]`; `break` (avoid duplicates)

---

## 11. `sourcesOriginales(grapheReseau g)`

**Goal:** Display articles that cite no one (out-degree = 0).

**Logic:**
- For each `i`: if `g->articles[i] != ELEMENT_VIDE` AND `estVide(g->adjList[i])` → display

---

## 12. `articlesIsoles(grapheReseau g)`

**Goal:** Display articles that are neither cited nor cite anyone.

**Logic:**
- For each `i`: if `g->degre_in[i] == 0` AND `estVide(g->adjList[i])` → display

---

## 13. `articlePlusCite(grapheReseau g)`

**Goal:** Return the article with the highest `degre_in`.

**Logic:**
- `maxDegre = -1`, `e = ELEMENT_VIDE`
- For each `i`: if `g->degre_in[i] > maxDegre` → update `maxDegre` and `e`
- Return `e`

---

## 14. `comparerArticle(ELEMENT art1, ELEMENT art2)`

**Goal:** Compare two articles chronologically.

**Logic (cascaded comparisons):**
```
if annee differs  → return difference
if mois differs   → return difference
if jour differs   → return difference
if heure differs  → return difference
return minute difference
```

Returns: negative if `art1` is earlier, `0` if equal, positive if `art1` is later.

---

## 15. `trierParDate(grapheReseau g)`

**Goal:** Sort articles in the graph by chronological order (insertion sort).

**Logic:**
```
for i from 1 to g->V - 1:
    save: tempArt  = g->articles[i]
          tempList = g->adjList[i]
          tempDeg  = g->degre_in[i]
    j = i - 1
    while j >= 0 AND comparerArticle(g->articles[j], tempArt) > 0:
        shift right: articles[j+1], adjList[j+1], degre_in[j+1] = [j]
        j--
    place tempArt at j+1
```

> **Critical:** `adjList` and `degre_in` must move together with `articles` to keep the graph consistent.

---

## 16. `premierCitant(grapheReseau g, int idDest)`

**Goal:** Find the oldest article that cites `idDest`.

**Logic:**
- `plusAncien = ELEMENT_VIDE`
- For each `i` (skip `i == idxDest`): walk `adjList[i]`; if cites `idDest`:
  - If `plusAncien == NULL` OR `comparerArticle(articles[i], plusAncien) < 0` → update `plusAncien`
- Display result or "no citant found"

---

## 17. `chainePropagation(grapheReseau g, int idSrc)`

**Goal:** Reconstruct the chronological propagation chain from `idSrc`.

**Logic:**
- Allocate: `file[]`, `visite[]`, `chainePropagation[]`
- BFS from `idSrc` (enqueue source, mark visited):
  - Dequeue `curr`; add `g->articles[curr]` to chain
  - For each article `i` that **cites** `articles[curr]` and is not visited → enqueue, mark visited
- Sort `chainePropagation[]` by date (bubble sort with `comparerArticle`)
- Display in chronological order
- Free all allocated memory

---

## 18. `simulerPropagation(grapheReseau g, int idSrc)` — BFS

**Goal:** Simulate level-by-level propagation from `idSrc` with propagation scores.

**Logic:**
- Allocate: `file[]`, `visite[]`, `scoreProp[]`, `niveau[]`
- Initialize: enqueue `idSrc`; `scoreProp[idSrc] = articles[idSrc]->score_fiabilite`; `niveau[idSrc] = 0`
- BFS loop:
  - Dequeue `curr`; display with its propagation score and level
  - For each article `i` that cites `articles[curr]` and is unvisited:
    - `scoreProp[i] = scoreProp[curr] × (articles[i]->score_fiabilite / 100.0)`
    - `niveau[i] = niveau[curr] + 1`
    - Enqueue `i`, mark visited
- Display total articles reached + reliability warning if source score < 50
- Free all memory

---

## 19. `articlesAccessibles(grapheReseau g, int idSrc)`

**Goal:** Display all articles reachable from `idSrc` via **outgoing** arcs.

**Logic:**
- BFS following `adjList[curr]` (citations made BY `curr`, not citations TO `curr`)
- For each unvisited neighbor in `adjList[curr]` → enqueue, mark visited
- Display all visited articles (including source)

---

## 20. `analyserArticle(ELEMENT article)`

**Goal:** Compute suspicion score and update `score_fiabilite`.

**Formula:**
```
score_suspicion = (nb_fakes × 40) + (nb_mots_suspects × 10)
score_fiabilite = max(0, 100 - score_suspicion)
```

**Logic:**
- Copy title to `titreLower` using `tolower()`
- Replace all `'_'` with `' '` (titles use underscores; `BASE_FAKES` uses spaces)
- Count `nb_fakes`: for each phrase in `BASE_FAKES` → `strstr()` match
- Count `nb_mots_suspects`: for each word in `MOTS_SUSPECTS` → `strstr()` match
- Update `article->score_fiabilite`
- Return `score_suspicion`

> **No `printf` in this function** — it is a pure calculation; display is handled by `analyserReseau()`.

---

## 21. `analyserReseau(grapheReseau g)`

**Goal:** Analyze all articles and print the classification report.

**Logic:**
- For each article `i`:
  1. Call `analyserArticle(g->articles[i])` → updates `score_fiabilite`
  2. Read `score = g->articles[i]->score_fiabilite`
  3. Print classification first:
     - `score < 40`  → `[SUSPECT]` red
     - `score < 70`  → `[DOUTEUX]` yellow
     - `score >= 70` → `[FIABLE]`  green
  4. Display title and score
  5. Rebuild `titreLower` and display matched fake phrases and suspect words
- Print final summary: count of SUSPECT / DOUTEUX / FIABLE

---

## 22. `articlesSuspectsCites(grapheReseau g)`

**Goal:** Display suspect articles (score < 40 or keyword match) that are cited (`degre_in > 0`), sorted by in-degree descending.

**Logic:**
- Build `suspects[]` array of indices satisfying the condition
- Bubble sort by `degre_in` descending
- Display each suspect article with its score and citation count

---

## 23. `simulerSuppression(grapheReseau g, int idArt)` *(Bonus)*

**Goal:** Simulate the impact of removing an article **without actually deleting it**.

**Logic:**
- Display articles that would lose an incoming citation (those cited by `idArt` via `adjList[idArt]`)
- Display articles that would lose an outgoing citation (those that cite `idArt` via scanning all `adjList[j]`)
- Identify articles that would become **isolated** after removal:
  - Articles cited only by `idArt` (`degre_in == 1`) AND citing no one (`estVide(adjList)`)
- Print summary: `nb_citants`, `nb_cites`, `nb_isolated`

---

## 24. `neutraliserPropagation(grapheReseau g, int idSrc, int idDest)` *(Bonus)*

**Goal:** Cut the arc `idSrc → idDest` and measure the reduction in reachable articles.

**Logic:**
- Validate `g`, `idxSrc`, `idxDest`; confirm the citation exists
- **BFS before** — count articles reachable from `idxDest` → `nbAvant`
- Remove the citation `idSrc → idDest` (no `printf`); decrement `degre_in[idxDest]`
- **BFS after** — recount reachable articles from `idxDest` → `nbApres`
- Display:
  - Accessible before: `nbAvant`
  - Accessible after: `nbApres`
  - Propagation reduced by: `nbAvant - nbApres`
- If `degre_in[idxDest] == 0` → yellow warning "no longer cited by anyone"
- Free `visite_avant`, `visite_apres`, `file`
- Return `1` (success)

---

## 25. `IndexParId(grapheReseau g, int id)` — Internal Helper

**Goal:** Map an article `id` to its array index in `articles[]`.

**Logic:**
- Validate `g != NULL` → return `-1`
- Linear scan from `0` to `g->V - 1`:
  - If `g->articles[i] != ELEMENT_VIDE` AND `g->articles[i]->id == id` → return `i`
- If not found → return `-1`

---

## 26. `ajouterArticleSuspect(grapheReseau g, ELEMENT art)`

**Goal:** Run suspicion check **before** adding an article to the graph.

**Logic:**
- Validate `g != NULL` and `art != ELEMENT_VIDE`
- `analyserArticle(art)` → compute score
- If `score < 40` (SUSPECT):
  - Print red alert + details (matched fake phrases + suspect words)
  - Ask for confirmation via `scanf`; if `choix != 1` → cancel, return `0`
- If `score < 70` (DOUTEUX): yellow warning
- If `score >= 70` (FIABLE): green message
- Call `ajouterArticle(g, art)` → return its result

---

## Notes on Memory Safety

- **Every `malloc`/`calloc`/`realloc` return value is checked** before use
- `realloc` is always done via a **temporary pointer** — if it fails, the original pointer remains valid
- `copierArticle()` is used for **ownership transfer** (deep copy); `affecterArticle()` is used for **reference sharing** (shallow copy)
- BFS allocates `file[]` and `visite[]` on the heap and **always frees them** before returning, even on early error exits
- `supprimerArticle()` walks adjacency lists using a **saved `suivant` pointer** before calling `supprimer()` to avoid use-after-free on the traversal cursor
