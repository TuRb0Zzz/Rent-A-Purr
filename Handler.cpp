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

void Handler::GetCats(const HttpRequestPtr& request, function<void(const HttpResponsePtr&)>&& callback) {
    cout << "GET /cats" << endl;
    Json::Value resp;
    Json::Value cats(Json::arrayValue);

    char* sql = sqlite3_mprintf(
        "SELECT cats.*, "
        "GROUP_CONCAT(tags.name) as tags "
        "FROM cats "
        "LEFT JOIN cat_tags ON cats.id = cat_tags.cat_id "
        "LEFT JOIN tags ON cat_tags.tag_id = tags.id "
        "GROUP BY cats.id"
    );

    db.Sql_request_callback(sql, [&cats](vector<string> output) {
        Json::Value buffer_cat;

        buffer_cat["id"] = stoi(output[0]);
        buffer_cat["name"] = output[1];
        buffer_cat["description"] = output[2];
        buffer_cat["breed"] = output[3];
        buffer_cat["age"] = output[4];
        buffer_cat["filename"] = output[5];
        
        Json::Value tags(Json::arrayValue);
        if (!output[6].empty()) {
            string tags_string = output[6];
            size_t pos = 0;
            string tag;
            while ((pos = tags_string.find(',')) != string::npos) {
                tag = tags_string.substr(0, pos);
                tags.append(tag);
                tags_string.erase(0, pos + 1);
            }
            tags.append(tags_string);
        }
        buffer_cat["tags"] = tags;
        
        Json::Value bookings(Json::arrayValue);
        
        
        char* bookings_sql = sqlite3_mprintf("SELECT start_time, end_time FROM bookings WHERE cat_id = %d AND status = 1 ORDER BY start_time",stoi(output[0]));
        
        db.Sql_request_callback(bookings_sql, [&bookings](vector<string> booking_output) {
            Json::Value tek_booking(Json::arrayValue);
            tek_booking.append(booking_output[0]);
            tek_booking.append(booking_output[1]);
            bookings.append(tek_booking);
        });
        
        sqlite3_free(bookings_sql);
        buffer_cat["bookings"] = bookings;

        Json::Value medical(Json::arrayValue);

        char* sql = sqlite3_mprintf("SELECT id, icon, label, color, bg FROM medical WHERE cat_id=%d",stoi(output[0]));

        db.Sql_request_callback(sql, [&medical](vector<string> medical_string) {
            Json::Value med;
            med["id"]=stoi(medical_string[0]);
            med["icon"]=medical_string[1];
            med["label"]=medical_string[2];
            med["color"]=medical_string[3];
            med["bg"]=medical_string[4];
            medical.append(med);
        });
        sqlite3_free(sql);
        buffer_cat["medical"]=medical;
        cats.append(buffer_cat);
    });

    sqlite3_free(sql);
    
    resp["status"] = "ok";
    resp["cats"] = cats;
    
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
    cout<<"POST /cats"<<endl;
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
    
    if (params.find("name") == params.end() || params.find("description") == params.end() || params.find("breed") == params.end() || params.find("age") == params.end()){
        Json::Value bad_answer;
        bad_answer["status"]="bad";
        bad_answer["message"]="Not enough parameters";
        auto response = HttpResponse::newHttpJsonResponse(bad_answer);
        response->setStatusCode(k400BadRequest);
        callback(response);
        return;
    }

    string cat_name = params.at("name");
    string cat_description = params.at("description");
    string cat_breed = params.at("breed");
    string cat_age = params.at("age");
    string cat_tags = params.find("tags") != params.end() ? params.at("tags") : "";
    string cat_medical = params.find("medical") != params.end() ? params.at("medical") : "";

    string originalName = file.getFileName();
    string ext = originalName.substr(originalName.find_last_of("."));

    string newFilename = GenerateFileName(ext);
    
    string savePath = "../images/" + newFilename;
    if (!file.saveAs(savePath)) {
        Json::Value bad_answer;
        bad_answer["status"]="bad";
        bad_answer["message"]="Failed to save file";
        auto response = HttpResponse::newHttpJsonResponse(bad_answer);
        response->setStatusCode(k500InternalServerError);
        callback(response);
        return;
    }
    
    char* sql = sqlite3_mprintf("INSERT INTO cats (name, description, breed, age, photo_filename) VALUES (%Q, %Q, %Q, %Q, %Q)",cat_name.c_str(),cat_description.c_str(),cat_breed.c_str(),cat_age.c_str(),newFilename.c_str());
    db.Sql_exec(sql);
    sqlite3_free(sql);
    
    int cat_id =sqlite3_last_insert_rowid(db.GetDataBaseLink());
    Json::Value tagsArray(Json::arrayValue);
    if(!cat_tags.empty()){
        vector<string> tag_names;
        size_t pos = 0;
        string temp_tags = cat_tags;
        while ((pos = temp_tags.find(',')) != string::npos) {
            string tag = temp_tags.substr(0, pos);

            tag.erase(0, tag.find_first_not_of(" \t"));
            tag.erase(tag.find_last_not_of(" \t") + 1);

            if (!tag.empty()) {
                tag_names.push_back(tag);
            }
            temp_tags.erase(0, pos + 1);
        }
        string tag = temp_tags;
        tag.erase(0, tag.find_first_not_of(" \t"));
        tag.erase(tag.find_last_not_of(" \t") + 1);
        if (!tag.empty()) {
            tag_names.push_back(tag);
        }

        for (const string& tag_name : tag_names) {
            tagsArray.append(tag_name);
            
            char* sql = sqlite3_mprintf("SELECT id FROM tags WHERE name = %Q",tag_name.c_str());
            int tag_id = -1;
            
            db.Sql_request_callback(sql, [&tag_id](vector<string> output) {
                if (!output.empty()) {
                    tag_id = stoi(output[0]);
                }
            });
            
            sqlite3_free(sql);

            if (tag_id == -1) {
                char* sql = sqlite3_mprintf("INSERT INTO tags (name) VALUES (%Q)",tag_name.c_str());
                db.Sql_exec(sql);
                sqlite3_free(sql);
                tag_id = sqlite3_last_insert_rowid(db.GetDataBaseLink());
            }
            if(tag_id!=-1){
                char* sql = sqlite3_mprintf("INSERT OR IGNORE INTO cat_tags (cat_id, tag_id) VALUES (%d, %d)",cat_id, tag_id);
                db.Sql_exec(sql);
                sqlite3_free(sql);
            }
        }
    }

    Json::Value medArray(Json::arrayValue);
    if(!cat_medical.empty()){
        vector<string> medical_fields;
        size_t pos = 0;
        string temp_medical = cat_medical;

        while ((pos = temp_medical.find(',')) != string::npos) {
            string field = temp_medical.substr(0, pos);

            field.erase(0, field.find_first_not_of(" \t"));
            field.erase(field.find_last_not_of(" \t") + 1);
            
            medical_fields.push_back(field);
            temp_medical.erase(0, pos + 1);
        }
        
        if (!temp_medical.empty()) {
            string last_field = temp_medical;
            last_field.erase(0, last_field.find_first_not_of(" \t"));
            last_field.erase(last_field.find_last_not_of(" \t") + 1);
            medical_fields.push_back(last_field);
        }
        
        if (medical_fields.size() % 4 != 0) {
            Json::Value bad_answer;
            bad_answer["status"] = "bad";
            bad_answer["message"] = "Medical fields must be multiple of 4. Each record requires icon, label, color, bg";
            bad_answer["received_fields"] = (int)medical_fields.size();
            bad_answer["received_data"] = cat_medical;            
            auto response = HttpResponse::newHttpJsonResponse(bad_answer);
            response->setStatusCode(k400BadRequest);
            response->addHeader("Access-Control-Allow-Origin", "http://localhost:3000");
            callback(response);
            return;
        }
        
        
        for (size_t i = 0; i + 3 < medical_fields.size(); i += 4) {
            string icon = medical_fields[i];
            string label = medical_fields[i + 1];
            string color = medical_fields[i + 2];
            string bg = medical_fields[i + 3];
            
            char* med_sql = sqlite3_mprintf("INSERT INTO medical (cat_id, icon, label, color, bg) VALUES (%d, %Q, %Q, %Q, %Q)",cat_id, icon.c_str(), label.c_str(), color.c_str(), bg.c_str());
            
            if (db.Sql_exec(med_sql)) {
                Json::Value medRecord;
                medRecord["id"] = sqlite3_last_insert_rowid(db.GetDataBaseLink());
                medRecord["icon"] = icon;
                medRecord["label"] = label;
                medRecord["color"] = color;
                medRecord["bg"] = bg;
                medArray.append(medRecord);
            }
            
            sqlite3_free(med_sql);
        }
    }

    Json::Value resp;
    resp["status"] = "ok";
    resp["name"]=cat_name;
    resp["description"]=cat_description;
    resp["breed"]=cat_breed;
    resp["age"]=cat_age;
    resp["filename"] = newFilename;
    resp["tags"] = tagsArray;
    resp["medical"]=medArray;
    
    auto response = HttpResponse::newHttpJsonResponse(resp);
    response->setStatusCode(k201Created);
    response->addHeader("Access-Control-Allow-Origin", "http://localhost:3000");
    callback(response);
}

