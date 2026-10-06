// CloudRecord.h
// One metadata record kept by a (simulated) cloud storage service.
// The B+ tree indexes these records by their numeric recordId.

#ifndef CLOUD_RECORD_H
#define CLOUD_RECORD_H

#include <iosfwd>
#include <string>
#include <vector>

struct CloudRecord {
    int         recordId = 0;      // primary key — the value the B+ tree is ordered by
    std::string tenant;            // customer account that owns the resource
    std::string resourceType;      // e.g. Object, VM-Snapshot, DB-Backup
    std::string region;            // data-centre region the bytes live in
    double      sizeGB = 0.0;      // stored size
    std::string createdDate;       // YYYY-MM-DD

    CloudRecord() = default;
    CloudRecord(int id, std::string tenantName, std::string type,
                std::string regionName, double size, std::string date);

    // One-line summary, used for search results.
    std::string summary() const;
};

// Pretty-prints a list of records as an aligned table.
void printRecordTable(std::ostream& out, const std::vector<CloudRecord>& records);
void printRecordTableHeader(std::ostream& out);
void printRecordRow(std::ostream& out, const CloudRecord& rec);

// Allowed values, shared by the menu (input validation) and the sample data.
const std::vector<std::string>& validRegions();
const std::vector<std::string>& validResourceTypes();

#endif
