// CloudIndexApp.h
// Menu-driven front end: a cloud provider's metadata index that finds stored
// resources (objects, snapshots, backups...) by record ID using a B+ tree.

#ifndef CLOUD_INDEX_APP_H
#define CLOUD_INDEX_APP_H

#include "BPlusTree.h"

#include <memory>
#include <vector>

class CloudIndexApp {
public:
    explicit CloudIndexApp(int order = 4);
    void run();

private:
    std::unique_ptr<BPlusTree> index_;

    // Record IDs in the order they arrived. This models how rows sit in an
    // unindexed "heap" table, and is what a full scan has to read through.
    std::vector<int> arrivalOrder_;

    void printBanner() const;
    void printMenu() const;
    void printChanges() const;

    // One handler per menu option.
    void loadSampleData();
    void addRecord();
    void searchRecord();
    void rangeSearch();
    void updateRecord();
    void deleteRecord();
    void showStructure() const;
    void showAllRecords() const;
    void showStatistics() const;
    void compareWithFullScan();
    void scalabilityExperiment() const;
    void rebuildWithNewOrder();

    int readRecordId(const std::string& prompt) const;
    CloudRecord readRecordFields(int recordId) const;
};

#endif
