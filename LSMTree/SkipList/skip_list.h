#include <iostream> 
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <mutex>
#include <fstream>
#include "DataDefinition/kv_definition.h"

std::mutex mutex_;

template<typename K>
class SkipList{
public:
    SkipList(int);
    ~SkipList();
    int get_random_level();
    KVNode<K>* create_node(K,Json::Value,int);
    int insert_element(K,Json::Value);
    void delete_element(K);
    bool search_element(K);
    void display_list();
private:
    int max_level;
    int skip_list_level;
    KVNode<K>* head;
};