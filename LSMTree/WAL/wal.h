#ifndef WAL_H
#define WAL_H

#include <string>
#include <mutex>

class WalClass{
public:
    void Write();
    void Read();
};

struct WalNode{
    int fd_;
    std::string path_;
    std::mutex mutex_;
};

#endif