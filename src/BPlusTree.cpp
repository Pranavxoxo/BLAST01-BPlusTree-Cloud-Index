// BPlusTree.cpp

#include "BPlusTree.h"

#include <algorithm>
#include <iostream>
#include <queue>
#include <sstream>
#include <stdexcept>

// ---------------------------------------------------------------------------
// Construction / destruction
// ---------------------------------------------------------------------------

BPlusTree::BPlusTree(int order) : order_(order) {
    if (order < MIN_ORDER || order > MAX_ORDER)
        throw std::invalid_argument("B+ tree order must be between 3 and 512");
}

BPlusTree::~BPlusTree() { destroy(root_); }

void BPlusTree::clear() {
    destroy(root_);
    root_ = nullptr;
    size_ = 0;
    splits_ = merges_ = borrows_ = 0;
    changes_.clear();
}

// Postorder delete: children are freed before their parent.
void BPlusTree::destroy(Node* node) {
    if (!node) return;
    if (!node->leaf)
        for (Node* child : node->children) destroy(child);
    delete node;
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

// Minimum fill for every node except the root.
//   leaf:     ceil((m - 1) / 2) keys
//   internal: ceil(m / 2) children, i.e. ceil(m / 2) - 1 keys
int BPlusTree::minKeys(const Node* node) const {
    if (node->leaf) return order_ / 2;
    return (order_ + 1) / 2 - 1;
}

std::string BPlusTree::keysToString(const Node* node) {
    std::ostringstream os;
    os << "[";
    for (size_t i = 0; i < node->keys.size(); ++i) {
        if (i) os << " ";
        os << node->keys[i];
    }
    os << "]";
    return os.str();
}

// Which child of an internal node a key belongs in.
// Separator keys[i] means: children[i] holds keys < keys[i],
// children[i + 1] holds keys >= keys[i].
int BPlusTree::childIndex(const Node* node, int key, int* comparisons) {
    int i = 0;
    int n = static_cast<int>(node->keys.size());
    while (i < n) {
        if (comparisons) ++*comparisons;
        if (key < node->keys[i]) break;
        ++i;
    }
    return i;
}

// Walks from the root down to the leaf where `key` lives (or would live).
BPlusTree::Node* BPlusTree::findLeaf(int key, SearchStats* stats) const {
    Node* node = root_;
    while (node) {
        if (stats) {
            ++stats->nodesVisited;
            stats->path.push_back((node->leaf ? "leaf " : "internal ") + keysToString(node));
        }
        if (node->leaf) return node;
        node = node->children[childIndex(node, key, stats ? &stats->keyComparisons : nullptr)];
    }
    return nullptr;
}

// ---------------------------------------------------------------------------
// Search
// ---------------------------------------------------------------------------

const CloudRecord* BPlusTree::search(int key, SearchStats* stats) const {
    Node* leaf = findLeaf(key, stats);
    if (!leaf) return nullptr;
    for (size_t i = 0; i < leaf->keys.size(); ++i) {
        if (stats) ++stats->keyComparisons;
        if (leaf->keys[i] == key) return &leaf->records[i];
        if (leaf->keys[i] > key) break;   // keys are sorted, so it is not here
    }
    return nullptr;
}

// Range query: one root-to-leaf descent to find `low`, then walk sideways
// along the leaf chain until a key passes `high`. This sideways walk is the
// main reason databases use B+ trees rather than plain B-trees.
std::vector<CloudRecord> BPlusTree::rangeQuery(int low, int high, SearchStats* stats) const {
    std::vector<CloudRecord> result;
    if (low > high) return result;
    Node* leaf = findLeaf(low, stats);
    while (leaf) {
        if (stats) ++stats->leavesScanned;
        for (size_t i = 0; i < leaf->keys.size(); ++i) {
            if (stats) ++stats->keyComparisons;
            if (leaf->keys[i] > high) return result;
            if (leaf->keys[i] >= low) result.push_back(leaf->records[i]);
        }
        leaf = leaf->next;
    }
    return result;
}

std::vector<CloudRecord> BPlusTree::allRecords() const {
    std::vector<CloudRecord> result;
    result.reserve(size_);
    Node* node = root_;
    while (node && !node->leaf) node = node->children.front();
    for (; node; node = node->next)
        result.insert(result.end(), node->records.begin(), node->records.end());
    return result;
}

bool BPlusTree::update(const CloudRecord& rec) {
    Node* leaf = findLeaf(rec.recordId, nullptr);
    if (!leaf) return false;
    for (size_t i = 0; i < leaf->keys.size(); ++i) {
        if (leaf->keys[i] == rec.recordId) {
            leaf->records[i] = rec;   // key is unchanged, so the tree shape is too
            return true;
        }
    }
    return false;
}

// ---------------------------------------------------------------------------
// Insertion
// ---------------------------------------------------------------------------

bool BPlusTree::insert(const CloudRecord& rec) {
    changes_.clear();

    if (!root_) {
        root_ = new Node(true);
        root_->keys.push_back(rec.recordId);
        root_->records.push_back(rec);
        size_ = 1;
        changes_.push_back("Tree was empty: created root leaf " + keysToString(root_));
        return true;
    }

    int promotedKey = 0;
    Node* newSibling = nullptr;
    if (!insertInto(root_, rec, promotedKey, newSibling)) return false;

    // The root itself split, so the tree grows one level taller. This is the
    // only way a B+ tree ever gains height, which is why it stays balanced.
    if (newSibling) {
        Node* newRoot = new Node(false);
        newRoot->keys.push_back(promotedKey);
        newRoot->children.push_back(root_);
        newRoot->children.push_back(newSibling);
        root_ = newRoot;
        changes_.push_back("Root split: new root " + keysToString(root_) +
                           ", height is now " + std::to_string(height()));
    }
    ++size_;
    return true;
}

// Recursive insert. If `node` overflows it splits, and the caller receives the
// separator key and the new right-hand node to add to itself.
bool BPlusTree::insertInto(Node* node, const CloudRecord& rec, int& promotedKey,
                           Node*& newSibling) {
    newSibling = nullptr;

    if (node->leaf) {
        auto it = std::lower_bound(node->keys.begin(), node->keys.end(), rec.recordId);
        if (it != node->keys.end() && *it == rec.recordId) return false;   // duplicate
        size_t pos = static_cast<size_t>(it - node->keys.begin());
        node->keys.insert(it, rec.recordId);
        node->records.insert(node->records.begin() + pos, rec);
        if (static_cast<int>(node->keys.size()) > maxKeys())
            splitLeaf(node, promotedKey, newSibling);
        return true;
    }

    size_t i = static_cast<size_t>(childIndex(node, rec.recordId));
    int childKey = 0;
    Node* childSibling = nullptr;
    if (!insertInto(node->children[i], rec, childKey, childSibling)) return false;

    if (childSibling) {
        node->keys.insert(node->keys.begin() + i, childKey);
        node->children.insert(node->children.begin() + i + 1, childSibling);
        if (static_cast<int>(node->keys.size()) > maxKeys())
            splitInternal(node, promotedKey, newSibling);
    }
    return true;
}

// Leaf split: the right half moves to a new leaf and its first key is
// *copied* up. The key stays in the leaf because leaves hold the data.
void BPlusTree::splitLeaf(Node* leaf, int& promotedKey, Node*& newSibling) {
    std::string before = keysToString(leaf);
    size_t mid = leaf->keys.size() / 2;

    Node* right = new Node(true);
    right->keys.assign(leaf->keys.begin() + mid, leaf->keys.end());
    right->records.assign(leaf->records.begin() + mid, leaf->records.end());
    leaf->keys.erase(leaf->keys.begin() + mid, leaf->keys.end());
    leaf->records.erase(leaf->records.begin() + mid, leaf->records.end());

    right->next = leaf->next;   // splice the new leaf into the chain
    leaf->next = right;

    promotedKey = right->keys.front();
    newSibling = right;
    ++splits_;
    changes_.push_back("Leaf " + before + " overflowed: split into " + keysToString(leaf) +
                       " + " + keysToString(right) + ", key " +
                       std::to_string(promotedKey) + " copied up");
}

// Internal split: the middle key *moves* up and is removed from both halves,
// because internal keys are only signposts.
void BPlusTree::splitInternal(Node* node, int& promotedKey, Node*& newSibling) {
    std::string before = keysToString(node);
    size_t mid = node->keys.size() / 2;
    promotedKey = node->keys[mid];

    Node* right = new Node(false);
    right->keys.assign(node->keys.begin() + mid + 1, node->keys.end());
    right->children.assign(node->children.begin() + mid + 1, node->children.end());
    node->keys.erase(node->keys.begin() + mid, node->keys.end());
    node->children.erase(node->children.begin() + mid + 1, node->children.end());

    newSibling = right;
    ++splits_;
    changes_.push_back("Internal " + before + " overflowed: split into " + keysToString(node) +
                       " + " + keysToString(right) + ", key " +
                       std::to_string(promotedKey) + " moved up");
}

// ---------------------------------------------------------------------------
// Deletion
// ---------------------------------------------------------------------------

bool BPlusTree::remove(int key) {
    changes_.clear();
    if (!root_ || !removeFrom(root_, key)) return false;
    --size_;

    if (root_->leaf && root_->keys.empty()) {
        delete root_;
        root_ = nullptr;
        changes_.push_back("Last record removed: tree is now empty");
    } else if (!root_->leaf && root_->keys.empty()) {
        // A merge emptied the root. Its only child becomes the new root and
        // the tree shrinks one level — the mirror image of a root split.
        Node* oldRoot = root_;
        root_ = root_->children.front();
        oldRoot->children.clear();
        delete oldRoot;
        changes_.push_back("Root emptied: its child " + keysToString(root_) +
                           " is the new root, height is now " + std::to_string(height()));
    }
    return true;
}

// Recursive delete. After a child loses a key, the parent checks whether the
// child fell below minimum fill and repairs it by borrowing or merging.
//
// Separator keys in internal nodes are not rewritten when the key they copy is
// deleted. They are routing values only, and the rule "left < separator <=
// right" still holds, so lookups stay correct. Many production engines are
// equally lazy here.
bool BPlusTree::removeFrom(Node* node, int key) {
    if (node->leaf) {
        auto it = std::lower_bound(node->keys.begin(), node->keys.end(), key);
        if (it == node->keys.end() || *it != key) return false;
        size_t pos = static_cast<size_t>(it - node->keys.begin());
        node->keys.erase(it);
        node->records.erase(node->records.begin() + pos);
        return true;
    }

    size_t i = static_cast<size_t>(childIndex(node, key));
    Node* child = node->children[i];
    if (!removeFrom(child, key)) return false;
    if (static_cast<int>(child->keys.size()) < minKeys(child)) fixUnderflow(node, i);
    return true;
}

// Repair order: borrow from a sibling that can spare a key; otherwise merge.
void BPlusTree::fixUnderflow(Node* parent, size_t childPos) {
    Node* left  = childPos > 0 ? parent->children[childPos - 1] : nullptr;
    Node* right = childPos + 1 < parent->children.size() ? parent->children[childPos + 1] : nullptr;

    if (left && static_cast<int>(left->keys.size()) > minKeys(left))
        borrowFromLeft(parent, childPos);
    else if (right && static_cast<int>(right->keys.size()) > minKeys(right))
        borrowFromRight(parent, childPos);
    else if (left)
        mergeChildren(parent, childPos - 1);
    else
        mergeChildren(parent, childPos);
}

void BPlusTree::borrowFromLeft(Node* parent, size_t childPos) {
    Node* child = parent->children[childPos];
    Node* left  = parent->children[childPos - 1];
    ++borrows_;

    if (child->leaf) {
        child->keys.insert(child->keys.begin(), left->keys.back());
        child->records.insert(child->records.begin(), left->records.back());
        left->keys.pop_back();
        left->records.pop_back();
        parent->keys[childPos - 1] = child->keys.front();
        changes_.push_back("Leaf underflow: borrowed key " + std::to_string(child->keys.front()) +
                           " from left sibling, giving " + keysToString(left) + " + " +
                           keysToString(child));
    } else {
        // Rotate right through the parent: separator comes down, left's last key goes up.
        child->keys.insert(child->keys.begin(), parent->keys[childPos - 1]);
        child->children.insert(child->children.begin(), left->children.back());
        parent->keys[childPos - 1] = left->keys.back();
        left->keys.pop_back();
        left->children.pop_back();
        changes_.push_back("Internal underflow: rotated a key from left sibling through parent, giving " +
                           keysToString(left) + " + " + keysToString(child));
    }
}

void BPlusTree::borrowFromRight(Node* parent, size_t childPos) {
    Node* child = parent->children[childPos];
    Node* right = parent->children[childPos + 1];
    ++borrows_;

    if (child->leaf) {
        int moved = right->keys.front();
        child->keys.push_back(moved);
        child->records.push_back(right->records.front());
        right->keys.erase(right->keys.begin());
        right->records.erase(right->records.begin());
        parent->keys[childPos] = right->keys.front();
        changes_.push_back("Leaf underflow: borrowed key " + std::to_string(moved) +
                           " from right sibling, giving " + keysToString(child) + " + " +
                           keysToString(right));
    } else {
        child->keys.push_back(parent->keys[childPos]);
        child->children.push_back(right->children.front());
        parent->keys[childPos] = right->keys.front();
        right->keys.erase(right->keys.begin());
        right->children.erase(right->children.begin());
        changes_.push_back("Internal underflow: rotated a key from right sibling through parent, giving " +
                           keysToString(child) + " + " + keysToString(right));
    }
}

// Merges children[leftPos + 1] into children[leftPos] and drops the separator
// between them from the parent. The parent may underflow in turn, which the
// recursion one level up handles.
void BPlusTree::mergeChildren(Node* parent, size_t leftPos) {
    Node* left  = parent->children[leftPos];
    Node* right = parent->children[leftPos + 1];
    int separator = parent->keys[leftPos];
    ++merges_;

    if (left->leaf) {
        left->keys.insert(left->keys.end(), right->keys.begin(), right->keys.end());
        left->records.insert(left->records.end(), right->records.begin(), right->records.end());
        left->next = right->next;   // unlink the absorbed leaf from the chain
        changes_.push_back("Leaf underflow: merged two leaves into " + keysToString(left) +
                           ", separator " + std::to_string(separator) + " removed from parent");
    } else {
        // Internal merge pulls the separator down between the two halves.
        left->keys.push_back(separator);
        left->keys.insert(left->keys.end(), right->keys.begin(), right->keys.end());
        left->children.insert(left->children.end(), right->children.begin(), right->children.end());
        right->children.clear();
        changes_.push_back("Internal underflow: merged with sibling into " + keysToString(left) +
                           ", separator " + std::to_string(separator) + " pulled down");
    }

    parent->keys.erase(parent->keys.begin() + leftPos);
    parent->children.erase(parent->children.begin() + leftPos + 1);
    delete right;
}

// ---------------------------------------------------------------------------
// Inspection
// ---------------------------------------------------------------------------

int BPlusTree::height() const {
    int h = 0;
    for (Node* node = root_; node; node = node->leaf ? nullptr : node->children.front()) ++h;
    return h;
}

int BPlusTree::countNodes(const Node* node) const {
    if (!node) return 0;
    int total = 1;
    if (!node->leaf)
        for (const Node* child : node->children) total += countNodes(child);
    return total;
}

int BPlusTree::nodeCount() const { return countNodes(root_); }

int BPlusTree::leafCount() const {
    Node* node = root_;
    while (node && !node->leaf) node = node->children.front();
    int count = 0;
    for (; node; node = node->next) ++count;
    return count;
}

void BPlusTree::printTree(std::ostream& out) const {
    if (!root_) {
        out << "  (empty index)\n";
        return;
    }
    const size_t wrapWidth = 96;
    std::queue<const Node*> current;
    current.push(root_);
    int level = 0;
    int h = height();

    while (!current.empty()) {
        std::queue<const Node*> nextLevel;
        std::string label = level == 0 ? "root" : (level == h - 1 ? "leaves" : "internal");
        std::string prefix = "  L" + std::to_string(level) + " " + label;
        prefix.resize(16, ' ');
        prefix += ": ";

        std::string line = prefix;
        while (!current.empty()) {
            const Node* node = current.front();
            current.pop();
            std::string text = keysToString(node);
            if (line.size() + text.size() + 2 > wrapWidth && line.size() > prefix.size()) {
                out << line << "\n";
                line = std::string(prefix.size(), ' ');
            }
            line += text + "  ";
            if (!node->leaf)
                for (const Node* child : node->children) nextLevel.push(child);
        }
        out << line << "\n";
        current = nextLevel;
        ++level;
    }
}

void BPlusTree::printLeafChain(std::ostream& out) const {
    Node* node = root_;
    while (node && !node->leaf) node = node->children.front();
    if (!node) {
        out << "  (empty index)\n";
        return;
    }
    std::string line = "  ";
    for (; node; node = node->next) {
        std::string text = keysToString(node) + " -> ";
        if (line.size() + text.size() > 96) {
            out << line << "\n";
            line = "    ";
        }
        line += text;
    }
    out << line << "NULL\n";
}

// ---------------------------------------------------------------------------
// Invariant checking
// ---------------------------------------------------------------------------

bool BPlusTree::validate(std::string* error) const {
    auto fail = [error](const std::string& msg) {
        if (error) *error = msg;
        return false;
    };

    if (!root_) return size_ == 0 ? true : fail("root is null but size is non-zero");
    if (!root_->leaf && root_->keys.empty()) return fail("internal root has no keys");

    int leafDepth = height() - 1;
    if (!validateNode(root_, 0, leafDepth, false, 0, false, 0, error)) return false;

    // The leaf chain must visit every key exactly once, in ascending order.
    Node* node = root_;
    while (!node->leaf) node = node->children.front();
    int seen = 0;
    bool first = true;
    int prev = 0;
    for (; node; node = node->next) {
        for (int k : node->keys) {
            if (!first && k <= prev) return fail("leaf chain is not in ascending order");
            prev = k;
            first = false;
            ++seen;
        }
    }
    if (seen != size_) return fail("leaf chain holds " + std::to_string(seen) +
                                   " keys but size is " + std::to_string(size_));
    return true;
}

bool BPlusTree::validateNode(const Node* node, int depth, int leafDepth, bool hasLow, int low,
                             bool hasHigh, int high, std::string* error) const {
    auto fail = [error, node](const std::string& msg) {
        if (error) *error = msg + " at node " + keysToString(node);
        return false;
    };

    int n = static_cast<int>(node->keys.size());
    if (n > maxKeys()) return fail("too many keys");
    if (node != root_ && n < minKeys(node)) return fail("too few keys");

    for (int i = 0; i < n; ++i) {
        if (i > 0 && node->keys[i - 1] >= node->keys[i]) return fail("keys not sorted");
        if (hasLow && node->keys[i] < low) return fail("key below subtree range");
        if (hasHigh && node->keys[i] >= high) return fail("key above subtree range");
    }

    if (node->leaf) {
        if (depth != leafDepth) return fail("leaves are not all at the same depth");
        if (node->records.size() != node->keys.size()) return fail("records/keys mismatch");
        for (int i = 0; i < n; ++i)
            if (node->records[i].recordId != node->keys[i]) return fail("record ID differs from key");
        return true;
    }

    if (static_cast<int>(node->children.size()) != n + 1) return fail("wrong child count");
    for (int i = 0; i <= n; ++i) {
        bool childHasLow  = i > 0 ? true : hasLow;
        int  childLow     = i > 0 ? node->keys[i - 1] : low;
        bool childHasHigh = i < n ? true : hasHigh;
        int  childHigh    = i < n ? node->keys[i] : high;
        if (!validateNode(node->children[i], depth + 1, leafDepth, childHasLow, childLow,
                          childHasHigh, childHigh, error))
            return false;
    }
    return true;
}
