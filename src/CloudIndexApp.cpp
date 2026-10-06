// CloudIndexApp.cpp

#include "CloudIndexApp.h"
#include "InputUtils.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>

namespace {

const int MIN_ID = 1;
const int MAX_ID = 999999;

// 24 sample resources, deliberately inserted out of order so the tree has to
// split leaves and internal nodes while it is being built.
const std::vector<CloudRecord>& sampleRecords() {
    static const std::vector<CloudRecord> records = {
        {1040, "acme-retail",   "DB-Backup",       "us-east-1",    512.00, "2026-03-14"},
        {1012, "northwind",     "Object",          "eu-west-1",      0.75, "2026-01-09"},
        {1075, "globex",        "VM-Snapshot",     "us-west-2",    120.00, "2026-05-22"},
        {1003, "initech",       "Log-Archive",     "us-east-1",     48.30, "2026-01-02"},
        {1058, "acme-retail",   "Object",          "ap-south-1",     2.10, "2026-04-18"},
        {1021, "umbrella-labs", "Container-Image", "ca-central-1",   1.40, "2026-02-03"},
        {1090, "northwind",     "DB-Backup",       "eu-west-1",    860.00, "2026-06-30"},
        {1034, "globex",        "Object",          "us-east-1",      0.02, "2026-03-01"},
        {1066, "initech",       "VM-Snapshot",     "us-west-2",     80.00, "2026-05-02"},
        {1008, "acme-retail",   "Log-Archive",     "us-east-1",     15.60, "2026-01-05"},
        {1047, "umbrella-labs", "DB-Backup",       "ca-central-1", 240.00, "2026-03-29"},
        {1081, "globex",        "Container-Image", "ap-south-1",     0.95, "2026-06-11"},
        {1015, "initech",       "Object",          "eu-west-1",      3.20, "2026-01-20"},
        {1029, "northwind",     "VM-Snapshot",     "us-east-1",     64.00, "2026-02-21"},
        {1052, "acme-retail",   "Container-Image", "us-west-2",      1.10, "2026-04-07"},
        {1097, "umbrella-labs", "Log-Archive",     "eu-west-1",     22.80, "2026-07-15"},
        {1070, "northwind",     "Object",          "ca-central-1",   0.40, "2026-05-12"},
        {1005, "globex",        "DB-Backup",       "us-east-1",    300.00, "2026-01-03"},
        {1038, "initech",       "Container-Image", "ap-south-1",     0.85, "2026-03-10"},
        {1063, "umbrella-labs", "Object",          "us-west-2",      7.50, "2026-04-27"},
        {1086, "acme-retail",   "VM-Snapshot",     "eu-west-1",    150.00, "2026-06-19"},
        {1019, "globex",        "Log-Archive",     "ca-central-1",  33.00, "2026-01-28"},
        {1044, "northwind",     "Container-Image", "us-east-1",      2.60, "2026-03-20"},
        {1026, "initech",       "DB-Backup",       "us-west-2",    410.00, "2026-02-14"},
    };
    return records;
}

std::string withCommas(long long n) {
    std::string s = std::to_string(n);
    for (int i = static_cast<int>(s.size()) - 3; i > 0; i -= 3) s.insert(static_cast<size_t>(i), ",");
    return s;
}

void sectionTitle(const std::string& title) {
    std::cout << "\n--- " << title << " " << std::string(70 - std::min<size_t>(title.size(), 66), '-') << "\n";
}

}  // namespace

CloudIndexApp::CloudIndexApp(int order) : index_(new BPlusTree(order)) {}

// ---------------------------------------------------------------------------
// Menu loop
// ---------------------------------------------------------------------------