void Handler::updateCatTagsAndMedical(const HttpRequestPtr& request, function<void(const HttpResponsePtr&)>&& callback) {
    string url = request->getPath();
    size_t poz = url.find_last_of('/');
    string id = url.substr(poz+1);
    int cat_id=-1;
    try{
        cat_id = stoi(id);
    }catch(const std::exception& e){
        Json::Value resp;
        resp["id"] = id;
        resp["path"] = url;
        resp["error"] = "not a number";
        auto responce = HttpResponse::newHttpJsonResponse(resp);
        callback(responce);
        return;
    }
    cout << "PUT /cats/" << cat_id << endl;
    string sessionId = request->getCookie("session_id");
    int user_id = checkAuth(sessionId);
    
    if (user_id == -1) {
        Json::Value resp;
        resp["status"] = "bad";
        resp["message"] = "Unauthorized";
        auto response = HttpResponse::newHttpJsonResponse(resp);
        response->setStatusCode(k401Unauthorized);
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:3000");
        callback(response);
        return;
    }
    
    if (!CheckPermissions(user_id)) {
        Json::Value resp;
        resp["status"] = "bad";
        resp["message"] = "Access Denied";
        auto response = HttpResponse::newHttpJsonResponse(resp);
        response->setStatusCode(k403Forbidden);
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:3000");
        callback(response);
        return;
    }
    
    char* check_sql = sqlite3_mprintf("SELECT id FROM cats WHERE id = %d", cat_id);
    vector<vector<string>> check_result = db.Sql_request_vector(check_sql);
    sqlite3_free(check_sql);
    
    if (check_result.empty()) {
        Json::Value resp;
        resp["status"] = "bad";
        resp["message"] = "Cat not found";
        auto response = HttpResponse::newHttpJsonResponse(resp);
        response->setStatusCode(k404NotFound);
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:3000");
        callback(response);
        return;
    }
    
    auto json = request->getJsonObject();
    if (!json) {
        Json::Value resp;
        resp["status"] = "bad";
        resp["message"] = "Invalid JSON";
        auto response = HttpResponse::newHttpJsonResponse(resp);
        response->setStatusCode(k400BadRequest);
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:3000");
        callback(response);
        return;
    }
    
    Json::Value response_data;
    response_data["status"] = "ok";
    response_data["cat_id"] = cat_id;
    
    if (json->isMember("tags")) {
        string tags_string = (*json)["tags"].asString();
        
        char* delete_tags_sql = sqlite3_mprintf("DELETE FROM cat_tags WHERE cat_id = %d", cat_id);
        db.Sql_exec(delete_tags_sql);
        sqlite3_free(delete_tags_sql);
        
        Json::Value tagsArray(Json::arrayValue);
        
        if (!tags_string.empty()) {
            vector<string> tag_names;
            size_t pos = 0;
            string temp_tags = tags_string;
            
            while ((pos = temp_tags.find(',')) != string::npos) {
                string tag = temp_tags.substr(0, pos);
                tag.erase(0, tag.find_first_not_of(" \t"));
                tag.erase(tag.find_last_not_of(" \t") + 1);
                if (!tag.empty()) {
                    tag_names.push_back(tag);
                }
                temp_tags.erase(0, pos + 1);
            }
            
            if (!temp_tags.empty()) {
                string last_tag = temp_tags;
                last_tag.erase(0, last_tag.find_first_not_of(" \t"));
                last_tag.erase(last_tag.find_last_not_of(" \t") + 1);
                if (!last_tag.empty()) {
                    tag_names.push_back(last_tag);
                }
            }
            
            for (const string& tag_name : tag_names) {
                if (tag_name.empty()) continue;
                
                tagsArray.append(tag_name);
                
                char* select_sql = sqlite3_mprintf("SELECT id FROM tags WHERE name = %Q", tag_name.c_str());
                vector<vector<string>> tag_result = db.Sql_request_vector(select_sql);
                sqlite3_free(select_sql);
                
                int tag_id = -1;
                
                if (!tag_result.empty()) {
                    tag_id = stoi(tag_result[0][0]);
                } else {
                    char* insert_sql = sqlite3_mprintf("INSERT INTO tags (name) VALUES (%Q)", tag_name.c_str());
                    if (db.Sql_exec(insert_sql)) {
                        tag_id = sqlite3_last_insert_rowid(db.GetDataBaseLink());
                    }
                    sqlite3_free(insert_sql);
                }
                
                if (tag_id != -1) {
                    char* link_sql = sqlite3_mprintf("INSERT INTO cat_tags (cat_id, tag_id) VALUES (%d, %d)",cat_id, tag_id);
                    db.Sql_exec(link_sql);
                    sqlite3_free(link_sql);
                }
            }
        }
        
        response_data["tags"] = tagsArray;
    }
    
    if (json->isMember("medical")) {
        string medical_string = (*json)["medical"].asString();
        
        char* delete_medical_sql = sqlite3_mprintf("DELETE FROM medical WHERE cat_id = %d", cat_id);
        db.Sql_exec(delete_medical_sql);
        sqlite3_free(delete_medical_sql);
        
        Json::Value medArray(Json::arrayValue);
        
        if (!medical_string.empty()) {
            vector<string> medical_fields;
            size_t pos = 0;
            string temp_medical = medical_string;
            
            while ((pos = temp_medical.find(',')) != string::npos) {
                string field = temp_medical.substr(0, pos);
                field.erase(0, field.find_first_not_of(" \t"));
                field.erase(field.find_last_not_of(" \t") + 1);
                medical_fields.push_back(field);
                temp_medical.erase(0, pos + 1);
            }
            
            if (!temp_medical.empty()) {
                string last_field = temp_medical;
                last_field.erase(0, last_field.find_first_not_of(" \t"));
                last_field.erase(last_field.find_last_not_of(" \t") + 1);
                medical_fields.push_back(last_field);
            }

            if (medical_fields.size() % 4 != 0) {
                Json::Value error_resp;
                error_resp["status"] = "bad";
                error_resp["message"] = "Medical fields must be multiple of 4";
                error_resp["received_fields"] = (int)medical_fields.size();
                error_resp["received_data"] = medical_string;
                
                auto response = HttpResponse::newHttpJsonResponse(error_resp);
                response->setStatusCode(k400BadRequest);
                response->addHeader("Access-Control-Allow-Origin", "http://localhost:3000");
                callback(response);
                return;
            }
            
            for (size_t i = 0; i + 3 < medical_fields.size(); i += 4) {
                string icon = medical_fields[i];
                string label = medical_fields[i + 1];
                string color = medical_fields[i + 2];
                string bg = medical_fields[i + 3];
                
                if (icon.empty() || label.empty() || color.empty() || bg.empty()) {
                    Json::Value error_resp;
                    error_resp["status"] = "bad";
                    error_resp["message"] = "Medical fields cannot be empty";
                    error_resp["record_index"] = (int)(i / 4);
                    
                    auto response = HttpResponse::newHttpJsonResponse(error_resp);
                    response->setStatusCode(k400BadRequest);
                    response->addHeader("Access-Control-Allow-Origin", "http://localhost:3000");
                    callback(response);
                    return;
                }
                
                char* med_sql = sqlite3_mprintf(
                    "INSERT INTO medical (cat_id, icon, label, color, bg) VALUES (%d, %Q, %Q, %Q, %Q)",
                    cat_id, icon.c_str(), label.c_str(), color.c_str(), bg.c_str()
                );
                
                if (db.Sql_exec(med_sql)) {
                    Json::Value medRecord;
                    medRecord["id"] = sqlite3_last_insert_rowid(db.GetDataBaseLink());
                    medRecord["icon"] = icon;
                    medRecord["label"] = label;
                    medRecord["color"] = color;
                    medRecord["bg"] = bg;
                    medArray.append(medRecord);
                }
                
                sqlite3_free(med_sql);
            }
        }
        
        response_data["medical"] = medArray;
    }
    
    if (!json->isMember("tags") && !json->isMember("medical")) {
        response_data["message"] = "No updates provided. Use 'tags' or 'medical' fields";
    }
    
    auto response = HttpResponse::newHttpJsonResponse(response_data);
    response->setStatusCode(k200OK);
    response->addHeader("Access-Control-Allow-Origin", "http://localhost:3000");
    callback(response);
}

