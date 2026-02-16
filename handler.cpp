#include "Handler.h"
#include <json/json.h>     
#include <iostream>         
#include <string>
#include <sqlite3.h>
using namespace drogon;     
using namespace std;

extern DataBase db;

bool Handler::verifyPassword(const string& Password, const string& hashedPassword) {
            return hashPassword(Password) == hashedPassword;
}

string Handler::hashPassword(const string& password) {
            unsigned char hash[SHA256_DIGEST_LENGTH];
            SHA256((unsigned char*)password.c_str(), password.length(), hash);
            stringstream ss;
            for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
                ss << hex << setw(2) << setfill('0') << (int)hash[i];
            }
            return ss.str();
}

int Handler::checkAuth(string session_id) {
    if (session_id.empty()) {
        return -1;
    }
    int userId = getUserIdFromSession(session_id);
    return userId;
}


string Handler::generateSessionId() {
        static int counter = 0;
        string data = to_string(time(nullptr)) + to_string(rand()) + to_string(counter++);
        
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256((unsigned char*)data.c_str(), data.length(), hash);
        
        stringstream ss;
        for(int i = 0; i < 16; i++) {
            ss << hex << setw(2) << setfill('0') << (int)hash[i];
        }
        return ss.str();
}

string Handler::createSession(long long id){

    string sessionId = generateSessionId();
    char* sql = sqlite3_mprintf("INSERT INTO sessions (session_id, user_id, expires_at) VALUES (%Q, %d, datetime('now', '+1 day'))",sessionId.c_str(), id);
    if (db.Sql_exec(sql)) {
            sqlite3_free(sql);
            return sessionId;
    }
    sqlite3_free(sql);
    return "";
}


int Handler::getUserIdFromSession(const string& sessionId) {
        db.cleanExpiredSessions();
        
        char* sql = sqlite3_mprintf("SELECT user_id FROM sessions WHERE session_id = %Q",sessionId.c_str());
        
        vector<vector<string>> result = db.Sql_request_vector(sql);
        sqlite3_free(sql);
        
        if (!result.empty()) {
            return stoi(result[0][0]);
        }
        return -1;
}

void Handler::RegisterUser(const HttpRequestPtr& request,function<void(const HttpResponsePtr&)>&& callback){
    auto json = request->getJsonObject();
    if(!json){
        Json::Value bad_answer;
        bad_answer["status"]="bad";
        bad_answer["message"]="Bad Json";
        auto response = HttpResponse::newHttpJsonResponse(bad_answer);
        response->setStatusCode(k400BadRequest);
        callback(response);
        return;
    }
    string username = (*json)["username"].asString();
    string password = (*json)["password"].asString();
    password = hashPassword(password);
    char *sql = sqlite3_mprintf("SELECT id FROM users WHERE username=%Q",username.c_str());
    vector<vector<string>> check = db.Sql_request_vector(sql);
    sqlite3_free(sql);
    if(!check.empty()){
        Json::Value bad_answer;
        bad_answer["status"]="bad";
        bad_answer["message"]="User already exists";
        auto response = HttpResponse::newHttpJsonResponse(bad_answer);
        response->setStatusCode(k409Conflict);
        callback(response);
        return;
    }
    sql = sqlite3_mprintf("INSERT INTO users (username, password) VALUES (%Q, %Q)",username.c_str(),password.c_str());
    if(!(db.Sql_exec(sql))){
        sqlite3_free(sql);
        Json::Value bad_answer;
        bad_answer["status"]="bad";
        bad_answer["message"]="Cant insert Data";
        auto response = HttpResponse::newHttpJsonResponse(bad_answer);
        response->setStatusCode(k500InternalServerError);
        callback(response);
        return;
    }
    sqlite3_free(sql);

    long long id = db.GetLastInsertId();
    string session_id = createSession(id);

    Json::Value resp;
    resp["status"]="ok";
    resp["message"] = "User created";
    auto response = HttpResponse::newHttpJsonResponse(resp);
    if(session_id!=""){
        response->addHeader("Set-Cookie","session_id=" + session_id + "; Max-Age=86400; Path=/; HttpOnly");
    } 
    response->setStatusCode(k201Created);
    callback(response);
}