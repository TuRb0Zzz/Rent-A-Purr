#pragma once
#include <sqlite3.h>
#include <string>
#include <vector>
#include <iostream>
#include <functional>
using namespace std;

class DataBase{
private:
    sqlite3* db;
public:
    DataBase();

    ~DataBase();

    bool OpenDataBase();

    bool CloseDataBase();

    bool Sql_exec(const string& sql_string);

    void Sql_request(const string& sql_string,std::function<void(vector<string>)>callback);
};




