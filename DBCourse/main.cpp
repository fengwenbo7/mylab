#include <iostream>
#include "ContanceManager.h"
using namespace std;
int main()
{
    ContactManager cm("ContactDat.dat");
    while (true)
    {
        std::cout << "input function:" << std::endl;
        std::cout << "1.new" << std::endl;
        std::cout << "2.search" << std::endl;
        std::cout << "3.show all" << std::endl;
        int func_type;
        std::cin >> func_type;
        switch (func_type)
        {
        case 1:
        {
            PersonInfo info;
            std::cout << "name:";
            std::cin >> info.name;
            std::cout << "age:";
            std::cin >> info.age;
            std::cout << "gender:";
            std::cin >> info.gender;
            std::cout << "phonenumber:";
            std::cin >> info.phonenumber;
            std::cout << "email:";
            std::cin >> info.email;
            std::cout << "resume:";
            std::cin >> info.resume;
            cm.SaveData(info);
        }
        break;
        case 2:
            break;
        case 3:
        {
            PersonInfo *data;
            size_t data_len_ = cm.ReadFromDat(data);
            int index = 0;
            std::cout << "read from dat:" << data_len_ << std::endl;
            /*if (data_len_ > 0)
            {
                for (size_t i = 0; i < data_len_; i++)
                {
                    std::cout << "name:" << data->name << " "
                              << "age:" << data->age << " "
                              << "phonenumber" << data->phonenumber << " "
                              << "email" << data->email << std::endl;
                    data = data + sizeof(pi);
                }
            }*/
        }
        break;
        default:
            break;
        }
    }
}