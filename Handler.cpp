#include "Handler.h"
#include <json/json.h>     
#include <iostream>         
#include <string>
#include <sqlite3.h>
using namespace drogon;     
using namespace std;

extern DataBase db;

string Handler::hashPassword(const string& password) {
            unsigned char hash[SHA256_DIGEST_LENGTH];
            SHA256((unsigned char*)password.c_str(), password.length(), hash);
            stringstream ss;
            for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
                ss << hex << setw(2) << setfill('0') << (int)hash[i];
            }
            return ss.str();
}

bool Handler::verifyPassword(const string& plainPassword, const string& hashedPassword) {
            return hashPassword(plainPassword) == hashedPassword;
}

int Handler::checkAuth(string session_id) {
    if (session_id.empty()) {
        return -1;
    }
    int userId = getUserIdFromSession(session_id);
    return userId;
}

bool Handler::CheckPermissions(int user_id){
    char* sql = sqlite3_mprintf("SELECT access_level FROM users WHERE id = %d",user_id);
    vector<vector<string>> result = db.Sql_request_vector(sql);
    sqlite3_free(sql);
    if(result[0][0]=="0"){
        return false;
    }else{
        return true;
    }
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

    char* sql = sqlite3_mprintf("SELECT user_id FROM sessions WHERE user_id = %d",id);
    if(!db.Sql_request_vector(sql).empty()){
        sqlite3_free(sql);
        return "";
    }
    sqlite3_free(sql);
    sql = sqlite3_mprintf("INSERT INTO sessions (session_id, user_id, expires_at) VALUES (%Q, %d, datetime('now', '+1 day'))",sessionId.c_str(), id);
    if (db.Sql_exec(sql)) {
            sqlite3_free(sql);
            return sessionId;
    }
    sqlite3_free(sql);
    return "";
}

string Handler::GenerateFileName(string extension){
    return "Cat_" + to_string(time(nullptr)) + "_" + 
           to_string(rand()) + extension;
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
    cout<<"POST /register"<<endl;
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
    string nickname = (*json)["nickname"].asString();
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
    sql = sqlite3_mprintf("INSERT INTO users (username, password, nickname) VALUES (%Q, %Q, %Q)",username.c_str(),password.c_str(),nickname.c_str());
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
    resp["nickname"] = nickname;

    auto response = HttpResponse::newHttpJsonResponse(resp);
    if(session_id!=""){
        response->addHeader("Set-Cookie","session_id=" + session_id + "; Max-Age=86400; Path=/; HttpOnly");
        response->addHeader("Access-Control-Allow-Credentials", "true");
    } 
    response->setStatusCode(k201Created);
    response->addHeader("Access-Control-Allow-Origin", "http://localhost:3000");
    callback(response);
}

void Handler::AutoriseUser(const HttpRequestPtr& request,function<void(const HttpResponsePtr&)>&& callback){
    cout<<"GET /login"<<endl;
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

    char* sql = sqlite3_mprintf("SELECT id, password, nickname FROM users WHERE username=%Q",username.c_str());
    vector<vector<string>> check = db.Sql_request_vector(sql);
    sqlite3_free(sql);
    if(check.empty()){
        Json::Value bad_answer;
        bad_answer["status"]="bad";
        bad_answer["message"]="Wrong username or password";
        auto response = HttpResponse::newHttpJsonResponse(bad_answer);
        response->setStatusCode(k401Unauthorized);
        callback(response);
        return;
    }
    if(!verifyPassword(password,check[0][1])){
        Json::Value bad_answer;
        bad_answer["status"]="bad";
        bad_answer["message"]="Wrong username or password";
        auto response = HttpResponse::newHttpJsonResponse(bad_answer);
        response->setStatusCode(k401Unauthorized);
        callback(response);
        return;
    }

    Json::Value resp;
    resp["status"]="ok";
    resp["message"] = "User authorized";
    resp["nickname"]=check[0][2];
    auto response = HttpResponse::newHttpJsonResponse(resp);

    string session_id = createSession(stoi(check[0][0]));

    if(session_id!=""){
        response->addHeader("Set-Cookie","session_id=" + session_id + "; Max-Age=86400; Path=/; HttpOnly");
        response->addHeader("Access-Control-Allow-Credentials", "true");        
    }
    response->setStatusCode(k200OK);
    response->addHeader("Access-Control-Allow-Origin", "http://localhost:3000");
    callback(response);
}