void CloudIndexApp::run() {
    printBanner();
    while (true) {
        printMenu();
        int choice = 0;
        try {
            choice = input::readInt("Select an option: ", 0, 12);
            switch (choice) {
                case 1:  loadSampleData();        break;
                case 2:  addRecord();             break;
                case 3:  searchRecord();          break;
                case 4:  rangeSearch();           break;
                case 5:  updateRecord();          break;
                case 6:  deleteRecord();          break;
                case 7:  showStructure();         break;
                case 8:  showAllRecords();        break;
                case 9:  showStatistics();        break;
                case 10: compareWithFullScan();   break;
                case 11: scalabilityExperiment(); break;
                case 12: rebuildWithNewOrder();   break;
                case 0:
                    std::cout << "\nShutting down the index. Goodbye.\n";
                    return;
            }
        } catch (const InputClosed&) {
            std::cout << "Input closed - exiting.\n";
            return;
        }
    }
}

void CloudIndexApp::printBanner() const {
    std::cout << "\n"
              << "==============================================================\n"
              << "   CloudVault Record Index  -  B+ Tree indexed record search\n"
              << "   BLAST 01  |  B+ Tree x Cloud / Databases\n"
              << "==============================================================\n"
              << "Every stored cloud resource has a numeric record ID. A B+ tree\n"
              << "of order " << index_->order() << " (max " << index_->maxKeys() << " keys per node)"
              << " maps each ID to its\n"
              << "metadata, so a lookup reads one node per level instead of\n"
              << "scanning every record.\n";
}

void CloudIndexApp::printMenu() const {
    std::string status = "  Index: " + std::to_string(index_->size()) + " records   order " +
                         std::to_string(index_->order()) + "   height " +
                         std::to_string(index_->height());
    status.resize(60, ' ');
    std::cout << "\n"
              << "+------------------------------------------------------------+\n"
              << "|" << status << "|\n"
              << "+------------------------------------------------------------+\n"
              << "|   1  Load sample cloud records                             |\n"
              << "|   2  Add a record                                          |\n"
              << "|   3  Search for a record by ID                             |\n"
              << "|   4  Range search (ID from .. to)                          |\n"
              << "|   5  Update a record's metadata                            |\n"
              << "|   6  Delete a record                                       |\n"
              << "|   7  Display B+ tree structure                             |\n"
              << "|   8  Display all records (leaf-chain scan)                 |\n"
              << "|   9  Index statistics                                      |\n"
              << "|  10  Index lookup vs full table scan                       |\n"
              << "|  11  Scalability experiment (1K .. 1M records)             |\n"
              << "|  12  Rebuild index with a different order                  |\n"
              << "|   0  Exit                                                  |\n"
              << "+------------------------------------------------------------+\n";
}

void CloudIndexApp::printChanges() const {
    for (const std::string& change : index_->lastChanges())
        std::cout << "    * " << change << "\n";
}

// ---------------------------------------------------------------------------
// Shared input helpers
// ---------------------------------------------------------------------------

int CloudIndexApp::readRecordId(const std::string& prompt) const {
    return input::readInt(prompt, MIN_ID, MAX_ID);
}

CloudRecord CloudIndexApp::readRecordFields(int recordId) const {
    CloudRecord rec;
    rec.recordId = recordId;
    rec.tenant = input::readName("  Tenant (account name): ", 15);
    rec.resourceType = validResourceTypes()[input::readChoice("Resource type:", validResourceTypes())];
    rec.region = validRegions()[input::readChoice("Region:", validRegions())];
    rec.sizeGB = input::readDouble("  Size in GB (0.01 - 100000): ", 0.01, 100000.0);
    rec.createdDate = input::readDate("  Created date (YYYY-MM-DD): ");
    return rec;
}

// ---------------------------------------------------------------------------
// Menu handlers
// ---------------------------------------------------------------------------

void CloudIndexApp::loadSampleData() {
    sectionTitle("Load sample cloud records");
    int added = 0, skipped = 0;
    int splitsBefore = index_->totalSplits();
    for (const CloudRecord& rec : sampleRecords()) {
        if (index_->insert(rec)) {
            arrivalOrder_.push_back(rec.recordId);
            ++added;
        } else {
            ++skipped;
        }
    }
    std::cout << "  Inserted " << added << " records";
    if (skipped) std::cout << " (" << skipped << " skipped: ID already indexed)";
    std::cout << ".\n  Node splits while loading: " << index_->totalSplits() - splitsBefore
              << "   Tree height: " << index_->height() << "\n";
}

