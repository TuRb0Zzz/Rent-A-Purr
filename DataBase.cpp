#include "Database.h"
#include <iostream>


    DataBase::DataBase(): db(nullptr){
        OpenDataBase();
    }

    DataBase::~DataBase(){
        CloseDataBase();
    }

    bool DataBase::OpenDataBase(){
        int answer_from_db = sqlite3_open("CatsDataBase",&db);
        if(answer_from_db!=SQLITE_OK){
            cout<<"Cannot oped DB: "<<sqlite3_errmsg(db)<<endl;
            return false;
        }
        return true;
    }

    bool DataBase::CloseDataBase(){
        if(db){
            sqlite3_close(db);
            db=nullptr;
        }
    }

    bool DataBase::Sql_exec(const string& sql_string){
        char* error_message = nullptr;
        int answer_from_db = sqlite3_exec(db,sql_string.c_str(),nullptr,nullptr,&error_message);
        if(answer_from_db!=SQLITE_OK){
            cout<<"error: "<<error_message<<endl;
            sqlite3_free(error_message);
            error_message=nullptr;
            return false;
        }
        return true;
    }

    void DataBase::Sql_request(const string& sql_string,function<void(vector<string>)>callback){
        sqlite3_stmt* stmt;    
        sqlite3_prepare_v2(db, sql_string.c_str(), -1, &stmt, nullptr);      
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::vector<std::string> row;         
            int cols = sqlite3_column_count(stmt);         
            for (int i = 0; i < cols; i++) {
                const char* val = (const char*)sqlite3_column_text(stmt, i);
                row.push_back(val ? val : "NULL");
            }         
            callback(row);
        }
    }