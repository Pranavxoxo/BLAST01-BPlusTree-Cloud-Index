// CloudRecord.cpp

#include "CloudRecord.h"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <utility>

CloudRecord::CloudRecord(int id, std::string tenantName, std::string type,
                         std::string regionName, double size, std::string date)
    : recordId(id),
      tenant(std::move(tenantName)),
      resourceType(std::move(type)),
      region(std::move(regionName)),
      sizeGB(size),
      createdDate(std::move(date)) {}

std::string CloudRecord::summary() const {
    std::ostringstream os;
    os << "#" << recordId << "  " << tenant << " | " << resourceType << " | "
       << region << " | " << std::fixed << std::setprecision(2) << sizeGB
       << " GB | created " << createdDate;
    return os.str();
}

void printRecordTableHeader(std::ostream& out) {
    out << "  " << std::left
        << std::setw(8)  << "ID"
        << std::setw(16) << "Tenant"
        << std::setw(17) << "Resource"
        << std::setw(15) << "Region"
        << std::right << std::setw(10) << "Size(GB)"
        << "  " << "Created" << "\n";
    out << "  " << std::string(76, '-') << "\n";
}

void printRecordRow(std::ostream& out, const CloudRecord& rec) {
    out << "  " << std::left
        << std::setw(8)  << rec.recordId
        << std::setw(16) << rec.tenant
        << std::setw(17) << rec.resourceType
        << std::setw(15) << rec.region
        << std::right << std::setw(10) << std::fixed << std::setprecision(2) << rec.sizeGB
        << "  " << rec.createdDate << "\n";
}

void printRecordTable(std::ostream& out, const std::vector<CloudRecord>& records) {
    printRecordTableHeader(out);
    for (const CloudRecord& rec : records) printRecordRow(out, rec);
}

const std::vector<std::string>& validRegions() {
    static const std::vector<std::string> regions = {
        "us-east-1", "us-west-2", "ca-central-1", "eu-west-1", "ap-south-1"};
    return regions;
}

const std::vector<std::string>& validResourceTypes() {
    static const std::vector<std::string> types = {
        "Object", "VM-Snapshot", "DB-Backup", "Log-Archive", "Container-Image"};
    return types;
}
