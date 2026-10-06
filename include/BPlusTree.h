// BPlusTree.h
// A B+ tree that indexes CloudRecord objects by recordId.
//
// Terminology used throughout this project:
//   order m  = the maximum number of children an internal node may have.
//              Every node therefore holds at most m - 1 keys.
//   internal = routing node. Holds separator keys and child pointers only.
//   leaf     = data node. Holds keys *and* the full records, and is linked to
//              the next leaf so the whole index can be read in sorted order.
//
// All leaves sit at the same depth, so every lookup reads exactly `height`
// nodes. In a real database each node is one fixed-size disk page, so the
// number of nodes visited is the number of page reads.

#ifndef BPLUS_TREE_H
#define BPLUS_TREE_H

#include "CloudRecord.h"

#include <iosfwd>
#include <string>
#include <vector>

// Cost of one lookup / range query, reported back to the menu.
struct SearchStats {
    int nodesVisited   = 0;           // simulated page reads
    int keyComparisons = 0;
    int leavesScanned  = 0;           // range queries only
    std::vector<std::string> path;    // the keys in each node visited, root first
};

class BPlusTree {
public:
    static const int MIN_ORDER = 3;
    static const int MAX_ORDER = 512;

    explicit BPlusTree(int order = 4);
    ~BPlusTree();

    // The tree owns raw node pointers, so copying is disabled.
    BPlusTree(const BPlusTree&) = delete;
    BPlusTree& operator=(const BPlusTree&) = delete;

    // ---- core operations ----
    bool insert(const CloudRecord& rec);                 // false if the ID already exists
    const CloudRecord* search(int key, SearchStats* stats = nullptr) const;
    bool update(const CloudRecord& rec);                 // false if the ID does not exist
    bool remove(int key);                                // false if the ID does not exist
    std::vector<CloudRecord> rangeQuery(int low, int high, SearchStats* stats = nullptr) const;
    std::vector<CloudRecord> allRecords() const;         // in key order, via the leaf chain
    void clear();

    // ---- inspection ----
    int  order()     const { return order_; }
    int  maxKeys()   const { return order_ - 1; }
    int  size()      const { return size_; }
    bool empty()     const { return size_ == 0; }
    int  height()    const;
    int  nodeCount() const;
    int  leafCount() const;
    int  totalSplits() const { return splits_; }
    int  totalMerges() const { return merges_; }
    int  totalBorrows() const { return borrows_; }

    void printTree(std::ostream& out) const;       // level-by-level view
    void printLeafChain(std::ostream& out) const;  // [keys] -> [keys] -> ... -> NULL

    // Human-readable description of the structural changes (splits, merges,
    // borrows, root changes) made by the most recent insert or remove.
    const std::vector<std::string>& lastChanges() const { return changes_; }

    // Checks every B+ tree invariant. Used by the test program.
    bool validate(std::string* error = nullptr) const;

private:
    struct Node {
        bool leaf;
        std::vector<int> keys;
        std::vector<Node*> children;       // internal nodes: keys.size() + 1 entries
        std::vector<CloudRecord> records;  // leaf nodes: parallel to keys
        Node* next = nullptr;              // leaf nodes: right neighbour in key order

        explicit Node(bool isLeaf) : leaf(isLeaf) {}
    };

    Node* root_ = nullptr;
    int order_;
    int size_ = 0;
    int splits_ = 0, merges_ = 0, borrows_ = 0;
    std::vector<std::string> changes_;

    int minKeys(const Node* node) const;
    static std::string keysToString(const Node* node);
    static int childIndex(const Node* node, int key, int* comparisons = nullptr);
    Node* findLeaf(int key, SearchStats* stats) const;

    bool insertInto(Node* node, const CloudRecord& rec, int& promotedKey, Node*& newSibling);
    void splitLeaf(Node* leaf, int& promotedKey, Node*& newSibling);
    void splitInternal(Node* node, int& promotedKey, Node*& newSibling);

    bool removeFrom(Node* node, int key);
    void fixUnderflow(Node* parent, size_t childPos);
    void borrowFromLeft(Node* parent, size_t childPos);
    void borrowFromRight(Node* parent, size_t childPos);
    void mergeChildren(Node* parent, size_t leftPos);

    void destroy(Node* node);
    int  countNodes(const Node* node) const;
    bool validateNode(const Node* node, int depth, int leafDepth, bool hasLow, int low,
                      bool hasHigh, int high, std::string* error) const;
};

#endif