void Handler::AddToBookings(const HttpRequestPtr& request, function<void(const HttpResponsePtr&)>&& callback){
    cout<<"POST /bookings"<<endl;
    string sessionId = request->getCookie("session_id");
    int user_id = checkAuth(sessionId);
    
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

    if (!json->isMember("start") || !json->isMember("end") || !json->isMember("id")) {
        Json::Value resp;
        resp["status"] = "bad";
        resp["message"] = "Missing required fields: start or end or id";
        auto response = HttpResponse::newHttpJsonResponse(resp);
        response->setStatusCode(k400BadRequest);
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:3000");
        callback(response);
        return;
    }

    int cat_id = (*json)["id"].asInt();
    string start  = (*json)["start"].asString();
    string end = (*json)["end"].asString();


    if (user_id == -1) {
        Json::Value resp;
        resp["status"] = "bad";
        resp["message"] = "Unauthorized";
        auto response = HttpResponse::newHttpJsonResponse(resp);
        response->setStatusCode(k401Unauthorized);
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:3000");
        callback(response);
        return;
    }

    char* check_cat_sql = sqlite3_mprintf("SELECT id FROM cats WHERE id = %d", cat_id);
    vector<vector<string>> cat_check = db.Sql_request_vector(check_cat_sql);
    sqlite3_free(check_cat_sql);
    
    if (cat_check.empty()) {
        Json::Value resp;
        resp["status"] = "bad";
        resp["message"] = "Cat not found";
        auto response = HttpResponse::newHttpJsonResponse(resp);
        response->setStatusCode(k404NotFound);
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:3000");
        callback(response);
        return;
    }

    char* check_booking_sql = sqlite3_mprintf(
        "SELECT id FROM bookings WHERE cat_id = %d AND "
        "((start_time <= %Q AND end_time > %Q) OR "
        "(start_time < %Q AND end_time >= %Q) OR "
        "(start_time >= %Q AND end_time <= %Q))",
        cat_id, 
        start.c_str(), start.c_str(),
        end.c_str(), end.c_str(),
        start.c_str(), end.c_str()
    );
    
    vector<vector<string>> booking_check = db.Sql_request_vector(check_booking_sql);
    sqlite3_free(check_booking_sql);
    
    if (!booking_check.empty()) {
        Json::Value resp;
        resp["status"] = "bad";
        resp["message"] = "Cat is already booked for this time period";
        auto response = HttpResponse::newHttpJsonResponse(resp);
        response->setStatusCode(k409Conflict);
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:3000");
        callback(response);
        return;
    }

    char* insert_sql = sqlite3_mprintf("INSERT INTO bookings (cat_id, user_id, start_time, end_time) VALUES (%d, %d, %Q, %Q)",cat_id, user_id, start.c_str(), end.c_str());
    bool success = db.Sql_exec(insert_sql);
    int booking_id = sqlite3_last_insert_rowid(db.GetDataBaseLink());
    sqlite3_free(insert_sql);
    
    if (!success) {
        Json::Value resp;
        resp["status"] = "bad";
        resp["message"] = "Failed to create booking";
        auto response = HttpResponse::newHttpJsonResponse(resp);
        response->setStatusCode(k500InternalServerError);
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:3000");
        callback(response);
        return;
    }
    
    char* user_sql = sqlite3_mprintf("SELECT username, nickname FROM users WHERE id = %d", user_id);
    vector<vector<string>> user_info = db.Sql_request_vector(user_sql);
    sqlite3_free(user_sql);
    
    Json::Value resp;
    resp["status"] = "ok";
    resp["message"] = "Booking created successfully";
    resp["booking"]["id"] = booking_id;
    resp["booking"]["cat_id"] = cat_id;
    resp["booking"]["user_id"] = user_id;
    resp["booking"]["start_time"] = start;
    resp["booking"]["end_time"] = end;
    
    if (!user_info.empty()) {
        resp["booking"]["user"]["username"] = user_info[0][0];
        resp["booking"]["user"]["nickname"] = user_info[0][1];
    }
    
    auto response = HttpResponse::newHttpJsonResponse(resp);
    response->setStatusCode(k201Created);
    response->addHeader("Access-Control-Allow-Origin", "http://localhost:3000");
    callback(response);
}

