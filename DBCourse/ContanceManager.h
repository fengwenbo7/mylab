#ifndef CONTACT_MANAGER
#define CONTACT_MANAGER

#include <stdio.h>
#include <string>
#include "ContactInfo.h"
#include <fstream>
using namespace std;
class ContactManager
{
public:
    ContactManager(std::string dat_file) : dat_file_path_(dat_file)
    {
        file_off_ = 0;
        data_size_ = 0;
    }

    ~ContactManager()
    {
    }

    void SaveData(PersonInfo info)
    {
        fstream dat_file_s(dat_file_path_, ios::in | ios::out | ios::binary);
        if (!dat_file_s)
        {
            std::cout << "fopen error:" << dat_file_path_ << std::endl;
            return;
        }
        dat_file_s.seekp(file_off_, ios::beg);
        dat_file_s.write((char *)&info, sizeof(PersonInfo));
        file_off_ = file_off_ + sizeof(PersonInfo);
        data_size_++;
        std::cout << "save data to dat success,file_off:" << file_off_ << ",data_size:" << data_size_ << std::endl;
        dat_file_s.close();
    }

    size_t ReadFromDat(PersonInfo *data)
    {
        fstream dat_file_s(dat_file_path_, ios::in | ios::out | ios::binary);
        if (!dat_file_s)
        {
            std::cout << "fopen error:" << dat_file_path_ << std::endl;
            return 0;
        }
        dat_file_s.seekg(0, ios::beg);
        PersonInfo p1[data_size_];
        dat_file_s.read((char *)p1, sizeof(PersonInfo) * data_size_);
        for (size_t i = 0; i < data_size_; i++)
        {
            std::cout << i << " name:" << p1[i].name << std::endl;
        }

        dat_file_s.close();
        std::cout << "fclose ok." << std::endl;
        return data_size_;
    }

private:
    const std::string dat_file_path_;
    int file_off_;
    int data_size_;
};

#endif