#include <drogon/drogon.h>
#include "Handler.h"
#include "DataBase.h"

using namespace drogon;
using namespace std;

DataBase db;





int main() {

    app().setDocumentRoot("../images");
    app().setStaticFileHeaders({{"Cache-Control", "public, max-age=86400"}});


    app().registerHandler("/register", &Handler::RegisterUser, {Post});
    app().registerHandler("/register", &Handler::handleOptions, {Options});

    app().registerHandler("/login", &Handler::AutoriseUser, {Get});
    app().registerHandler("/login", &Handler::handleOptions, {Options});

    app().registerHandler("/cats", &Handler::GetCats, {Get});
    app().registerHandler("/cats", &Handler::handleOptions, {Options});
    app().registerHandler("/cats", &Handler::uploadCatPhoto, {Post});

    cout<<"server is running"<<endl;
    app().addListener("0.0.0.0", 8000).run();

    return 0;
}