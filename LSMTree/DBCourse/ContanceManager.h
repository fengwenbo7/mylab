#ifndef CONTACT_MANAGER
#define CONTACT_MANAGER

#include <stdio.h>
#include <string>
#include "ContactInfo.h"
class ContactManager
{
public:
    ContactManager(std::string dat_file) : dat_file_path_(dat_file)
    {
        dat_file_p_ = fopen(dat_file_path_.c_str(), "wb");
    }

    ~ContactManager()
    {
        fclose(dat_file_p_);
    }

private:
    std::string dat_file_path_;
    std::FILE *dat_file_p_;
};

#endif