#ifndef CONTACT_INFO_
#define CONTACT_INFO_

struct PersonInfo
{
    char name[128];
    uint age;
    uint gender;
    char phonenumber[20];
    char email[128];
};

struct PersonInfoWithResume
{
    PersonInfo person_info;
    uint resume_length;
    char *resume;
};

#endif
