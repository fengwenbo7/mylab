#ifndef METADATA
#define METADATA

struct MetaData{
    int version;//the version of meta info
    int data_start;//the start address of data
    int data_length;//the length of data
    int index_start;//the start address of index
    int index_length;//the length of index
};

struct Position{
    int start_;
    int len_;
    bool is_deleted_;
};

#endif