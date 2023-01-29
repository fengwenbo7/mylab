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

    void SaveData(PersonInfoWithResume info)
    {
        // save origin data
        fstream dat_file_s(dat_file_path_, ios::in | ios::out | ios::binary);
        if (!dat_file_s)
        {
            std::cout << "fopen error:" << dat_file_path_ << std::endl;
            return;
        }
        dat_file_s.seekp(file_off_, ios::beg);
        std::cout << (char *)info.person_info.name << "," << sizeof(info.person_info.name) << std::endl;
        dat_file_s.write((char *)info.person_info.name, sizeof(info.person_info.name));
        std::cout << "1" << std::endl;
        dat_file_s.write(std::to_string(info.person_info.age).c_str(), sizeof(info.person_info.age));
        std::cout << "2" << std::endl;
        dat_file_s.write(std::to_string(info.person_info.gender).c_str(), sizeof(info.person_info.gender));
        dat_file_s.write((char *)info.person_info.phonenumber, sizeof(info.person_info.phonenumber));
        dat_file_s.write((char *)info.person_info.email, sizeof(info.person_info.email));
        dat_file_s.write(std::to_string(info.resume_length).c_str(), sizeof(info.resume_length));
        dat_file_s.write((char *)info.resume, sizeof(info.resume));
        file_off_ = file_off_ + sizeof(info.person_info) + sizeof(info.resume_length) + info.resume_length;
        data_size_++;
        std::cout << "save data to dat success,file_off:" << file_off_ << ",data_size:" << data_size_ << std::endl;
        dat_file_s.close();

        // save bplus tree
    }

    list<PersonInfoWithResume> SearchFromDat(const char *name, const char *phonenum, const char *email)
    {
        list<PersonInfoWithResume> ret;
        fstream dat_file_s(dat_file_path_, ios::in | ios::out | ios::binary);
        if (!dat_file_s)
        {
            std::cout << "fopen error:" << dat_file_path_ << std::endl;
            return ret;
        }
        dat_file_s.seekg(0, ios::beg);
        PersonInfoWithResume p1[data_size_];
        dat_file_s.read((char *)p1, sizeof(PersonInfoWithResume) * data_size_);
        string input_name(name);
        string input_phonenum(phonenum);
        string input_email(email);
        transform(input_name.begin(), input_name.end(), input_name.begin(), ::tolower);
        transform(input_phonenum.begin(), input_phonenum.end(), input_phonenum.begin(), ::tolower);
        transform(input_email.begin(), input_email.end(), input_email.begin(), ::tolower);
        for (size_t i = 0; i < data_size_; i++)
        {
            string tmp_name(p1[i].person_info.name);
            string tmp_phonenum(p1[i].person_info.phonenumber);
            string tmp_email(p1[i].person_info.email);
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