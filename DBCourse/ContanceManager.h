#ifndef CONTACT_MANAGER
#define CONTACT_MANAGER

#include <stdio.h>
#include <string>
#include "ContactInfo.h"
#include <fstream>
#include <list>
#include <algorithm>
using namespace std;

#define RESUME_FILE_SIZE

class ContactManager
{
public:
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
        fstream resume_file_s(resume_file_path_, ios::in | ios::out | ios::binary);
        if (!resume_file_s)
        {
            std::cout << "fopen error:" << resume_file_path_ << std::endl;
            return;
        }

        memcpy(info.person_info.resume_file_name, resume_file_path_.c_str(), sizeof(resume_file_path_));
        // info.person_info.resume_file_name = (char *)resume_file_path_.c_str();
        info.person_info.resume_length = sizeof(info.resume);
        info.person_info.resume_start = resume_file_off_;

        dat_file_s.seekp(file_off_, ios::beg);
        dat_file_s.write((char *)&(info.person_info), sizeof(PersonInfo));
        file_off_ += sizeof(PersonInfo);
        data_size_++;
        dat_file_s.close();
        std::cout << (char *)info.person_info.name << "," << sizeof(info.person_info.name) << std::endl;

        resume_file_s.write((char *)info.resume, sizeof(info.resume));
        resume_file_off_ += sizeof(info.resume);
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
        for (size_t i = 0; i < data_size_; i++)
        {
            string tmp_name(p_info[i].name);
            string tmp_phonenum(p_info[i].phonenumber);
            string tmp_email(p_info[i].email);
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
            if (show_resume)
            {
                std::cout << "search resume,file:" << p_info[i].resume_file_name << ",start:" << p_info[i].resume_start << ",length:" << p_info[i].resume_length << std::endl;
                fstream resume_file_s(p_info[i].resume_file_name, ios::in | ios::out | ios::binary);
                if (resume_file_s)
                {
                    resume_file_s.seekg(p_info[i].resume_start, ios::beg);
                    resume_file_s.read((char *)p_full[i].resume, p_info[i].resume_length);
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
    const std::string dat_file_path_;
    const std::string resume_file_path_;
    int file_off_;
    int resume_file_off_;
    int data_size_;
};

#endif