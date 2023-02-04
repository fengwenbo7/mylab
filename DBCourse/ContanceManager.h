#ifndef CONTACT_MANAGER
#define CONTACT_MANAGER

#include <stdio.h>
#include <string>
#include "ContactInfo.h"
#include <fstream>
#include <list>
#include <algorithm>
#include <iostream>
using namespace std;

#define RESUME_FILE_SIZE

class ContactManager
{
private:
    static ContactManager *instance_;

public:
    static ContactManager *GetInstance(std::string dat_file, std::string resume_file_path)
    {
        if (nullptr == instance_)
        {
            instance_ = new ContactManager(dat_file, resume_file_path);
        }
        return instance_;
    }

    int GetDataSize()
    {
        return data_size_;
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
        fstream resume_file_s(resume_file_path_, ios::in | ios::out | ios::binary);
        if (!resume_file_s)
        {
            std::cout << "fopen error:" << resume_file_path_ << std::endl;
            return;
        }

        memcpy(info.person_info.resume_file_name, resume_file_path_.c_str(), sizeof(resume_file_path_));
        info.person_info.resume_length = strlen(info.resume);
        info.person_info.resume_start = resume_file_off_;

        dat_file_s.seekp(file_off_, ios::beg);
        dat_file_s.write((char *)&(info.person_info), sizeof(PersonInfo));
        file_off_ += sizeof(PersonInfo);
        data_size_++;
        dat_file_s.close();
        std::cout << (char *)info.person_info.name << "," << sizeof(info.person_info.name) << std::endl;

        resume_file_s.seekp(resume_file_off_, ios::beg);
        resume_file_s.write((char *)info.resume, strlen(info.resume));
        resume_file_off_ += strlen(info.resume);
        resume_file_s.close();
        std::cout << "save data to dat success,file_off:" << file_off_ << ",data_size:" << data_size_ << std::endl;

        // save bplus tree
    }

    list<PersonInfoWithResume> SearchFromDat(const char *name, const char *phonenum, const char *email, bool show_resume)
    {
        list<PersonInfoWithResume> ret;
        fstream dat_file_s(dat_file_path_, ios::in | ios::out | ios::binary);
        if (!dat_file_s)
        {
            std::cout << "fopen error:" << dat_file_path_ << std::endl;
            return ret;
        }
        dat_file_s.seekg(0, ios::beg);
        PersonInfoWithResume p_full[data_size_];
        PersonInfo p_info[data_size_];
        dat_file_s.read((char *)p_info, sizeof(PersonInfo) * data_size_);
        string input_name(name);
        string input_phonenum(phonenum);
        string input_email(email);
        transform(input_name.begin(), input_name.end(), input_name.begin(), ::tolower);
        transform(input_phonenum.begin(), input_phonenum.end(), input_phonenum.begin(), ::tolower);
        transform(input_email.begin(), input_email.end(), input_email.begin(), ::tolower);
        // std::cout << "search name:" << input_name << ",phonenumber:" << input_phonenum << ",email:" << input_email << ",size:" << data_size_ << std::endl;
        for (size_t i = 0; i < data_size_; i++)
        {
            string tmp_name(p_info[i].name);
            string tmp_phonenum(p_info[i].phonenumber);
            string tmp_email(p_info[i].email);
            // std::cout << "search each data-----name:" << tmp_name << ",phonenumber:" << tmp_phonenum << ",email:" << tmp_email << std::endl;
            transform(tmp_name.begin(), tmp_name.end(), tmp_name.begin(), ::tolower);
            transform(tmp_phonenum.begin(), tmp_phonenum.end(), tmp_phonenum.begin(), ::tolower);
            transform(tmp_email.begin(), tmp_email.end(), tmp_email.begin(), ::tolower);
            if (name != nullptr && strcmp(tmp_name.c_str(), input_name.c_str()) != 0)
            {
                std::cout << "continue name" << std::endl;
                continue;
            }
            if (phonenum != nullptr && strcmp(tmp_phonenum.c_str(), input_phonenum.c_str()) != 0)
            {
                std::cout << "continue phonenumber" << std::endl;
                continue;
            }
            if (email != nullptr && strcmp(tmp_email.c_str(), input_email.c_str()) != 0)
            {
                std::cout << "continue email."
                          << "email size:" << tmp_email.size() << "," << input_email.size() << std::endl;
                continue;
            }
            if (show_resume)
            {
                std::cout << "search resume,file:" << p_info[i].resume_file_name << ",start:" << p_info[i].resume_start << ",length:" << p_info[i].resume_length << std::endl;
                fstream resume_file_s(p_info[i].resume_file_name, ios::in | ios::out | ios::binary);
                if (resume_file_s)
                {
                    resume_file_s.seekg(p_info[i].resume_start, ios::beg);
                    std::cout << "start read." << std::endl;
                    resume_file_s.read((char *)p_full[i].resume, p_info[i].resume_length);
                    std::cout << "end read." << p_full[i].resume << std::endl;
                    resume_file_s.close();
                }
                else
                {
                    std::cout << "fopen error:" << p_info[i].resume_file_name << std::endl;
                }
            }
            p_full[i].person_info = p_info[i];
            ret.push_back(p_full[i]);
        }

        dat_file_s.close();
        std::cout << "fclose ok." << std::endl;
        return ret;
    }

private:
    ContactManager(std::string dat_file, std::string resume_file_path) : dat_file_path_(dat_file), resume_file_path_(resume_file_path)
    {
        file_off_ = 0;
        data_size_ = 0;
        resume_file_off_ = 0;
        fstream fs(dat_file, ios::out);
        if (fs)
        {
            fs.close();
        }
        fstream resume_fs(resume_file_path, ios::out);
        if (resume_fs)
        {
            resume_fs.close();
        }
    }

private:
    const std::string dat_file_path_;
    const std::string resume_file_path_;
    int file_off_;
    int resume_file_off_;
    int data_size_;
};

ContactManager *ContactManager::instance_ = NULL;
#endif