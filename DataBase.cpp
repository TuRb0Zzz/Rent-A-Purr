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

    sqlite3* DataBase::GetDataBaseLink(){
        return db;
    }

    bool DataBase::CreateTables(){
        Sql_exec("CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY AUTOINCREMENT, username TEXT UNIQUE NOT NULL, nickname TEXT NOT NULL, password TEXT NOT NULL, access_level INTEGER DEFAULT 0 CHECK(access_level IN (0, 1)));");
        Sql_exec("CREATE TABLE IF NOT EXISTS sessions (session_id TEXT PRIMARY KEY, user_id INTEGER UNIQUE NOT NULL, expires_at DATETIME NOT NULL, FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE);");
        Sql_exec("CREATE TABLE IF NOT EXISTS cats (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT NOT NULL, description TEXT, breed TEXT, age TEXT, photo_filename TEXT);");
        Sql_exec("CREATE TABLE IF NOT EXISTS bookings (id INTEGER PRIMARY KEY AUTOINCREMENT, cat_id INTEGER NOT NULL, user_id INTEGER NOT NULL, start_time DATETIME NOT NULL, end_time DATETIME NOT NULL, status INTEGER DEFAULT 0 CHECK(status IN (0, 1)), FOREIGN KEY (cat_id) REFERENCES cats(id) ON DELETE CASCADE, FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE);");        Sql_exec("CREATE INDEX IF NOT EXISTS idx_bookings_cat_time ON bookings(cat_id, start_time, end_time);");
        Sql_exec("CREATE TABLE IF NOT EXISTS tags (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT UNIQUE NOT NULL);");
        Sql_exec("CREATE TABLE IF NOT EXISTS cat_tags (cat_id INTEGER REFERENCES cats(id) ON DELETE CASCADE, tag_id INTEGER REFERENCES tags(id) ON DELETE CASCADE, PRIMARY KEY (cat_id, tag_id));");
        Sql_exec("CREATE TABLE IF NOT EXISTS medical (id INTEGER PRIMARY KEY AUTOINCREMENT, cat_id INTEGER REFERENCES cats(id) ON DELETE CASCADE, icon TEXT, label TEXT, color TEXT, bg TEXT);");
        
        return true;
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

    void DataBase::cleanExpiredSessions() {
        Sql_exec("DELETE FROM sessions WHERE expires_at <= datetime('now')");
    }

    long long DataBase::GetLastInsertId() {
        return sqlite3_last_insert_rowid(db);
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

