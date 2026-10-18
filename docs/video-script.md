# YouTube presentation

The video is built around **[`BLAST01-Video-Presentation.pptx`](BLAST01-Video-Presentation.pptx)**: 18 slides, with everything to say written in each slide's **speaker notes**. In PowerPoint, use *View → Presenter View* (or *Slide Show → Rehearse*) so the notes stay on your screen while the slides are recorded.

Each note starts with a rough time, e.g. `[~50 seconds]`. Read through at a relaxed pace and the whole talk runs about 15–16 minutes, including a 3-minute live demo. To get closer to 12 minutes, skip slide 5 (*Four rules*, which overlaps slide 4) and slide 17 (*Tested, not just demoed*), and shorten the demo.

Slide 15 (*Live demo*) is the cue to switch to the terminal. Its notes are a step-by-step demo checklist (exact menu options and inputs) rather than narration.

## Before recording

* Make the terminal font large (at least 18 pt) and use a dark theme.
* Run `make clean && make` just before the demo so a clean build is visible.
* Run through the demo once off camera so the inputs feel natural.

## Title

> B+ Trees in Cloud Databases: Indexed Record Search in C++ | BLAST 01 | Data Structures

## Description (paste into YouTube)

Update the chapter times to match your actual recording.

```
How do cloud databases find one record among millions in only a few reads? With B+ trees.

In this BLAST 01 project for CSC-6021 (Data Structures and Algorithms II, Dr. Victor Govindaswamy), I explain the B-tree / B+ tree, how search, range search, insertion (splits) and deletion (borrow/merge) work, their time and space complexity, and how B+ tree indexes are used in cloud databases such as Amazon DynamoDB, Amazon Aurora and Azure SQL.

I then demo "CloudVault Record Index", a menu-based C++ application I built from scratch that indexes cloud resource records with a B+ tree. It shows search paths, node splits and merges live, and runs a 1K to 1M record scalability experiment.

📂 GitHub (code, report, UML, diagrams): https://github.com/Pranavxoxo/BLAST01-BPlusTree-Cloud-Index
💼 LinkedIn post: <paste LinkedIn post link>

Chapters
0:00 Intro
0:30 Why cloud databases need indexes
1:40 What is a B+ tree
4:10 Search, insert and delete
7:00 Complexity
7:50 Real-world uses and the cloud
9:40 C++ design
11:05 Live demo
14:05 Scalability results and testing
15:35 Wrap-up

#BPlusTree #DataStructures #CPlusPlus #CloudComputing #Databases #Algorithms #ComputerScience
```

Tags: B+ tree, B-tree, data structures, C++, cloud computing, databases, database index, DynamoDB, algorithms, CSC-6021
