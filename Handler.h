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

        static int getUserIdFromSession(const string& sessionId);

        static int checkAuth(string session_id);

        static bool CheckPermissions(int user_id);

        static string GenerateFileName(string extension);

    public:

        static void RegisterUser(const HttpRequestPtr& request, function<void(const HttpResponsePtr&)>&& callback);
        
        static void AutoriseUser(const HttpRequestPtr& req, function<void(const HttpResponsePtr&)>&& callback);

        static void GetCats(const HttpRequestPtr& req, function<void(const HttpResponsePtr&)>&& callback);

        static void handleOptions(const HttpRequestPtr& req, function<void(const HttpResponsePtr&)>&& callback);

        static void uploadCatPhoto(const HttpRequestPtr& req, function<void(const HttpResponsePtr&)>&& callback);

        static void AddToBookings(const HttpRequestPtr& request, function<void(const HttpResponsePtr&)>&& callback);

        static void updateCatTagsAndMedical(const HttpRequestPtr& request, function<void(const HttpResponsePtr&)>&& callback);
};
