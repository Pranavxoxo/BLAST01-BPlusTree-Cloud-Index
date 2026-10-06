// test_bplustree.cpp
// Automated checks for the B+ tree. Every operation is mirrored in std::map
// (the reference answer) and the tree's invariants are re-validated after
// each step, across several orders.
//
// Build and run:  make test

#include "BPlusTree.h"

#include <cstdlib>
#include <iostream>
#include <map>
#include <random>
#include <string>

static int failures = 0;

#define CHECK(cond, msg)                                                    \
    do {                                                                    \
        if (!(cond)) {                                                      \
            std::cerr << "  FAIL: " << msg << " (line " << __LINE__ << ")\n"; \
            ++failures;                                                     \
        }                                                                   \
    } while (0)

static CloudRecord makeRecord(int id) {
    return CloudRecord(id, "tenant" + std::to_string(id % 7), "Object", "us-east-1",
                       id * 0.5, "2026-01-01");
}

static void checkValid(const BPlusTree& tree, const std::string& context) {
    std::string err;
    bool ok = tree.validate(&err);
    CHECK(ok, context + ": " + err);
}

static void testEmptyTree() {
    BPlusTree tree(4);
    CHECK(tree.empty(), "new tree should be empty");
    CHECK(tree.height() == 0, "empty tree height should be 0");
    CHECK(tree.search(10) == nullptr, "search on empty tree");
    CHECK(!tree.remove(10), "remove on empty tree");
    CHECK(tree.rangeQuery(0, 100).empty(), "range on empty tree");
    checkValid(tree, "empty");
}

static void testDuplicatesAndUpdate() {
    BPlusTree tree(4);
    CHECK(tree.insert(makeRecord(5)), "first insert");
    CHECK(!tree.insert(makeRecord(5)), "duplicate insert must be rejected");
    CHECK(tree.size() == 1, "size after duplicate");

    CloudRecord changed = makeRecord(5);
    changed.tenant = "renamed";
    CHECK(tree.update(changed), "update existing");
    CHECK(tree.search(5) && tree.search(5)->tenant == "renamed", "update applied");
    CHECK(!tree.update(makeRecord(6)), "update missing ID must fail");
    checkValid(tree, "duplicates");
}

static void testRootSplitAndCollapse() {
    BPlusTree tree(4);   // max 3 keys per node
    for (int k = 1; k <= 3; ++k) tree.insert(makeRecord(k));
    CHECK(tree.height() == 1, "3 keys fit in a single leaf");
    tree.insert(makeRecord(4));
    CHECK(tree.height() == 2, "4th key splits the root leaf");
    CHECK(tree.leafCount() == 2, "two leaves after first split");
    tree.remove(4);
    tree.remove(3);
    CHECK(tree.height() == 1, "merge collapses the root back to one leaf");
    checkValid(tree, "root split/collapse");
}

static void testRangeQuery() {
    BPlusTree tree(5);
    for (int k = 0; k < 200; k += 2) tree.insert(makeRecord(k));   // even keys only
    std::vector<CloudRecord> r = tree.rangeQuery(11, 21);
    CHECK(r.size() == 5, "range 11..21 over even keys gives 12,14,16,18,20");
    CHECK(!r.empty() && r.front().recordId == 12 && r.back().recordId == 20, "range endpoints");
    CHECK(tree.rangeQuery(21, 11).empty(), "reversed range is empty");
    CHECK(tree.rangeQuery(500, 600).empty(), "range past the end is empty");
    CHECK(tree.rangeQuery(-50, 1000).size() == 100, "range covering everything");
}

static void testRandomAgainstMap(int order, unsigned seed) {
    BPlusTree tree(order);
    std::map<int, CloudRecord> reference;
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> keyDist(0, 600);
    std::uniform_int_distribution<int> opDist(0, 9);

    for (int step = 0; step < 4000; ++step) {
        int key = keyDist(rng);
        int op = opDist(rng);
        std::string ctx = "order " + std::to_string(order) + " step " + std::to_string(step);

        if (op < 5) {
            bool expected = reference.find(key) == reference.end();
            bool got = tree.insert(makeRecord(key));
            CHECK(got == expected, ctx + " insert result");
            if (expected) reference[key] = makeRecord(key);
        } else if (op < 8) {
            bool expected = reference.erase(key) == 1;
            CHECK(tree.remove(key) == expected, ctx + " remove result");
        } else {
            const CloudRecord* found = tree.search(key);
            CHECK((found != nullptr) == (reference.count(key) == 1), ctx + " search result");
        }

        CHECK(tree.size() == static_cast<int>(reference.size()), ctx + " size");
        checkValid(tree, ctx);
        if (failures > 0) return;   // stop at the first broken state
    }

    // Leaf-chain order must equal the map's order.
    std::vector<CloudRecord> all = tree.allRecords();
    auto it = reference.begin();
    for (const CloudRecord& rec : all) {
        CHECK(it != reference.end() && rec.recordId == it->first, "leaf chain matches map");
        ++it;
    }

    // Drain the tree completely; it must shrink back to empty cleanly.
    for (const auto& kv : reference) tree.remove(kv.first);
    CHECK(tree.empty() && tree.height() == 0, "tree empty after deleting everything");
    checkValid(tree, "drained");
}

int main() {
    std::cout << "Running B+ tree tests...\n";
    testEmptyTree();
    testDuplicatesAndUpdate();
    testRootSplitAndCollapse();
    testRangeQuery();
    for (int order = 3; order <= 9; ++order)
        for (unsigned seed = 1; seed <= 3; ++seed) testRandomAgainstMap(order, seed);
    testRandomAgainstMap(64, 42);

    if (failures == 0) {
        std::cout << "All tests passed.\n";
        return EXIT_SUCCESS;
    }
    std::cout << failures << " check(s) failed.\n";
    return EXIT_FAILURE;
}
