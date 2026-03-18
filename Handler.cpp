#include "Handler.h"
#include <json/json.h>     
#include <iostream>         
#include <string>
#include <sqlite3.h>
using namespace drogon;     
using namespace std;

extern DataBase db;

int Handler::IdByLogin(string username){
    char* sql = sqlite3_mprintf("SELECT id FROM users WHERE username = %Q",username);
    vector<vector<string>> output = db.Sql_request_vector(sql);
    sqlite3_free(sql);
    if(output.empty()){
        return -1;
    }
    return stoi(output[0][0]);
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
    if(result.empty() || result[0][0]=="0"){
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

    char* sql = sqlite3_mprintf("SELECT user_id FROM sessions WHERE user_id = %d", id);
    auto existing = db.Sql_request_vector(sql);
    sqlite3_free(sql);
    
    if(!existing.empty()){
        char* get_sql = sqlite3_mprintf("SELECT session_id FROM sessions WHERE user_id = %d", id);
        auto existing_session = db.Sql_request_vector(get_sql);
        sqlite3_free(get_sql);
        
        if (!existing_session.empty()) {
            return existing_session[0][0];
        }
        return "";
    }
    
    sql = sqlite3_mprintf("INSERT INTO sessions (session_id, user_id, expires_at) VALUES (%Q, %d, datetime('now', '+1 day'))", sessionId.c_str(), id);
    bool success = db.Sql_exec(sql);
    sqlite3_free(sql);
    
    if (success) {
        return sessionId;
    }
    
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
    cout << request->getMethodString() << " " << request->getPath() << endl;

    auto json = request->getJsonObject();
    if(!json){
        Json::Value bad_answer;
        bad_answer["status"]="bad";
        bad_answer["message"]="Bad Json";
        auto response = HttpResponse::newHttpJsonResponse(bad_answer);
        response->setStatusCode(k400BadRequest);
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
        response->addHeader("Access-Control-Allow-Credentials", "true");
        callback(response);
        return;
    }
    
    string username = (*json)["username"].asString();
    string password = (*json)["password"].asString();
    string nickname = (*json)["nickname"].asString();
    password = hashPassword(password);
    
    char *sql = sqlite3_mprintf("SELECT id FROM users WHERE username=%Q", username.c_str());
    vector<vector<string>> check = db.Sql_request_vector(sql);
    sqlite3_free(sql);
    
    if(!check.empty()){
        Json::Value bad_answer;
        bad_answer["status"]="bad";
        bad_answer["message"]="User already exists";
        auto response = HttpResponse::newHttpJsonResponse(bad_answer);
        response->setStatusCode(k409Conflict);
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
        response->addHeader("Access-Control-Allow-Credentials", "true");
        callback(response);
        return;
    }
    
    sql = sqlite3_mprintf("INSERT INTO users (username, password, nickname) VALUES (%Q, %Q, %Q)", 
                          username.c_str(), password.c_str(), nickname.c_str());
    if(!(db.Sql_exec(sql))){
        sqlite3_free(sql);
        Json::Value bad_answer;
        bad_answer["status"]="bad";
        bad_answer["message"]="Cant insert Data";
        auto response = HttpResponse::newHttpJsonResponse(bad_answer);
        response->setStatusCode(k500InternalServerError);
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
        response->addHeader("Access-Control-Allow-Credentials", "true");
        callback(response);
        return;
    }
    sqlite3_free(sql);

    long long id = db.GetLastInsertId();
    string session_id = createSession(id);

    Json::Value resp;
    resp["status"] = "ok";
    resp["message"] = "User created";
    resp["user"]["nickname"] = nickname;
    resp["user"]["user_id"] = id;

    auto response = HttpResponse::newHttpJsonResponse(resp);
    
    if(session_id != ""){
        string cookie = "session_id=" + session_id + "; Max-Age=86400; Path=/; HttpOnly; SameSite=None; Secure";
        response->addHeader("Set-Cookie", cookie);
        response->addHeader("Access-Control-Allow-Credentials", "true");
    }
    
    response->setStatusCode(k201Created);
    response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
    response->addHeader("Access-Control-Allow-Credentials", "true");
    callback(response);
}

void Handler::AutoriseUser(const HttpRequestPtr& request,function<void(const HttpResponsePtr&)>&& callback){
    cout << request->getMethodString() << " " << request->getPath() << endl;

    auto json = request->getJsonObject();
    if(!json){
        Json::Value bad_answer;
        bad_answer["status"]="bad";
        bad_answer["message"]="Bad Json";
        auto response = HttpResponse::newHttpJsonResponse(bad_answer);
        response->setStatusCode(k400BadRequest);
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
        response->addHeader("Access-Control-Allow-Credentials", "true");
        callback(response);
        return;
    }
    
    string username = (*json)["username"].asString();
    string password = (*json)["password"].asString();

    char* sql = sqlite3_mprintf("SELECT id, password, nickname FROM users WHERE username=%Q", username.c_str());
    vector<vector<string>> check = db.Sql_request_vector(sql);
    sqlite3_free(sql);
    
    if(check.empty()){
        Json::Value bad_answer;
        bad_answer["status"]="bad";
        bad_answer["message"]="Wrong username or password";
        auto response = HttpResponse::newHttpJsonResponse(bad_answer);
        response->setStatusCode(k401Unauthorized);
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
        response->addHeader("Access-Control-Allow-Credentials", "true");
        callback(response);
        return;
    }
    
    if(!verifyPassword(password, check[0][1])){
        Json::Value bad_answer;
        bad_answer["status"]="bad";
        bad_answer["message"]="Wrong username or password";
        auto response = HttpResponse::newHttpJsonResponse(bad_answer);
        response->setStatusCode(k401Unauthorized);
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
        response->addHeader("Access-Control-Allow-Credentials", "true");
        callback(response);
        return;
    }

    string session_id = createSession(stoi(check[0][0]));

    Json::Value resp;
    resp["status"] = "ok";
    resp["message"] = "User authorized";
    resp["user"]["nickname"] = check[0][2];
    resp["user"]["user_id"] = stoi(check[0][0]);
    
    auto response = HttpResponse::newHttpJsonResponse(resp);

    if(session_id != ""){
        string cookie = "session_id=" + session_id + "; Max-Age=86400; Path=/; HttpOnly; SameSite=None; Secure";
        response->addHeader("Set-Cookie", cookie);
        response->addHeader("Access-Control-Allow-Credentials", "true");
    }
    
    response->setStatusCode(k200OK);
    response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
    response->addHeader("Access-Control-Allow-Credentials", "true");
    callback(response);
}

void Handler::GetCats(const HttpRequestPtr& request, function<void(const HttpResponsePtr&)>&& callback) {
    cout << request->getMethodString() << " " << request->getPath() << endl;

    Json::Value resp;
    Json::Value cats(Json::arrayValue);

    char* sql = sqlite3_mprintf(
        "SELECT cats.id, cats.name, cats.description, cats.breed, cats.age, "
        "GROUP_CONCAT(tags.name) as tags "
        "FROM cats "
        "LEFT JOIN cat_tags ON cats.id = cat_tags.cat_id "
        "LEFT JOIN tags ON cat_tags.tag_id = tags.id "
        "GROUP BY cats.id"
    );

    vector<vector<string>> cats_data = db.Sql_request_vector(sql);
    sqlite3_free(sql);

    sql = sqlite3_mprintf(
        "SELECT cat_id, filename FROM cat_photos ORDER BY cat_id, id"
    );
    
    vector<vector<string>> all_photos = db.Sql_request_vector(sql);
    sqlite3_free(sql);
    
    map<int, vector<string>> photos_by_cat;
    for (const auto& photo : all_photos) {
        int cat_id = stoi(photo[0]);
        photos_by_cat[cat_id].push_back(photo[1]);
    }

    sql = sqlite3_mprintf(
        "SELECT cat_id, user_id, start_time, end_time "
        "FROM bookings WHERE status = 1 "
        "ORDER BY cat_id, start_time"
    );

    vector<vector<string>> all_bookings = db.Sql_request_vector(sql);
    sqlite3_free(sql);

    map<int, vector<vector<string>>> bookings_by_cat;
    for (const auto& booking : all_bookings) {
        int cat_id = stoi(booking[0]);
        bookings_by_cat[cat_id].push_back(booking);
    }

    sql = sqlite3_mprintf(
        "SELECT cat_id, id, icon, label, color, bg "
        "FROM medical ORDER BY cat_id"
    );

    vector<vector<string>> all_medical = db.Sql_request_vector(sql);
    sqlite3_free(sql);

    map<int, vector<vector<string>>> medical_by_cat;
    for (const auto& med : all_medical) {
        int cat_id = stoi(med[0]);
        medical_by_cat[cat_id].push_back(med);
    }

    for (const auto& cat_row : cats_data) {
        Json::Value cat;
        int cat_id = stoi(cat_row[0]);

        cat["id"] = cat_id;
        cat["name"] = cat_row[1];
        cat["description"] = cat_row[2];
        cat["breed"] = cat_row[3];
        cat["age"] = cat_row[4];

        Json::Value tags(Json::arrayValue);
        if (!cat_row[5].empty()) {
            string tags_string = cat_row[5];
            size_t pos = 0;
            while ((pos = tags_string.find(',')) != string::npos) {
                tags.append(tags_string.substr(0, pos));
                tags_string.erase(0, pos + 1);
            }
            tags.append(tags_string);
        }
        cat["tags"] = tags;

        Json::Value filenames(Json::arrayValue);
        if (photos_by_cat.count(cat_id)) {
            for (const auto& filename : photos_by_cat[cat_id]) {
                filenames.append(filename);
            }
        }
        cat["filenames"] = filenames;

        Json::Value bookings(Json::arrayValue);
        if (bookings_by_cat.count(cat_id)) {
            for (const auto& booking : bookings_by_cat[cat_id]) {
                Json::Value tek_booking(Json::arrayValue);
                tek_booking.append(booking[1]);
                tek_booking.append(booking[2]);
                tek_booking.append(booking[3]);
                bookings.append(tek_booking);
            }
        }
        cat["bookings"] = bookings;

        Json::Value medical(Json::arrayValue);
        if (medical_by_cat.count(cat_id)) {
            for (const auto& med : medical_by_cat[cat_id]) {
                Json::Value med_record;
                med_record["id"] = stoi(med[1]);
                med_record["icon"] = med[2];
                med_record["label"] = med[3];
                med_record["color"] = med[4];
                med_record["bg"] = med[5];
                medical.append(med_record);
            }
        }
        cat["medical"] = medical;

        cats.append(cat);
    }

    resp["status"] = "ok";
    resp["cats"] = cats;

    auto response = HttpResponse::newHttpJsonResponse(resp);
    response->setStatusCode(k200OK);
    response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
    response->addHeader("Access-Control-Allow-Credentials", "true");
    callback(response);
}

void Handler::uploadCatPhoto(const HttpRequestPtr& request, function<void(const HttpResponsePtr&)>&& callback) {
    cout << request->getMethodString() << " " << request->getPath() << endl;

    string sessionId = request->getCookie("session_id");
    int user_id = checkAuth(sessionId);
    if (user_id == -1) {
        Json::Value bad_answer;
        bad_answer["status"]="bad";
        bad_answer["message"]="Unauthorized";
        auto response = HttpResponse::newHttpJsonResponse(bad_answer);
        response->setStatusCode(k401Unauthorized);
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
        response->addHeader("Access-Control-Allow-Credentials", "true");
        callback(response);
        return;
    }
    if(!CheckPermissions(user_id)){
        Json::Value bad_answer;
        bad_answer["status"]="bad";
        bad_answer["message"]="Access Denied";
        auto response = HttpResponse::newHttpJsonResponse(bad_answer);
        response->setStatusCode(k403Forbidden);
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
        response->addHeader("Access-Control-Allow-Credentials", "true");
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
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
        response->addHeader("Access-Control-Allow-Credentials", "true");
        callback(response);
        return;
    }
    
    auto& files = parser.getFiles();
    auto& params = parser.getParameters();
    
    if (files.empty()) {
        Json::Value bad_answer;
        bad_answer["status"]="bad";
        bad_answer["message"]="No file uploaded";
        auto response = HttpResponse::newHttpJsonResponse(bad_answer);
        response->setStatusCode(k400BadRequest);
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
        response->addHeader("Access-Control-Allow-Credentials", "true");
        callback(response);
        return;
    }
    
    if (params.find("name") == params.end() || params.find("description") == params.end() || params.find("breed") == params.end() || params.find("age") == params.end()){
        Json::Value bad_answer;
        bad_answer["status"]="bad";
        bad_answer["message"]="Not enough parameters";
        auto response = HttpResponse::newHttpJsonResponse(bad_answer);
        response->setStatusCode(k400BadRequest);
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
        response->addHeader("Access-Control-Allow-Credentials", "true");
        callback(response);
        return;
    }

    string cat_name = params.at("name");
    string cat_description = params.at("description");
    string cat_breed = params.at("breed");
    string cat_age = params.at("age");
    string cat_tags = params.find("tags") != params.end() ? params.at("tags") : "";
    string cat_medical = params.find("medical") != params.end() ? params.at("medical") : "";

    char* sql = sqlite3_mprintf("INSERT INTO cats (name, description, breed, age) VALUES (%Q, %Q, %Q, %Q)", cat_name.c_str(), cat_description.c_str(), cat_breed.c_str(), cat_age.c_str());
    db.Sql_exec(sql);
    sqlite3_free(sql);
    
    int cat_id = sqlite3_last_insert_rowid(db.GetDataBaseLink());
    
    vector<string> savedFilenames;
    
    for (const auto& file : files) {
        string originalName = file.getFileName();
        string ext = originalName.substr(originalName.find_last_of("."));
        string newFilename = GenerateFileName(ext);
        string savePath = "../images/" + newFilename;
        
        if (!file.saveAs(savePath)) {
            for (const auto& filename : savedFilenames) {
                string path = "../images/" + filename;
                remove(path.c_str());
            }

            char* delete_cat_sql = sqlite3_mprintf("DELETE FROM cats WHERE id = %d", cat_id);
            db.Sql_exec(delete_cat_sql);
            sqlite3_free(delete_cat_sql);
            
            Json::Value bad_answer;
            bad_answer["status"]="bad";
            bad_answer["message"]="Failed to save file: " + originalName;
            auto response = HttpResponse::newHttpJsonResponse(bad_answer);
            response->setStatusCode(k500InternalServerError);
            response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
            response->addHeader("Access-Control-Allow-Credentials", "true");
            callback(response);
            return;
        }
        
        char* photo_sql = sqlite3_mprintf("INSERT INTO cat_photos (cat_id, filename) VALUES (%d, %Q)", cat_id, newFilename.c_str());
        db.Sql_exec(photo_sql);
        sqlite3_free(photo_sql);
        
        savedFilenames.push_back(newFilename);
    }

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
            
            char* sql = sqlite3_mprintf("SELECT id FROM tags WHERE name = %Q", tag_name.c_str());
            int tag_id = -1;
            
            vector<vector<string>> tag_result = db.Sql_request_vector(sql);
            sqlite3_free(sql);
            
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
                char* link_sql = sqlite3_mprintf("INSERT OR IGNORE INTO cat_tags (cat_id, tag_id) VALUES (%d, %d)", cat_id, tag_id);
                db.Sql_exec(link_sql);
                sqlite3_free(link_sql);
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
            for (const auto& filename : savedFilenames) {
                string path = "../images/" + filename;
                remove(path.c_str());
            }
            
            char* delete_photos_sql = sqlite3_mprintf("DELETE FROM cat_photos WHERE cat_id = %d", cat_id);
            db.Sql_exec(delete_photos_sql);
            sqlite3_free(delete_photos_sql);
            
            char* delete_cat_sql = sqlite3_mprintf("DELETE FROM cats WHERE id = %d", cat_id);
            db.Sql_exec(delete_cat_sql);
            sqlite3_free(delete_cat_sql);
            
            Json::Value bad_answer;
            bad_answer["status"] = "bad";
            bad_answer["message"] = "Medical fields must be multiple of 4";
            bad_answer["received_fields"] = (int)medical_fields.size();
            bad_answer["received_data"] = cat_medical;            
            auto response = HttpResponse::newHttpJsonResponse(bad_answer);
            response->setStatusCode(k400BadRequest);
            response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
            response->addHeader("Access-Control-Allow-Credentials", "true");
            callback(response);
            return;
        }
        
        for (size_t i = 0; i + 3 < medical_fields.size(); i += 4) {
            string icon = medical_fields[i];
            string label = medical_fields[i + 1];
            string color = medical_fields[i + 2];
            string bg = medical_fields[i + 3];
            
            char* med_sql = sqlite3_mprintf("INSERT INTO medical (cat_id, icon, label, color, bg) VALUES (%d, %Q, %Q, %Q, %Q)",
                cat_id, icon.c_str(), label.c_str(), color.c_str(), bg.c_str());
            
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
    resp["name"] = cat_name;
    resp["description"] = cat_description;
    resp["breed"] = cat_breed;
    resp["age"] = cat_age;
    resp["filenames"] = Json::arrayValue;
    for (const auto& filename : savedFilenames) {
        resp["filenames"].append(filename);
    }
    resp["tags"] = tagsArray;
    resp["medical"] = medArray;
    
    auto response = HttpResponse::newHttpJsonResponse(resp);
    response->setStatusCode(k201Created);
    response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
    response->addHeader("Access-Control-Allow-Credentials", "true");
    callback(response);
}

void Handler::updateCat(const HttpRequestPtr& request, function<void(const HttpResponsePtr&)>&& callback) {
    cout << request->getMethodString() << " " << request->getPath() << endl;

    string url = request->getPath();
    size_t poz = url.find_last_of('/');
    string id = url.substr(poz+1);
    int cat_id = -1;
    try {
        cat_id = stoi(id);
    } catch(const std::exception& e) {
        Json::Value resp;
        resp["id"] = id;
        resp["path"] = url;
        resp["error"] = "not a number";
        auto response = HttpResponse::newHttpJsonResponse(resp);
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
        response->addHeader("Access-Control-Allow-Credentials", "true");
        callback(response);
        return;
    }

    string sessionId = request->getCookie("session_id");
    int user_id = checkAuth(sessionId);
    
    if (user_id == -1) {
        Json::Value resp;
        resp["status"] = "bad";
        resp["message"] = "Unauthorized";
        auto response = HttpResponse::newHttpJsonResponse(resp);
        response->setStatusCode(k401Unauthorized);
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
        response->addHeader("Access-Control-Allow-Credentials", "true");
        callback(response);
        return;
    }
    
    if (!CheckPermissions(user_id)) {
        Json::Value resp;
        resp["status"] = "bad";
        resp["message"] = "Access Denied";
        auto response = HttpResponse::newHttpJsonResponse(resp);
        response->setStatusCode(k403Forbidden);
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
        response->addHeader("Access-Control-Allow-Credentials", "true");
        callback(response);
        return;
    }
    
    char* get_cat_sql = sqlite3_mprintf("SELECT name, description, breed, age FROM cats WHERE id = %d", cat_id);
    vector<vector<string>> cat_data = db.Sql_request_vector(get_cat_sql);
    sqlite3_free(get_cat_sql);
    
    if (cat_data.empty()) {
        Json::Value resp;
        resp["status"] = "bad";
        resp["message"] = "Cat not found";
        auto response = HttpResponse::newHttpJsonResponse(resp);
        response->setStatusCode(k404NotFound);
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
        response->addHeader("Access-Control-Allow-Credentials", "true");
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
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
        response->addHeader("Access-Control-Allow-Credentials", "true");
        callback(response);
        return;
    }
    
    Json::Value response_data;
    response_data["status"] = "ok";
    response_data["cat_id"] = cat_id;
    
    string new_name = json->isMember("name") ? (*json)["name"].asString() : cat_data[0][0];
    string new_description = json->isMember("description") ? (*json)["description"].asString() : cat_data[0][1];
    string new_breed = json->isMember("breed") ? (*json)["breed"].asString() : cat_data[0][2];
    string new_age = json->isMember("age") ? (*json)["age"].asString() : cat_data[0][3];
    
    char* update_cat_sql = sqlite3_mprintf(
        "UPDATE cats SET name = %Q, description = %Q, breed = %Q, age = %Q WHERE id = %d",
        new_name.c_str(), new_description.c_str(), new_breed.c_str(), new_age.c_str(), cat_id
    );
    db.Sql_exec(update_cat_sql);
    sqlite3_free(update_cat_sql);
    
    response_data["updated_cat"] = Json::objectValue;
    response_data["updated_cat"]["name"] = new_name;
    response_data["updated_cat"]["description"] = new_description;
    response_data["updated_cat"]["breed"] = new_breed;
    response_data["updated_cat"]["age"] = new_age;
    
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
                    char* link_sql = sqlite3_mprintf("INSERT INTO cat_tags (cat_id, tag_id) VALUES (%d, %d)", cat_id, tag_id);
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
                response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
                response->addHeader("Access-Control-Allow-Credentials", "true");
                callback(response);
                return;
            }
            
            for (size_t i = 0; i + 3 < medical_fields.size(); i += 4) {
                string icon = medical_fields[i];
                string label = medical_fields[i + 1];
                string color = medical_fields[i + 2];
                string bg = medical_fields[i + 3];
                
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
    
    auto response = HttpResponse::newHttpJsonResponse(response_data);
    response->setStatusCode(k200OK);
    response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
    response->addHeader("Access-Control-Allow-Credentials", "true");
    callback(response);
}

void Handler::AddToBookings(const HttpRequestPtr& request, function<void(const HttpResponsePtr&)>&& callback){
    cout << request->getMethodString() << " " << request->getPath() << endl;
    string sessionId = request->getCookie("session_id");
    int user_id = checkAuth(sessionId);
    
    auto json = request->getJsonObject();
    if(!json){
        Json::Value bad_answer;
        bad_answer["status"]="bad";
        bad_answer["message"]="Bad Json";
        auto response = HttpResponse::newHttpJsonResponse(bad_answer);
        response->setStatusCode(k400BadRequest);
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
        response->addHeader("Access-Control-Allow-Credentials", "true");
        callback(response);
        return;
    }

    if (!json->isMember("start") || !json->isMember("end") || !json->isMember("id")) {
        Json::Value resp;
        resp["status"] = "bad";
        resp["message"] = "Missing required fields: start or end or id";
        auto response = HttpResponse::newHttpJsonResponse(resp);
        response->setStatusCode(k400BadRequest);
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
        response->addHeader("Access-Control-Allow-Credentials", "true");
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
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
        response->addHeader("Access-Control-Allow-Credentials", "true");
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
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
        response->addHeader("Access-Control-Allow-Credentials", "true");
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
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
        response->addHeader("Access-Control-Allow-Credentials", "true");
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
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
        response->addHeader("Access-Control-Allow-Credentials", "true");
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
    response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
    response->addHeader("Access-Control-Allow-Credentials", "true");
    callback(response);
}

void Handler::GetAdminBookings(const HttpRequestPtr& request, function<void(const HttpResponsePtr&)>&& callback) {
    cout << request->getMethodString() << " " << request->getPath() << endl;
    
    string sessionId = request->getCookie("session_id");
    int user_id = checkAuth(sessionId);
    
    if (user_id == -1) {
        Json::Value resp;
        resp["status"] = "bad";
        resp["message"] = "Unauthorized";
        auto response = HttpResponse::newHttpJsonResponse(resp);
        response->setStatusCode(k401Unauthorized);
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
        response->addHeader("Access-Control-Allow-Credentials", "true");
        callback(response);
        return;
    }
    
    if (!CheckPermissions(user_id)) {
        Json::Value resp;
        resp["status"] = "bad";
        resp["message"] = "Access Denied";
        auto response = HttpResponse::newHttpJsonResponse(resp);
        response->setStatusCode(k403Forbidden);
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
        response->addHeader("Access-Control-Allow-Credentials", "true");
        callback(response);
        return;
    }
    
    Json::Value bookings(Json::arrayValue);
    
    char* sql = sqlite3_mprintf(
    "SELECT b.id, b.cat_id, c.name as cat_name, b.user_id, u.nickname, "
    "b.start_time, b.end_time, b.status "
    "FROM bookings b "
    "JOIN cats c ON b.cat_id = c.id "
    "JOIN users u ON b.user_id = u.id "
    "ORDER BY b.status, b.start_time"
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
        booking["status"] = stoi(output[7]);
        bookings.append(booking);
    });
    
    sqlite3_free(sql);
    
    Json::Value resp;
    resp["status"] = "ok";
    resp["bookings"] = bookings;
    
    auto response = HttpResponse::newHttpJsonResponse(resp);
    response->setStatusCode(k200OK);
    response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
    response->addHeader("Access-Control-Allow-Credentials", "true");
    callback(response);
}

void Handler::ConfirmAdminBookings(const HttpRequestPtr& request, function<void(const HttpResponsePtr&)>&& callback){
    cout << request->getMethodString() << " " << request->getPath() << endl;
    string sessionId = request->getCookie("session_id");
    int user_id = checkAuth(sessionId);
    
    if (user_id == -1) {
        Json::Value resp;
        resp["status"] = "bad";
        resp["message"] = "Unauthorized";
        auto response = HttpResponse::newHttpJsonResponse(resp);
        response->setStatusCode(k401Unauthorized);
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
        response->addHeader("Access-Control-Allow-Credentials", "true");
        callback(response);
        return;
    }
    
    if (!CheckPermissions(user_id)) {
        Json::Value resp;
        resp["status"] = "bad";
        resp["message"] = "Access Denied";
        auto response = HttpResponse::newHttpJsonResponse(resp);
        response->setStatusCode(k403Forbidden);
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
        response->addHeader("Access-Control-Allow-Credentials", "true");
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
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
        response->addHeader("Access-Control-Allow-Credentials", "true");
        callback(response);
        return;
    }

    if (!json->isMember("booking_id")) {
        Json::Value resp;
        resp["status"] = "bad";
        resp["message"] = "Missing booking_id field";
        auto response = HttpResponse::newHttpJsonResponse(resp);
        response->setStatusCode(k400BadRequest);
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
        response->addHeader("Access-Control-Allow-Credentials", "true");
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
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
        response->addHeader("Access-Control-Allow-Credentials", "true");
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
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
        response->addHeader("Access-Control-Allow-Credentials", "true");
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
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
        response->addHeader("Access-Control-Allow-Credentials", "true");
        callback(response);
        return;
    }

    Json::Value resp;
    resp["status"] = "ok";
    resp["message"] = "Booking confirmed successfully";
    auto response = HttpResponse::newHttpJsonResponse(resp);
    response->setStatusCode(k200OK);
    response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
    response->addHeader("Access-Control-Allow-Credentials", "true");
    callback(response);
}

void Handler::RejectAdminBooking(const HttpRequestPtr& request, function<void(const HttpResponsePtr&)>&& callback){
    cout << request->getMethodString() << " " << request->getPath() << endl;
    
    string sessionId = request->getCookie("session_id");
    int user_id = checkAuth(sessionId);
    
    if (user_id == -1) {
        Json::Value resp;
        resp["status"] = "bad";
        resp["message"] = "Unauthorized";
        auto response = HttpResponse::newHttpJsonResponse(resp);
        response->setStatusCode(k401Unauthorized);
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
        response->addHeader("Access-Control-Allow-Credentials", "true");
        callback(response);
        return;
    }
    
    if (!CheckPermissions(user_id)) {
        Json::Value resp;
        resp["status"] = "bad";
        resp["message"] = "Access Denied";
        auto response = HttpResponse::newHttpJsonResponse(resp);
        response->setStatusCode(k403Forbidden);
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
        response->addHeader("Access-Control-Allow-Credentials", "true");
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
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
        response->addHeader("Access-Control-Allow-Credentials", "true");
        callback(response);
        return;
    }

    if (!json->isMember("booking_id")) {
        Json::Value resp;
        resp["status"] = "bad";
        resp["message"] = "Missing booking_id field";
        auto response = HttpResponse::newHttpJsonResponse(resp);
        response->setStatusCode(k400BadRequest);
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
        response->addHeader("Access-Control-Allow-Credentials", "true");
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
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
        response->addHeader("Access-Control-Allow-Credentials", "true");
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
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
        response->addHeader("Access-Control-Allow-Credentials", "true");
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
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
        response->addHeader("Access-Control-Allow-Credentials", "true");
        callback(response);
        return;
    }

    Json::Value resp;
    resp["status"] = "ok";
    resp["message"] = "Booking rejected and deleted successfully";
    auto response = HttpResponse::newHttpJsonResponse(resp);
    response->setStatusCode(k200OK);
    response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
    response->addHeader("Access-Control-Allow-Credentials", "true");
    callback(response);
}

void Handler::AddAdminBooking(const HttpRequestPtr& request, function<void(const HttpResponsePtr&)>&& callback){
    cout << request->getMethodString() << " " << request->getPath() << endl;
    
    string sessionId = request->getCookie("session_id");
    int user_id_adm = checkAuth(sessionId);
    
    if (user_id_adm == -1){
        Json::Value resp;
        resp["status"] = "bad";
        resp["message"] = "Unauthorized";
        auto response = HttpResponse::newHttpJsonResponse(resp);
        response->setStatusCode(k401Unauthorized);
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
        response->addHeader("Access-Control-Allow-Credentials", "true");
        callback(response);
        return;
    }
    
    if (!CheckPermissions(user_id_adm)){
        Json::Value resp;
        resp["status"] = "bad";
        resp["message"] = "Access Denied";
        auto response = HttpResponse::newHttpJsonResponse(resp);
        response->setStatusCode(k403Forbidden);
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
        response->addHeader("Access-Control-Allow-Credentials", "true");
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
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
        response->addHeader("Access-Control-Allow-Credentials", "true");
        callback(response);
        return;
    }
    
    if (!json->isMember("email") || !json->isMember("cat_id") || !json->isMember("start_time") || !json->isMember("end_time")){
        Json::Value resp;
        resp["status"] = "bad";
        resp["message"] = "Missing email or cat_id or start_time or end_time field";
        auto response = HttpResponse::newHttpJsonResponse(resp);
        response->setStatusCode(k400BadRequest);
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
        response->addHeader("Access-Control-Allow-Credentials", "true");
        callback(response);
        return;
    }
    
    string email = (*json)["email"].asString();
    int cat_id = (*json)["cat_id"].asInt();
    string start_time = (*json)["start_time"].asString();
    string end_time = (*json)["end_time"].asString(); 

    int user_id = IdByLogin(email);
    if(user_id ==-1){
        Json::Value resp;
        resp["status"] = "bad";
        resp["message"] = "User not found";
        auto response = HttpResponse::newHttpJsonResponse(resp);
        response->setStatusCode(k404NotFound);
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
        response->addHeader("Access-Control-Allow-Credentials", "true");
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
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
        response->addHeader("Access-Control-Allow-Credentials", "true");
        callback(response);
        return;
    }

    char* check_booking_sql = sqlite3_mprintf(
        "SELECT id FROM bookings WHERE cat_id = %d AND status = 1 AND "
        "((start_time <= %Q AND end_time > %Q) OR "
        "(start_time < %Q AND end_time >= %Q) OR "
        "(start_time >= %Q AND end_time <= %Q))",
        cat_id, 
        start_time.c_str(), start_time.c_str(),
        end_time.c_str(), end_time.c_str(),
        start_time.c_str(), end_time.c_str()
    );
    
    vector<vector<string>> booking_check = db.Sql_request_vector(check_booking_sql);
    sqlite3_free(check_booking_sql);
    
    if (!booking_check.empty()) {
        Json::Value resp;
        resp["status"] = "bad";
        resp["message"] = "Cat is already booked for this time period";
        auto response = HttpResponse::newHttpJsonResponse(resp);
        response->setStatusCode(k409Conflict);
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
        response->addHeader("Access-Control-Allow-Credentials", "true");
        callback(response);
        return;
    }




    
    char* sql = sqlite3_mprintf("INSERT INTO bookings (cat_id, user_id, start_time, end_time, status) VALUES (%d, %d, %Q, %Q, 1)",cat_id, user_id,start_time,end_time);
    if(!db.Sql_exec(sql)){
        sqlite3_free(sql);
        Json::Value resp;
        resp["status"] = "bad";
        resp["message"] = "Failed to create booking";
        auto response = HttpResponse::newHttpJsonResponse(resp);
        response->setStatusCode(k500InternalServerError);
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
        response->addHeader("Access-Control-Allow-Credentials", "true");
        callback(response);
        return;
    }
    sqlite3_free(sql);

    int booking_id = sqlite3_last_insert_rowid(db.GetDataBaseLink());

    Json::Value resp;

    resp["status"] = "ok";
    resp["message"] = "Booking created successfully by admin";
    resp["booking"]["id"] = booking_id;
    resp["booking"]["cat_id"] = cat_id;
    resp["booking"]["user_id"] = user_id;
    resp["booking"]["start_time"] = start_time;
    resp["booking"]["end_time"] = end_time;
    resp["booking"]["status"] = 1;

    auto response = HttpResponse::newHttpJsonResponse(resp);
    response->setStatusCode(k201Created);
    response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
    response->addHeader("Access-Control-Allow-Credentials", "true");
    callback(response);
}

void Handler::GetUserData(const HttpRequestPtr& request, function<void(const HttpResponsePtr&)>&& callback){
    cout << request->getMethodString() << " " << request->getPath() << endl;

    string sessionId = request->getCookie("session_id");
    int user_id = checkAuth(sessionId);
    
    if (user_id == -1) {
        Json::Value resp;
        resp["status"] = "bad";
        resp["message"] = "Unauthorized";
        auto response = HttpResponse::newHttpJsonResponse(resp);
        response->setStatusCode(k401Unauthorized);
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
        response->addHeader("Access-Control-Allow-Credentials", "true");
        callback(response);
        return;
    }

    char* sql = sqlite3_mprintf("SELECT id, nickname, access_level FROM users WHERE id=%d", user_id);
    vector<vector<string>> user_data_vector = db.Sql_request_vector(sql);
    sqlite3_free(sql);
    
    if(user_data_vector.empty()){
        Json::Value resp;
        resp["status"] = "bad";
        resp["message"] = "no user data";
        auto response = HttpResponse::newHttpJsonResponse(resp);
        response->setStatusCode(k404NotFound);
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
        response->addHeader("Access-Control-Allow-Credentials", "true");
        callback(response);
        return;
    }

    Json::Value user_data;
    user_data["user_id"] = stoi(user_data_vector[0][0]);
    user_data["nickname"] = user_data_vector[0][1];
    user_data["status"]=stoi(user_data_vector[0][2]);

    sql = sqlite3_mprintf(
        "SELECT b.cat_id, b.user_id, b.start_time, b.end_time, b.status, "
        "GROUP_CONCAT(cp.filename) as photos "
        "FROM bookings b "
        "LEFT JOIN cat_photos cp ON b.cat_id = cp.cat_id "
        "WHERE b.user_id = %d "
        "GROUP BY b.id",
        user_id
    );
    
    Json::Value bookings(Json::arrayValue);
    db.Sql_request_callback(sql, [&bookings](vector<string> row){
        Json::Value booking;
        booking["cat_id"] = stoi(row[0]);
        booking["user_id"] = stoi(row[1]);
        booking["start_time"] = row[2];
        booking["end_time"] = row[3];
        booking["status"] = stoi(row[4]);
        
        Json::Value filenames(Json::arrayValue);
        if (!row[5].empty()) {
            string photos_string = row[5];
            size_t pos = 0;
            while ((pos = photos_string.find(',')) != string::npos) {
                filenames.append(photos_string.substr(0, pos));
                photos_string.erase(0, pos + 1);
            }
            filenames.append(photos_string);
        }
        
        booking["filename"] = filenames;
        bookings.append(booking);
    });
    
    sqlite3_free(sql);
    user_data["bookings"] = bookings;

    Json::Value resp;
    resp["status"] = "ok";
    resp["user"] = user_data;

    auto response = HttpResponse::newHttpJsonResponse(resp);
    response->setStatusCode(k200OK);
    response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
    response->addHeader("Access-Control-Allow-Credentials", "true");
    callback(response);
}

void Handler::LogOut(const HttpRequestPtr& request, function<void(const HttpResponsePtr&)>&& callback) {
    cout << request->getMethodString() << " " << request->getPath() << endl;

    string sessionId = request->getCookie("session_id");
    
    Json::Value resp;
    
    if (sessionId.empty()) {
        resp["status"] = "ok";
        resp["message"] = "Already logged out";
        auto response = HttpResponse::newHttpJsonResponse(resp);
        response->setStatusCode(k200OK);
        
        string clearCookie = "session_id=; Max-Age=0; Path=/; HttpOnly; SameSite=None; Secure";
        response->addHeader("Set-Cookie", clearCookie);
        response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
        response->addHeader("Access-Control-Allow-Credentials", "true");
        
        callback(response);
        return;
    }

    char* sql = sqlite3_mprintf("DELETE FROM sessions WHERE session_id = %Q", sessionId.c_str());
    bool success = db.Sql_exec(sql);
    sqlite3_free(sql);

    if (success) {
        resp["status"] = "ok";
        resp["message"] = "Logged out successfully";
    } else {
        resp["status"] = "bad";
        resp["message"] = "Failed to logout";
    }

    auto response = HttpResponse::newHttpJsonResponse(resp);
    
    string clearCookie = "session_id=; Max-Age=0; Path=/; HttpOnly; SameSite=None; Secure";
    response->addHeader("Set-Cookie", clearCookie);
    
    response->setStatusCode(success ? k200OK : k500InternalServerError);
    response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
    response->addHeader("Access-Control-Allow-Credentials", "true");
    callback(response);
}