void CloudIndexApp::addRecord() {
    sectionTitle("Add a record");
    int id = readRecordId("  New record ID (1-999999): ");
    if (index_->search(id)) {
        std::cout << "  ! Record " << id << " already exists. IDs are unique keys -\n"
                  << "    use option 5 to change its metadata instead.\n";
        return;
    }
    CloudRecord rec = readRecordFields(id);
    index_->insert(rec);
    arrivalOrder_.push_back(id);
    std::cout << "  Record " << id << " indexed. Structural changes:\n";
    if (index_->lastChanges().empty())
        std::cout << "    * none - the target leaf had room\n";
    printChanges();
}

void CloudIndexApp::searchRecord() {
    sectionTitle("Search by record ID");
    if (index_->empty()) {
        std::cout << "  The index is empty. Load sample data (1) or add a record (2) first.\n";
        return;
    }
    int id = readRecordId("  Record ID to find: ");
    SearchStats stats;
    const CloudRecord* rec = index_->search(id, &stats);

    std::cout << "  Path from root to leaf:\n";
    for (size_t i = 0; i < stats.path.size(); ++i)
        std::cout << "    " << std::string(i * 2, ' ') << "level " << i << ": " << stats.path[i] << "\n";
    if (rec)
        std::cout << "  FOUND  " << rec->summary() << "\n";
    else
        std::cout << "  NOT FOUND  No record has ID " << id << ".\n";
    std::cout << "  Cost: " << stats.nodesVisited << " node reads, " << stats.keyComparisons
              << " key comparisons.\n";
}

void CloudIndexApp::rangeSearch() {
    sectionTitle("Range search");
    if (index_->empty()) {
        std::cout << "  The index is empty. Load sample data (1) or add a record (2) first.\n";
        return;
    }
    int low = readRecordId("  From ID: ");
    int high = readRecordId("  To ID:   ");
    if (low > high) {
        std::swap(low, high);
        std::cout << "  (From was larger than To, so the bounds were swapped: " << low << " .. " << high << ")\n";
    }
    SearchStats stats;
    std::vector<CloudRecord> results = index_->rangeQuery(low, high, &stats);
    if (results.empty()) {
        std::cout << "  No records with IDs in " << low << " .. " << high << ".\n";
    } else {
        double total = 0;
        for (const CloudRecord& r : results) total += r.sizeGB;
        printRecordTable(std::cout, results);
        std::cout << "  " << results.size() << " record(s), " << std::fixed << std::setprecision(2)
                  << total << " GB in total.\n";
    }
    std::cout << "  Cost: " << stats.nodesVisited << " node reads to reach the first leaf, then "
              << stats.leavesScanned << " leaves walked along the next-pointers.\n";
}

void CloudIndexApp::updateRecord() {
    sectionTitle("Update a record");
    int id = readRecordId("  Record ID to update: ");
    const CloudRecord* existing = index_->search(id);
    if (!existing) {
        std::cout << "  ! No record has ID " << id << ". Nothing to update.\n";
        return;
    }
    std::cout << "  Current: " << existing->summary() << "\n"
              << "  Enter the new metadata (the ID stays the same):\n";
    CloudRecord rec = readRecordFields(id);
    index_->update(rec);
    std::cout << "  Updated: " << rec.summary() << "\n"
              << "  The key did not change, so the tree structure is untouched.\n";
}