void Handler::GetCats(const HttpRequestPtr& request,function<void(const HttpResponsePtr&)>&& callback){
    cerr<<"GET /cats"<<endl;
    Json::Value resp;
    Json::Value cats(Json::arrayValue);
    char* sql = sqlite3_mprintf("SELECT * FROM cats");
    db.Sql_request_callback(sql,[&cats](vector<string> output){
        Json::Value buffer_cat;
        buffer_cat["name"]=output[1];
        buffer_cat["description"]=output[2];
        buffer_cat["filename"]=output[3];
        cats.append(buffer_cat);
    });
    sqlite3_free(sql);
    resp["status"]="ok";
    resp["cats"]=cats;
    auto response = HttpResponse::newHttpJsonResponse(resp);
    response->setStatusCode(k200OK);
    response->addHeader("Access-Control-Allow-Origin", "http://localhost:3000");
    callback(response);
}

void Handler::handleOptions(const HttpRequestPtr& request, function<void(const HttpResponsePtr&)>&& callback) {
    auto resp = HttpResponse::newHttpResponse();
    resp->setStatusCode(k200OK);
    resp->addHeader("Access-Control-Allow-Origin", "http://localhost:3000");
    resp->addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    resp->addHeader("Access-Control-Allow-Headers","Content-Type, X-Tunnel-Skip-AntiPhishing-Page, Accept, Authorization, X-Requested-With");
    resp->addHeader("Access-Control-Allow-Credentials", "true");
    resp->addHeader("Access-Control-Max-Age", "3600");
    callback(resp);
}

void Handler::uploadCatPhoto(const HttpRequestPtr& request, function<void(const HttpResponsePtr&)>&& callback) {
    
    string sessionId = request->getCookie("session_id");
    int user_id = checkAuth(sessionId);
    if (user_id == -1) {
        Json::Value bad_answer;
        bad_answer["status"]="bad";
        bad_answer["message"]="Unauthorized";
        auto response = HttpResponse::newHttpJsonResponse(bad_answer);
        response->setStatusCode(k401Unauthorized);
        callback(response);
        return;
    }
    if(!CheckPermissions(user_id)){
        Json::Value bad_answer;
        bad_answer["status"]="bad";
        bad_answer["message"]="Access Denied";
        auto response = HttpResponse::newHttpJsonResponse(bad_answer);
        response->setStatusCode(k403Forbidden);
        callback(response);
        return;
    }
    
    MultiPartParser parser;
    if (parser.parse(request) != 0) {
        Json::Value bad_answer;
        bad_answer["status"]="bad";
        bad_answer["message"]="Invalid multipart data";
        auto response = HttpResponse::newHttpJsonResponse(bad_answer);
        response->setStatusCode(k400BadRequest);
        callback(response);
        return;
    }
    
    if (parser.getFiles().empty()) {
        Json::Value bad_answer;
        bad_answer["status"]="bad";
        bad_answer["message"]="No file uploaded";
        auto response = HttpResponse::newHttpJsonResponse(bad_answer);
        response->setStatusCode(k400BadRequest);
        callback(response);
        return;
    }
    
    auto& file = parser.getFiles()[0];
    auto& params = parser.getParameters();
    
    if (params.find("name") == params.end() || params.find("description") == params.end()){
        Json::Value bad_answer;
        bad_answer["status"]="bad";
        bad_answer["message"]="No name or description field";
        auto response = HttpResponse::newHttpJsonResponse(bad_answer);
        response->setStatusCode(k400BadRequest);
        callback(response);
        return;
    }

    string cat_name = params.at("name");
    string cat_description = params.at("description");

    string originalName = file.getFileName();
    string ext = originalName.substr(originalName.find_last_of("."));

    string newFilename = GenerateFileName(ext);
    
    string savePath = "./images/" + newFilename;
    if (!file.saveAs(savePath)) {
        Json::Value bad_answer;
        bad_answer["status"]="bad";
        bad_answer["message"]="Failed to save file";
        auto response = HttpResponse::newHttpJsonResponse(bad_answer);
        response->setStatusCode(k500InternalServerError);
        callback(response);
        return;
    }
    
    char* sql = sqlite3_mprintf("INSERT INTO cats (name, description, photo_filename) VALUES (%Q, %Q, %Q)",cat_name.c_str(),cat_description.c_str(),newFilename.c_str());
    db.Sql_exec(sql);
    sqlite3_free(sql);
    
    Json::Value resp;
    resp["status"] = "ok";
    resp["filename"] = newFilename;
    
    auto response = HttpResponse::newHttpJsonResponse(resp);
    response->setStatusCode(k201Created);
    response->addHeader("Access-Control-Allow-Origin", "http://localhost:3000");
    callback(response);
}