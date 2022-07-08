#ifndef LSM_H
#define LSM_H

template<class K,class V>
class LSM{
private:
    LSM<K,V>(const LSM<K,V> &other) = default;
    LSM<K,V>(LSM<K,V> &other) = default;
    LSM<K,V>(unsigned long eltsPerRun,unsigned int numRuns, double merged_frac,double bf_fp,unsigned int pageSize,unsigned int diskRunPerLevel);
public:
};

#endif