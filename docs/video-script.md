# YouTube presentation: outline, title and description

Target length: about 10–12 minutes. Treat the beats below as points to cover in your own words, not a script to read aloud. For every section, give the *reason* first, then show it working.

Before recording:

* Make the terminal font large (at least 18 pt) and use a dark theme.
* Run `make clean && make` on camera, or just before recording, so the build shows as clean.
* Keep `docs/diagrams/png/` open in an image viewer, in order, so you can switch to each diagram quickly.

## Run of show

| Time | Section | What to show | Points to make |
|---|---|---|---|
| 0:00–0:40 | Intro | Face cam or title slide | Name, CSC-6021, Dr. Govindaswamy. Topic: B+ tree. Area: Cloud / Databases. App: indexed cloud-record search. |
| 0:40–2:00 | The problem | `06-cloud-architecture.png` | A cloud database holds billions of rows on network storage, and every page read is a round trip. A full scan grows with the data, which is unacceptable. An index has to find a row in a handful of reads. |
| 2:00–3:45 | What a B+ tree is | `01-bplus-tree-structure.png`, then `05-btree-vs-bplus-tree.png` | Many keys per node, and order *m*. Internal nodes are signposts only; leaves hold the records and are linked. Every node at least half full; all leaves at the same depth, so the tree is always balanced. B-tree vs B+ tree: data only in the leaves gives higher fan-out and cheap range scans. |
| 3:45–5:00 | Operations, using the diagrams | `02-search-and-range-scan.png`, `03-insert-...png`, `04-delete-...png` | Trace the search for 1052 by hand: 3 reads. Range search: one descent, then sideways. Insert: leaf split copies the key up, internal split moves it up, and only a root split adds height. Delete: borrow if a sibling can spare a key, otherwise merge. |
| 5:00–6:00 | Real-world uses | Report section 6/7, or a slide | MySQL InnoDB (16 KB pages), PostgreSQL, SQL Server and Azure SQL (rowstore indexes are B+ trees), SQLite (all data in the leaves), Btrfs. Cloud: DynamoDB storage nodes keep partition data in a B-tree; Aurora runs B+ tree indexes on 6-copy, 3-AZ storage. |
| 6:00–7:00 | C++ design | `07-uml-class-diagram.png`, then a quick scroll through `BPlusTree.h` and `splitLeaf` / `fixUnderflow` | `BPlusTree` is a pure data structure, `CloudIndexApp` is the menu, and `InputUtils` validates input. Mention `validate()` and the randomized tests against `std::map`. |
| 7:00–10:00 | Live demo | Terminal | **1** load → **7** structure → **3** `1052` (path and cost) → **3** `1050` (miss) → **4** `1020`–`1050` → **2** add `1078` (show the validation errors, then the cascading split) → **7** → **6** delete `1044` (merge) and `1052` (merge plus borrow) → **10** with `4242` (index 3 reads vs scan 22 rows) → **9** (self-check PASSED). Edge cases: type `abc` at the menu, try duplicate `1052`. |
| 10:00–11:00 | Complexity / performance | Option **11** with order `128`, then optionally with `4` | O(log_m n). 1K → 1M records adds only one level. At 1M records, order 4 needs 13 reads and order 128 needs 3, and order 128 is about 3× faster even in RAM. On cloud storage, each of those extra reads would be a network round trip. |
| 11:00–11:30 | Wrap-up | GitHub repo page | Show the README, and say where the code, report, diagrams and LinkedIn post are. |

## Title

> B+ Trees in Cloud Databases: Indexed Record Search in C++ | BLAST 01 | Data Structures

## Description (paste into YouTube)

```
How do cloud databases find one record among millions in only a few reads? With B+ trees.

In this BLAST 01 project for CSC-6021 (Data Structures and Algorithms II, Dr. Victor Govindaswamy), I explain the B-tree / B+ tree, how search, range search, insertion (splits) and deletion (borrow/merge) work, their time and space complexity, and how B+ tree indexes are used in cloud databases such as Amazon DynamoDB, Amazon Aurora and Azure SQL.

I then demo "CloudVault Record Index", a menu-based C++ application I built from scratch that indexes cloud resource records with a B+ tree. It shows search paths, node splits and merges live, and runs a 1K to 1M record scalability experiment.

📂 GitHub (code, report, UML, diagrams): https://github.com/Pranavxoxo/BLAST01-BPlusTree-Cloud-Index
💼 LinkedIn post: <paste LinkedIn post link>

Chapters
0:00 Intro
0:40 Why cloud databases need indexes
2:00 What is a B+ tree
3:45 Search, range search, insert and delete
5:00 Real-world uses: MySQL, PostgreSQL, Azure SQL, DynamoDB, Aurora
6:00 C++ design and UML
7:00 Live demo
10:00 Complexity and scalability results
11:00 Wrap-up

#BPlusTree #DataStructures #CPlusPlus #CloudComputing #Databases #Algorithms #ComputerScience
```

Tags: B+ tree, B-tree, data structures, C++, cloud computing, databases, database index, DynamoDB, algorithms, CSC-6021
