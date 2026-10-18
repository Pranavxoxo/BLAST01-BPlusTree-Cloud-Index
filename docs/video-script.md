# YouTube presentation

The video is recorded straight from **[`BLAST01-Video-Presentation.pptx`](BLAST01-Video-Presentation.pptx)** with PowerPoint's built-in recorder. It has 19 slides and runs about **12 minutes**. Everything to say is in each slide's **speaker notes**, which start with a target time such as `[~40 seconds]`. The times add up to 12:00.

There is no live demo. Slides 13–17 (*Step 1* to *Step 5*) walk through real program output, captured from `docs/sample-output.txt`: load and display, search and range search, insert with splits, delete with merges, and edge cases with the self-check. Each step has three numbered callouts that match the order of the narration.

## Recording with PowerPoint

1. Open the deck and choose **Record** (top right), or **Slide Show → Record Slide Show**.
2. The recorder shows each slide's notes at the top of the screen. Read them at a relaxed pace and advance with the arrow key.
3. Export with **File → Export → Create a Video** (1080p) and upload the .mp4 to YouTube.

## Title

> B+ Trees in Cloud Databases: Indexed Record Search in C++ | BLAST 01 | Data Structures

## Description (paste into YouTube)

Update the chapter times to match your actual recording.

```
How do cloud databases find one record among millions in only a few reads? With B+ trees.

In this BLAST 01 project for CSC-6021 (Data Structures and Algorithms II, Dr. Victor Govindaswamy), I explain the B-tree / B+ tree, how search, range search, insertion (splits) and deletion (borrow/merge) work, their time and space complexity, and how B+ tree indexes are used in cloud databases such as Amazon DynamoDB, Amazon Aurora and Azure SQL.

I then walk through the real output of "CloudVault Record Index", a menu-based C++ application I built from scratch that indexes cloud resource records with a B+ tree. It prints search paths, node splits and merges as they happen, and runs a 1K to 1M record scalability experiment.

📂 GitHub (code, report, UML, diagrams): https://github.com/Pranavxoxo/BLAST01-BPlusTree-Cloud-Index
💼 LinkedIn post: <paste LinkedIn post link>

Chapters
0:00 Intro
0:25 Why cloud databases need indexes
1:05 What is a B+ tree
2:25 Search, insert and delete
4:35 Complexity
5:10 Real-world uses and the cloud
6:25 The C++ application and design
7:25 Output walkthrough
10:50 Scalability results
11:35 Wrap-up

#BPlusTree #DataStructures #CPlusPlus #CloudComputing #Databases #Algorithms #ComputerScience
```

Tags: B+ tree, B-tree, data structures, C++, cloud computing, databases, database index, DynamoDB, algorithms, CSC-6021
