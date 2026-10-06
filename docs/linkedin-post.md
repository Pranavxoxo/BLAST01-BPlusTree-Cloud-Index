# LinkedIn post draft

Edit this into your own voice before posting. The part people engage with most is something specific that surprised *you*, so replace the bracketed line with a real moment from building this. Tag Dr. Govindaswamy using LinkedIn's @-mention so the tag links to his profile.

---

How does a cloud database find one record among millions in only 3 reads? 🌲☁️

For BLAST 01 in CSC-6021 (Data Structures and Algorithms II) with @Dr. Victor Govindaswamy, I researched the **B+ tree** and built **CloudVault Record Index**, a menu-based C++ application that indexes cloud resource records (objects, snapshots, backups) with a B+ tree I wrote from scratch.

What I learned:

🔹 **Wide beats tall.** Each B+ tree node holds many keys, and in a real database each node is one storage page. In my experiment, going from 1,000 to 1,000,000 records added only one level to the tree.

🔹 **Data lives only in the leaves, and the leaves are linked.** That one design choice makes range queries ("all of tenant X's backups between two IDs") a single descent followed by a sequential scan.

🔹 **Balance comes for free.** The tree only grows taller when the root splits, so every leaf stays at the same depth. Deletes keep nodes at least half full by borrowing from a sibling or merging with one.

🔹 **Fan-out matters even in RAM.** With 1M records, order 4 needed 13 node reads per lookup and order 128 needed only 3, and was about 3× faster. On cloud storage, each of those reads would be a network round trip.

This is the same idea running inside MySQL, PostgreSQL, Azure SQL and SQLite, and inside the storage nodes of Amazon DynamoDB.

[Add 1–2 sentences about what challenged you, e.g. getting the borrow/merge logic right on delete, or seeing a cascading split happen live in the program.]

📂 Code, report, UML and diagrams: https://github.com/Pranavxoxo/BLAST01-BPlusTree-Cloud-Index
▶️ Video walkthrough: <paste YouTube link>

#DataStructures #BPlusTree #CPlusPlus #CloudComputing #Databases #Algorithms #ComputerScience #SoftwareEngineering
