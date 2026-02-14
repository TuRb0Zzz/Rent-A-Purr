#include "DataBase.h"
#include <future>
#include <iostream>


    DataBase::DataBase(): db(nullptr){
        if(OpenDataBase()){
            CreateTables();
        }
    }

    DataBase::~DataBase(){
        CloseDataBase();
    }

    bool DataBase::CreateTables(){
        Sql_exec("CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY AUTOINCREMENT, username TEXT UNIQUE NOT NULL, password TEXT NOT NULL, access_level INTEGER DEFAULT 0 CHECK(access_level IN (0, 1)));");
        Sql_exec("CREATE TABLE IF NOT EXISTS sessions (session_id TEXT PRIMARY KEY, user_id INTEGER NOT NULL, expires_at DATETIME NOT NULL, FOREIGN KEY (user_id) REFERENCES users(id));");
        return true;//bruh
    }


    bool DataBase::OpenDataBase(){
        int answer_from_db = sqlite3_open("../DataBase/CatsDataBase.db",&db);
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
            return true;
        }
        return false;
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

    void DataBase::Sql_request_callback(const string& sql_string,function<void(vector<string>)>callback){
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
        sqlite3_finalize(stmt);
    }

    vector<vector<string>> DataBase::Sql_request_vector(const string& sql_string){
        vector<vector<string>> result;
        sqlite3_stmt* stmt;    
        sqlite3_prepare_v2(db, sql_string.c_str(), -1, &stmt, nullptr);      
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::vector<std::string> row;         
            int cols = sqlite3_column_count(stmt);         
            for (int i = 0; i < cols; i++) {
                const char* val = (const char*)sqlite3_column_text(stmt, i);
                row.push_back(val ? val : "NULL");
            }
            result.push_back(row);         
        }
        sqlite3_finalize(stmt);
        return result;
    }

