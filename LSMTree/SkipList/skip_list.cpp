#include "skip_list.h"


template<typename K>
SkipList<K>::SkipList(int max_level){
    this->max_level=max_level;
    this->skip_list_level=0;
    //create header node
    K k;
    Json::Value v;
    this->head=new Node<K,V>(k,v,max_level);
}

template<typename K>
SkipList<K>::~SkipList(){
    delete head;
}

//2-1/2:2
//2,2-1/4:3
//2,2,2-1/8:4
template<typename K>
int SkipList<K>::get_random_level(){
    int k=1;
    while(rand()%2){
        k++;
    }
    return k<max_level?k:max_level;
}

template<typename K>
KVNode<K>* SkipList<K>::create_node(K k,Json::Value v,int level){
    return new KVNode(k,v,level);
}

// Insert given key and value in skip list 
// return 1 means element exists  
// return 0 means insert successfully
/* 
                           +------------+
                           |  insert 50 |
                           +------------+
level 4     +-->1+                                                      100
                 |
                 |                      insert +----+
level 3         1+-------->10+---------------> | 50 |          70       100
                                               |    |
                                               |    |
level 2         1          10         30       | 50 |          70       100
                                               |    |
                                               |    |
level 1         1    4     10         30       | 50 |          70       100
                                               |    |
                                               |    |
level 0         1    4   9 10         30   40  | 50 |  60      70       100
                                               +----+
                                        update[0]     current

*/
template<typename K>
int SkipList<K>::insert_element(K k,Json::Value v){
    mutex_.lock();
    //1.get head as current
    Node<K,V>* current=head;
    //2.initial array of update for record the position of isnerted node,and update point to the preview ndoe of the inserted node
    Node<K,V>* update[max_level+1];
    memset(update,0,sizeof(Node<K,V>*)*max_level);
    //3.find the position and set the array
    for(int i=skip_list_level;i>=0;i--){
        while(current->nexts[i]!=NULL&&current->nexts[i]->get_key()<k){
            current=current->nexts[i];
        }
        update[i]=current;
    }
    //4.if exists?return:insert
    if(update[0]->nexts[0]!=NULL&&update[0]->nexts[0]->get_key()==k){
        std::cout<<"key:"<<k<<" exists."<<std::endl;
        mutex_.unlock();
        return 1;
    }else{
        //4.1 get random level
        int random_level=get_random_level();
        //4.2 initial more node if random level is greater than skip list level
        if(random_level>skip_list_level){
            for(int i=skip_list_level+1;i<random_level+1;i++){
                update[i]=head;
            }
            skip_list_level=random_level;
        }
        //4.3 create new node
        Node<K,V>* new_node=create_node(k,v,random_level);
        //4.4 new node should be inserted between update[i] and update[i]->nexts[i]
        for(int i=0;i<random_level;i++){
            new_node->nexts[i]=update[i]->nexts[i];
            update[i]->nexts[i]=new_node;
        }
        std::cout<<"successfully insert key:"<<k<<",value:"<<v<<std::endl;
    }

    mutex_.unlock();
    return 0;
}

template<typename K>
void SkipList<K>::delete_element(K key){
    mutex_.lock();
    Node<K,V>* current=head;
    Node<K,V>* update[max_level];
    memset(update,0,sizeof(Node<K,V>*)*max_level);
    //1.find the position 
    for(int i=skip_list_level;i>=0;i--){
        while(current->nexts[i]&&current->nexts[i]->get_key()<key){
            current=current->nexts[i];
        }
        update[i]=current;
    }
    current=current->nexts[0];
    //2.unlink the link and update the skip list level
    if(current&&current->get_key()==key){
        for(int i=0;i<skip_list_level;i++){
            if(update[i]->nexts[i]!=current)
                break;//level over so delete over
            update[i]->nexts[i]=current->nexts[i];
        }
        while(skip_list_level>0 && head->nexts[skip_list_level]==0){
            skip_list_level--;
        }
        std::cout<<"successfully delete key:"<<key<<std::endl;
    }
    mutex_.unlock();
}

/*
                           +------------+
                           |  select 60 |
                           +------------+
level 4     +-->1+                                                      100
                 |
                 |
level 3         1+-------->10+------------------>50+           70       100
                                                   |
                                                   |
level 2         1          10         30         50|           70       100
                                                   |
                                                   |
level 1         1    4     10         30         50|           70       100
                                                   |
                                                   |
level 0         1    4   9 10         30   40    50+-->60      70       100
*/
template<typename K>
bool SkipList<K>::search_element(K key){
    //1.find preview node
    Node<K,V>* current=head;
    for(int i=skip_list_level;i>=0;i--){
        while(current->nexts[i]&&current->nexts[i]->get_key()<key){
            current=current->nexts[i];
        }
    }
    //2.get next node
    current=current->nexts[0];
    if(current&&current->get_key()==key){
        std::cout<<"Fount the target node,key:"<<key<<",value:"<<current->get_value()<<std::endl;
        return true;
    }
    std::cout<<"Not found the target node,key:"<<key<<std::endl;
    return false;
}

template<typename V>
void SkipList<V>::display_list() {
    for (int i = 0; i <= skip_list_level; i++) {
        Node<K, V> *node = this->head->nexts[i]; 
        std::cout << "Level " << i << ": ";
        while (node != NULL) {
            std::cout << node->get_key() << ":" << node->get_value() << ";";
            node = node->nexts[i];
        }
        std::cout << std::endl;
    }
}