void CloudIndexApp::deleteRecord() {
    sectionTitle("Delete a record");
    if (index_->empty()) {
        std::cout << "  The index is empty - there is nothing to delete.\n";
        return;
    }
    int id = readRecordId("  Record ID to delete: ");
    const CloudRecord* existing = index_->search(id);
    if (!existing) {
        std::cout << "  ! No record has ID " << id << ". Nothing was deleted.\n";
        return;
    }
    std::cout << "  Deleting " << existing->summary() << "\n";
    index_->remove(id);
    arrivalOrder_.erase(std::find(arrivalOrder_.begin(), arrivalOrder_.end(), id));
    std::cout << "  Record " << id << " removed. Structural changes:\n";
    if (index_->lastChanges().empty())
        std::cout << "    * none - the leaf still meets minimum fill\n";
    printChanges();
}

void CloudIndexApp::showStructure() const {
    sectionTitle("B+ tree structure (order " + std::to_string(index_->order()) + ")");
    index_->printTree(std::cout);
    if (!index_->empty()) {
        std::cout << "\n  Leaf chain (used for range scans):\n";
        index_->printLeafChain(std::cout);
    }
}

void CloudIndexApp::showAllRecords() const {
    sectionTitle("All records in ID order");
    if (index_->empty()) {
        std::cout << "  (empty index)\n";
        return;
    }
    printRecordTable(std::cout, index_->allRecords());
    std::cout << "  " << index_->size() << " records, read left-to-right across "
              << index_->leafCount() << " linked leaves.\n";
}

void CloudIndexApp::showStatistics() const {
    sectionTitle("Index statistics");
    int nodes = index_->nodeCount();
    int leaves = index_->leafCount();
    double fill = leaves ? 100.0 * index_->size() / (leaves * index_->maxKeys()) : 0.0;
    std::cout << std::left
              << "  Records indexed       : " << index_->size() << "\n"
              << "  Order (max children)  : " << index_->order() << "  (max " << index_->maxKeys()
              << " keys per node)\n"
              << "  Height (levels)       : " << index_->height() << "  = node reads per lookup\n"
              << "  Total nodes           : " << nodes << "  (" << nodes - leaves << " internal, "
              << leaves << " leaves)\n"
              << "  Average leaf fill     : " << std::fixed << std::setprecision(1) << fill << "%\n"
              << "  Splits / merges / borrows so far : " << index_->totalSplits() << " / "
              << index_->totalMerges() << " / " << index_->totalBorrows() << "\n";
    std::string err;
    std::cout << "  Structural self-check : "
              << (index_->validate(&err) ? "PASSED (sorted, balanced, all nodes within fill limits)"
                                         : "FAILED - " + err)
              << "\n" << std::right;
}

void CloudIndexApp::compareWithFullScan() {
    sectionTitle("Index lookup vs full table scan");
    if (index_->empty()) {
        std::cout << "  The index is empty. Load sample data (1) first.\n";
        return;
    }
    int id = readRecordId("  Record ID to look up: ");

    SearchStats stats;
    bool found = index_->search(id, &stats) != nullptr;

    // Without an index the rows sit in arrival order, so the only option is
    // to read them one by one until the ID turns up (or the table ends).
    auto it = std::find(arrivalOrder_.begin(), arrivalOrder_.end(), id);
    long long scanned = found ? (it - arrivalOrder_.begin()) + 1 : static_cast<long long>(arrivalOrder_.size());

    std::cout << "  Result: " << (found ? "found" : "not found") << "\n"
              << "  B+ tree index   : " << stats.nodesVisited << " node reads, "
              << stats.keyComparisons << " key comparisons\n"
              << "  Full table scan : " << scanned << " of " << arrivalOrder_.size()
              << " rows read (rows are stored in arrival order)\n";
    if (!found)
        std::cout << "  A missing ID is the worst case for a scan: every row must be read to\n"
                  << "  prove it is absent. The index proved it after " << stats.nodesVisited
                  << " node reads.\n";
    std::cout << "  Index cost tracks the tree height (" << index_->height()
              << "); scan cost tracks the row count (" << index_->size() << ").\n";
}

