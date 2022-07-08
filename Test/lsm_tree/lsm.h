#ifndef LSM_H
#define LSM_H

template<class K,class V>
class LSM{
public:
    ~LSM<K,V>();
    LSM<K,V>(const LSM<K,V> &other) = default;
    LSM<K,V>(LSM<K,V> &other) = default;
    LSM<K,V>(unsigned long eltsPerRun,unsigned int numRuns, double merged_frac,double bf_fp,unsigned int pageSize,unsigned int diskRunPerLevel);
    void insert_key(K &key,V &value);
    void delete_key(K &key);
    bool lookup(K &key,V &value);
private:
    void mergeRunsToLevel(int level);
    //void merge_runs(vector<Run<K,V>*> runs_to_merge,vector<>)
    void do_merge();
    unsigned long num_buffer();
    unsigned long size();
};

#endif