void Handler::GetAdminBookings(const HttpRequestPtr& request, function<void(const HttpResponsePtr&)>&& callback) {
    cout << "GET /bookings/admin" << endl;
    
    string sessionId = request->getCookie("session_id");
    int user_id = checkAuth(sessionId);
    
    if (user_id == -1) {
        Json::Value resp;
        resp["status"] = "bad";
        resp["message"] = "Unauthorized";
        auto response = HttpResponse::newHttpJsonResponse(resp);
        response->setStatusCode(k401Unauthorized);
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:3000");
        callback(response);
        return;
    }
    
    if (!CheckPermissions(user_id)) {
        Json::Value resp;
        resp["status"] = "bad";
        resp["message"] = "Access Denied";
        auto response = HttpResponse::newHttpJsonResponse(resp);
        response->setStatusCode(k403Forbidden);
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:3000");
        callback(response);
        return;
    }
    
    Json::Value bookings(Json::arrayValue);
    
    char* sql = sqlite3_mprintf(
        "SELECT b.id, b.cat_id, c.name as cat_name, b.user_id, u.nickname, "
        "b.start_time, b.end_time "
        "FROM bookings b "
        "JOIN cats c ON b.cat_id = c.id "
        "JOIN users u ON b.user_id = u.id "
        "WHERE b.status = 0 "
        "ORDER BY b.start_time"
    );
    
    db.Sql_request_callback(sql, [&bookings](vector<string> output) {
        Json::Value booking;
        booking["id"] = stoi(output[0]);
        booking["cat_id"] = stoi(output[1]);
        booking["cat_name"] = output[2];
        booking["user_id"] = stoi(output[3]);
        booking["nickname"] = output[4];
        booking["start_time"] = output[5];
        booking["end_time"] = output[6];
        bookings.append(booking);
    });
    
    sqlite3_free(sql);
    
    Json::Value resp;
    resp["status"] = "ok";
    resp["bookings"] = bookings;
    
    auto response = HttpResponse::newHttpJsonResponse(resp);
    response->setStatusCode(k200OK);
    response->addHeader("Access-Control-Allow-Origin", "http://localhost:3000");
    callback(response);
}

