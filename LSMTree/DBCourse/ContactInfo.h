#ifndef CONTACT_INFO_
#define CONTACT_INFO_
#include <string>
#include <fstream>
#include <vector>
#include <stdio.h>
using namespace std;
class ContantInfo
{
public:
    char name[128];
    uint age;
    uint gender;
    uint phonenumber[20];
    char email[128];
    char resume[128];

public:
    void WriteToDat(FILE *fp, int f_off)
    {
        int ok = fseek(fp, f_off, SEEK_SET);
        if (ok)
        {
            fwrite(this, sizeof(ContantInfo), 1, fp);
        }
    }

    ContantInfo *ReadFromDat(FILE *fp, int data_size)
    {
        ContantInfo info[data_size];
        int ok = fseek(fp, 0, SEEK_SET);
        if (ok)
        {
            fread(info, sizeof(ContantInfo), data_size, fp);
        }
        return info;
    }

    ContantInfo &SearchFromDat(FILE *fp, std::string name)
    {
    }
};

#endif
