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

    bool CreateTables();

    bool OpenDataBase();

    bool CloseDataBase();

    long long GetLastInsertId();

    void cleanExpiredSessions();

    bool Sql_exec(const string& sql_string);

    void Sql_request_callback(const string& sql_string,std::function<void(vector<string>)>callback);

    vector<vector<string>> Sql_request_vector(const string& sql_string);
};