void Handler::ConfirmAdminBookings(const HttpRequestPtr& request, function<void(const HttpResponsePtr&)>&& callback){
    cout << "PUT /admin/bookings/"<<endl;
    string sessionId = request->getCookie("session_id");
    int user_id = checkAuth(sessionId);
    
    if (user_id == -1) {
        Json::Value resp;
        resp["status"] = "bad";
        resp["message"] = "Unauthorized";
        auto response = HttpResponse::newHttpJsonResponse(resp);
        response->setStatusCode(k401Unauthorized);
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:3000");
        callback(response);
        return;
    }
    
    if (!CheckPermissions(user_id)) {
        Json::Value resp;
        resp["status"] = "bad";
        resp["message"] = "Access Denied";
        auto response = HttpResponse::newHttpJsonResponse(resp);
        response->setStatusCode(k403Forbidden);
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:3000");
        callback(response);
        return;
    }

    auto json = request->getJsonObject();
    if (!json) {
        Json::Value resp;
        resp["status"] = "bad";
        resp["message"] = "Invalid JSON";
        auto response = HttpResponse::newHttpJsonResponse(resp);
        response->setStatusCode(k400BadRequest);
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:3000");
        callback(response);
        return;
    }

    if (!json->isMember("booking_id")) {
        Json::Value resp;
        resp["status"] = "bad";
        resp["message"] = "Missing booking_id field";
        auto response = HttpResponse::newHttpJsonResponse(resp);
        response->setStatusCode(k400BadRequest);
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:3000");
        callback(response);
        return;
    }

    int booking_id = (*json)["booking_id"].asInt();

    char* check_sql = sqlite3_mprintf("SELECT id, status FROM bookings WHERE id = %d", booking_id);
    vector<vector<string>> check = db.Sql_request_vector(check_sql);
    sqlite3_free(check_sql);

    if (check.empty()) {
        Json::Value resp;
        resp["status"] = "bad";
        resp["message"] = "Booking not found";
        auto response = HttpResponse::newHttpJsonResponse(resp);
        response->setStatusCode(k404NotFound);
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:3000");
        callback(response);
        return;
    }

    int current_status = stoi(check[0][1]);
    if (current_status == 1) {
        Json::Value resp;
        resp["status"] = "bad";
        resp["message"] = "Booking is already confirmed";
        auto response = HttpResponse::newHttpJsonResponse(resp);
        response->setStatusCode(k400BadRequest);
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:3000");
        callback(response);
        return;
    }

    char* update_sql = sqlite3_mprintf("UPDATE bookings SET status = 1 WHERE id = %d", booking_id);
    bool success = db.Sql_exec(update_sql);
    sqlite3_free(update_sql);


    if (!success) {
        Json::Value resp;
        resp["status"] = "bad";
        resp["message"] = "Failed to confirm booking";
        auto response = HttpResponse::newHttpJsonResponse(resp);
        response->setStatusCode(k500InternalServerError);
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:3000");
        callback(response);
        return;
    }

    Json::Value resp;
    resp["status"] = "ok";
    resp["message"] = "Booking confirmed successfully";
    auto response = HttpResponse::newHttpJsonResponse(resp);
    response->setStatusCode(k200OK);
    response->addHeader("Access-Control-Allow-Origin", "http://localhost:3000");
    callback(response);
}

