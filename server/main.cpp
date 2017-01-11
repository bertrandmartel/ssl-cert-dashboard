/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Bertrand Martel
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
/**
    main.cpp

    launch Ssl dashboard server

    @author Bertrand Martel
    @version 1.0
*/
#include <QCoreApplication>
#include <iostream>
#include <QStringList>
#include <QDebug>
#include "string"
#include <sys/param.h>
#include "SslHandler.h"
#include "httpserverimpl/httpserver.h"
#include "ClientSocketHandler.h"
#include "HashDigestListener.h"
#include "DigestManager.h"
#include "database.h"
#include "utils/fileutils.h"
#include "qjson/parser.h"

using namespace std;


struct CleanExit{

    CleanExit() {

        signal(SIGINT, &CleanExit::exitQt);
        signal(SIGTERM, &CleanExit::exitQt);
    }

    static void exitQt(int sig) {

        QCoreApplication::exit(0);

    }
};

int main(int argc,char *argv[]){

    CleanExit cleanExit;
    QCoreApplication a(argc,argv);

    int port = 4343;
    string ip = "127.0.0.1";
    bool useSSL = false;
    string publicCert = "";
    string privateCert = "";
    string caCerts = "";
    string privateKeyPass="";
    string digestRealm = "bertrandmartel_realm";
    string webpath="~/ssl-dashboard/web";
    string keyFile="~/ssl-dashboard/rand.txt";
    string algoStr = "MD5";
    digest_algorithm digestAlgo = ALGO_MD5;

    //ignore SIGPIPE signal (broken pipe issue)
    signal(SIGPIPE, SIG_IGN);

    QStringList args = a.arguments();

    string config ="";

    if (args.size() >1){

        config=args[1].toStdString();

        QByteArray fileData = fileutils::readFromFile((char*)config.data());

        if (fileData.size()>0){

            QJson::Parser parser;
            bool ok;

            // json is a QString containing the data to convert
            QVariantMap result = parser.parse(fileData.data(), &ok).toMap();

            if (ok){
                port = result["port"].toInt();
                ip = result["ip"].toString().toStdString();
                useSSL = result["useSSL"].toBool();
                publicCert=result["publicCert"].toString().toStdString();
                privateCert=result["privateCert"].toString().toStdString();
                caCerts=result["caCerts"].toString().toStdString();
                privateKeyPass=result["privateKeyPass"].toString().toStdString();
                digestRealm=result["digestRealm"].toString().toStdString();
                webpath=result["webPath"].toString().toStdString();
                keyFile=result["keyFile"].toString().toStdString();

                algoStr = result["digestAlgo"].toString().toStdString();

                if (strcmp(algoStr.data(),"MD5")==0)
                    digestAlgo=ALGO_MD5;
                else if (strcmp(algoStr.data(),"SHA1")==0)
                    digestAlgo=ALGO_SHA1;
                else
                    cerr << "Error algo must be MD5 or SHA1 (default MD5)" << endl;
            }
            else{
                cerr << "Json configuration is inconsistent" << endl;
            }
        }
    }

    cout << "--------------------------------" << endl;
    cout << "Current configuration :" << endl;
    cout << "Port              : " << port << endl;
    cout << "IP                : " << ip.data() << endl;
    cout << "use ssl           : " << useSSL << endl;
    cout << "public cert file  : " << publicCert.data() << endl;
    cout << "private cert file : " << privateCert.data() << endl;
    cout << "ca certs file     : " << caCerts.data() << endl;
    cout << "private key pass  : " << privateKeyPass.data() << endl;
    cout << "digest realm      : " << digestRealm.data() << endl;
    cout << "web path          : " << webpath.data() << endl;
    cout << "key file          : " << keyFile.data() << endl;
    cout << "digest algorithm  : " << algoStr.data() << endl;
    cout << "--------------------------------" << endl;

    Database database;
    Database* db_ptr=&database;
    database.set_debug(true);
    database.setDigestParams(digestRealm,digestAlgo);
    database.setKey(fileutils::readFromFile((char*)keyFile.data()));
    database.init_user();

    HashDigestListener digest_listener(db_ptr);
    DigestManager digest_manager;
    digest_manager.set_digest_algorithm(digestAlgo);
    digest_manager.set_session_type(SESSION_COOKIE);

    digest_manager.set_digest_listener(&digest_listener);

    ClientSocketHandler clientHandler(db_ptr,&digest_manager,webpath,digestRealm);

    //instance of HTTP server
    HttpServer server;
    server.set_debug(false);

    if (useSSL){

        //set secured HTTP server
        server.setSSL(true);

        cout << "setting server certs ..." << endl;

        //set public / private and certification authority list into http server object
        server.setPublicCert(SslHandler::retrieveCertFromFile((char*)publicCert.data()));
        server.setPrivateCert(SslHandler::retrieveKeyCertFile((char*)privateCert.data(),(char*)privateKeyPass.data()));
        server.setCaCert(SslHandler::retrieveveCaCertListFromFile((char*)caCerts.data()));
    }

    server.addClientEventListener(&clientHandler);

    if (!server.listen(QHostAddress(ip.data()),port)) {
        qDebug() << "An error occured while initializing hope proxy server... Maybe another instance is already running on "<< ip.data() << ":" << port << endl;
        return -1;
    }

    cout << "Starting HTTP server on "<< ip.data()  << ":" << port << endl;

    return a.exec();
}
