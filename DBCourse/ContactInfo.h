#ifndef CONTACT_INFO_
#define CONTACT_INFO_

struct PersonInfo
{
    char name[128];
    uint age;
    uint gender;
    char phonenumber[20];
    char email[128];
    char resume_file_name[128];
    uint resume_start;
    uint resume_length;
};

struct PersonInfoWithResume
{
    PersonInfo person_info;
    char *resume;
};

#endif
