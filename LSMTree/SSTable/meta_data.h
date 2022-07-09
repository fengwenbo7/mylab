#ifndef METADATA
#define METADATA

struct MetaData{
    int data_start;//the start address of data
    int data_length;//the length of data
    int index_start;//the start address of index
    int index_length;//the length of index
};

class KVNode{
public:
    KVNode(int key,int value):key_(key),value_(value),is_deleted_(false){}
    int get_key(){return key_;}
    int get_value(){return value_;}
    void set_value(int value){value_=value;}
private:
    int key_;
    int value_;
    bool is_deleted_;
};

#endif