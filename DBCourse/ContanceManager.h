#ifndef CONTACT_MANAGER
#define CONTACT_MANAGER

#include <stdio.h>
#include <string>
#include "ContactInfo.h"
#include <fstream>
#include <list>
#include <algorithm>
using namespace std;
class ContactManager
{
public:
    ContactManager(std::string dat_file) : dat_file_path_(dat_file)
    {
        file_off_ = 0;
        data_size_ = 0;
        fstream fs(dat_file, ios::out);
        if (fs)
        {
            fs.close();
        }
    }

    ~ContactManager()
    {
    }

    void SaveData(PersonInfo info)
    {
        // save origin data
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

        // save bplus tree
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

    list<PersonInfo> SearchFromDat(const char *name, const char *phonenum, const char *email)
    {
        list<PersonInfo> ret;
        fstream dat_file_s(dat_file_path_, ios::in | ios::out | ios::binary);
        if (!dat_file_s)
        {
            std::cout << "fopen error:" << dat_file_path_ << std::endl;
            return ret;
        }
        dat_file_s.seekg(0, ios::beg);
        PersonInfo p1[data_size_];
        dat_file_s.read((char *)p1, sizeof(PersonInfo) * data_size_);
        string input_name(name);
        string input_phonenum(phonenum);
        string input_email(email);
        transform(input_name.begin(), input_name.end(), input_name.begin(), ::tolower);
        transform(input_phonenum.begin(), input_phonenum.end(), input_phonenum.begin(), ::tolower);
        transform(input_email.begin(), input_email.end(), input_email.begin(), ::tolower);
        for (size_t i = 0; i < data_size_; i++)
        {
            string tmp_name(p1[i].name);
            string tmp_phonenum(p1[i].phonenumber);
            string tmp_email(p1[i].email);
            transform(tmp_name.begin(), tmp_name.end(), tmp_name.begin(), ::tolower);
            transform(tmp_phonenum.begin(), tmp_phonenum.end(), tmp_phonenum.begin(), ::tolower);
            transform(tmp_email.begin(), tmp_email.end(), tmp_email.begin(), ::tolower);
            if (name != nullptr && strcmp(tmp_name.c_str(), input_name.c_str()) != 0)
            {
                continue;
            }
            if (phonenum != nullptr && strcmp(tmp_phonenum.c_str(), input_phonenum.c_str()) != 0)
            {
                continue;
            }
            if (email != nullptr && strcmp(tmp_email.c_str(), input_email.c_str()) != 0)
            {
                continue;
            }
            ret.push_back(p1[i]);
        }

        dat_file_s.close();
        std::cout << "fclose ok." << std::endl;
        return ret;
    }

private:
    const std::string dat_file_path_;
    int file_off_;
    int data_size_;
};

#endif