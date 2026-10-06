// main.cpp
// BLAST 01 — B+ Tree indexed cloud-record search
// Author: Pranav Anil Singh
// Course: CSC-6021 Data Structures and Algorithms II
//
// Usage:  ./cloud_index            (default order 4)
//         ./cloud_index <order>    (order 3-512)

#include "CloudIndexApp.h"

#include <cstdlib>
#include <iostream>

int main(int argc, char* argv[]) {
    int order = 4;
    if (argc > 1) {
        char* end = nullptr;
        long value = std::strtol(argv[1], &end, 10);
        if (*end != '\0' || value < BPlusTree::MIN_ORDER || value > BPlusTree::MAX_ORDER) {
            std::cerr << "Order must be a whole number from " << BPlusTree::MIN_ORDER << " to "
                      << BPlusTree::MAX_ORDER << ".\n";
            return EXIT_FAILURE;
        }
        order = static_cast<int>(value);
    }

    CloudIndexApp app(order);
    app.run();
    return EXIT_SUCCESS;
}
