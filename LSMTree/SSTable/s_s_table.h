#ifndef SSTABLE_H
#define SSTABLE_H

#include "meta_data.h"
#include <vector>
#include <string>
#include <map>
#include <mutex>
#include <vector>

struct SSTable{
    int fd_;
    std::string file_path_;
    MetaData meta_info_;
    std::map<std::string,Position> sparse_index_;
    std::vector<std::string> sorted_index_;
    std::mutex mutex_;
};

struct SSTableTree{
    std::vector<TableNode*> levels_;
    std::mutex mutex_;
};

struct TableNode{
    int index_;
    SSTable* table_;
    TableNode* next_;
};


#endif