void CloudIndexApp::scalabilityExperiment() const {
    sectionTitle("Scalability experiment");
    std::cout << "  Builds separate B+ trees of 1K, 10K, 100K and 1M random record IDs and\n"
              << "  times lookups against a linear scan. Your current index is not changed.\n"
              << "  Real database pages hold a few hundred keys; try 4 and then 128.\n";
    int order = input::readInt("  Node order for the experiment (3-512): ", BPlusTree::MIN_ORDER,
                               BPlusTree::MAX_ORDER);

    const int sizes[] = {1000, 10000, 100000, 1000000};
    const int treeLookups = 100000;
    const int scanLookups = 200;
    std::mt19937 rng(2026);

    std::cout << "\n  " << std::left << std::setw(11) << "Records" << std::right
              << std::setw(7) << "Height" << std::setw(10) << "Nodes"
              << std::setw(12) << "Reads/find" << std::setw(12) << "Cmps/find"
              << std::setw(13) << "Index ns" << std::setw(13) << "Scan ns"
              << std::setw(11) << "Speed-up" << "\n  " << std::string(89, '-') << "\n";

    for (int n : sizes) {
        std::vector<int> ids(static_cast<size_t>(n));
        std::iota(ids.begin(), ids.end(), 1);
        std::shuffle(ids.begin(), ids.end(), rng);

        BPlusTree tree(order);
        for (int id : ids) tree.insert(CloudRecord(id, "t", "Object", "us-east-1", 1.0, "2026-01-01"));

        std::uniform_int_distribution<int> pick(1, n);
        std::vector<int> targets(treeLookups);
        for (int& t : targets) t = pick(rng);

        // Cost model: count reads and comparisons on a sample of lookups.
        long long reads = 0, comparisons = 0;
        for (int i = 0; i < 1000; ++i) {
            SearchStats s;
            tree.search(targets[static_cast<size_t>(i)], &s);
            reads += s.nodesVisited;
            comparisons += s.keyComparisons;
        }

        // Wall-clock timing without stats collection.
        using clock = std::chrono::steady_clock;
        long long sink = 0;
        auto t0 = clock::now();
        for (int t : targets) sink += tree.search(t)->recordId;
        auto t1 = clock::now();
        for (int i = 0; i < scanLookups; ++i) {
            int target = targets[static_cast<size_t>(i)];
            for (int id : ids)
                if (id == target) { sink += id; break; }
        }
        auto t2 = clock::now();

        double indexNs = std::chrono::duration<double, std::nano>(t1 - t0).count() / treeLookups;
        double scanNs = std::chrono::duration<double, std::nano>(t2 - t1).count() / scanLookups;

        std::cout << "  " << std::left << std::setw(11) << withCommas(n) << std::right
                  << std::setw(7) << tree.height() << std::setw(10) << withCommas(tree.nodeCount())
                  << std::fixed << std::setprecision(1)
                  << std::setw(12) << reads / 1000.0 << std::setw(12) << comparisons / 1000.0
                  << std::setw(13) << indexNs << std::setw(13) << scanNs
                  << std::setw(10) << std::setprecision(0) << scanNs / indexNs << "x\n";
        if (sink == -1) std::cout << "";   // keeps the timed loops from being optimised away
    }
    std::cout << "\n  Every 10x more records adds at most about one level to the tree, so\n"
              << "  index cost grows like log(n) while scan cost grows linearly with n.\n";
}

void CloudIndexApp::rebuildWithNewOrder() {
    sectionTitle("Rebuild index with a different order");
    std::cout << "  Current order: " << index_->order() << ". Small orders (3-5) make splits easy\n"
              << "  to see; large orders (100+) are closer to real database pages.\n";
    int order = input::readInt("  New order (3-512): ", BPlusTree::MIN_ORDER, BPlusTree::MAX_ORDER);

    std::unique_ptr<BPlusTree> rebuilt(new BPlusTree(order));
    for (int id : arrivalOrder_) rebuilt->insert(*index_->search(id));   // replay original arrival order
    int oldHeight = index_->height();
    index_ = std::move(rebuilt);
    std::cout << "  Rebuilt " << index_->size() << " records with order " << order
              << ". Height changed from " << oldHeight << " to " << index_->height() << ".\n";
}
