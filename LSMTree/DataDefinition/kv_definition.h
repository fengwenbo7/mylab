#ifndef KV_DEFINITION_H
#define KV_DEFINITION_H

#include <jsoncpp/json/json.h>

template<typename K>
class KVNode{
public:
    KVNode();

    KVNode(K key,Json::Value value,int level){
        key_=key;
        value_ = const_cast<char*>(fast_writer_.write(value).c_str());
        nexts_=new KVNode*[level];
    }

    K get_key(){return key_;}

    char* get_value(){return value_;}

    void set_value(Json::Value value){
        value_ = const_cast<char*>(fast_writer_.write(value).c_str());
    }
private:
    K key_;
    char* value_;
    bool is_deleted_;
    KVNode **nexts_;
    Json::FastWriter fast_writer_;
};

#endif