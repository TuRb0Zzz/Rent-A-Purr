#pragma once
#include <drogon/drogon.h>
#include "DataBase.h"

#include <openssl/sha.h>
#include <iomanip>
#include <sstream>

#include <json/json.h>
using namespace std;
using namespace drogon;

class Handler{
    private:
        static string hashPassword(const string& password);

        static bool verifyPassword(const string& plainPassword, const string& hashedPassword);

        static string createSession(long long id);

        static string generateSessionId();

        int getUserIdFromSession(const string& sessionId);

        int checkAuth(string session_id);

    public:



        static void RegisterUser(const HttpRequestPtr& request,std::function<void(const HttpResponsePtr&)>&& callback);
        
        //static void AutoriseUser(const HttpRequestPtr& req,function<void(const HttpResponsePtr&)> callback);
};