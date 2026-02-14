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

    Json::Value resp;
    resp["status"]="ok";
    resp["message"] = "User created";
    auto response = HttpResponse::newHttpJsonResponse(resp);
    response->setStatusCode(k201Created);
    callback(response);
}