void Handler::RejectAdminBooking(const HttpRequestPtr& request, function<void(const HttpResponsePtr&)>&& callback){
    cout << "DELETE /admin/bookings" << endl;
    
    string sessionId = request->getCookie("session_id");
    int user_id = checkAuth(sessionId);
    
    if (user_id == -1) {
        Json::Value resp;
        resp["status"] = "bad";
        resp["message"] = "Unauthorized";
        auto response = HttpResponse::newHttpJsonResponse(resp);
        response->setStatusCode(k401Unauthorized);
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:3000");
        callback(response);
        return;
    }
    
    if (!CheckPermissions(user_id)) {
        Json::Value resp;
        resp["status"] = "bad";
        resp["message"] = "Access Denied";
        auto response = HttpResponse::newHttpJsonResponse(resp);
        response->setStatusCode(k403Forbidden);
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:3000");
        callback(response);
        return;
    }

    auto json = request->getJsonObject();
    if (!json) {
        Json::Value resp;
        resp["status"] = "bad";
        resp["message"] = "Invalid JSON";
        auto response = HttpResponse::newHttpJsonResponse(resp);
        response->setStatusCode(k400BadRequest);
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:3000");
        callback(response);
        return;
    }

    if (!json->isMember("booking_id")) {
        Json::Value resp;
        resp["status"] = "bad";
        resp["message"] = "Missing booking_id field";
        auto response = HttpResponse::newHttpJsonResponse(resp);
        response->setStatusCode(k400BadRequest);
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:3000");
        callback(response);
        return;
    }

    int booking_id = (*json)["booking_id"].asInt();

    char* check_sql = sqlite3_mprintf("SELECT id, status FROM bookings WHERE id = %d", booking_id);
    vector<vector<string>> check = db.Sql_request_vector(check_sql);
    sqlite3_free(check_sql);

    if (check.empty()) {
        Json::Value resp;
        resp["status"] = "bad";
        resp["message"] = "Booking not found";
        auto response = HttpResponse::newHttpJsonResponse(resp);
        response->setStatusCode(k404NotFound);
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:3000");
        callback(response);
        return;
    }

    int current_status = stoi(check[0][1]);
    if (current_status == 1) {
        Json::Value resp;
        resp["status"] = "bad";
        resp["message"] = "Cannot delete confirmed booking. Use cancel instead.";
        auto response = HttpResponse::newHttpJsonResponse(resp);
        response->setStatusCode(k400BadRequest);
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:3000");
        callback(response);
        return;
    }

    char* delete_sql = sqlite3_mprintf("DELETE FROM bookings WHERE id = %d", booking_id);
    bool success = db.Sql_exec(delete_sql);
    sqlite3_free(delete_sql);

    if (!success) {
        Json::Value resp;
        resp["status"] = "bad";
        resp["message"] = "Failed to delete booking";
        auto response = HttpResponse::newHttpJsonResponse(resp);
        response->setStatusCode(k500InternalServerError);
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:3000");
        callback(response);
        return;
    }

    Json::Value resp;
    resp["status"] = "ok";
    resp["message"] = "Booking rejected and deleted successfully";
    auto response = HttpResponse::newHttpJsonResponse(resp);
    response->setStatusCode(k200OK);
    response->addHeader("Access-Control-Allow-Origin", "http://localhost:3000");
    callback(response);
}

