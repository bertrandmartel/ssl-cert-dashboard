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
    ClientSocketHandler.cpp
    Client socket managing event handler

    @author Bertrand Martel
    @version 1.0
*/
#include "ClientSocketHandler.h"

#include "httpserverinter/IHttpClient.h"
#include "DigestManager.h"
#include "iostream"
#include "string.h"
#include "protocol/inter/http/IhttpFrame.h"
#include "sstream"
#include "QFile"
#include "QTextStream"
#include "vector"
#include "qjson/serializer.h"
#include "qjson/parser.h"
#include "sslgen.h"
#include "utils.h"

#define HTTP_METHOD_GET  "GET"
#define HTTP_METHOD_POST "POST"

using namespace std;

/**
* @brief ClientSocketHandler
*      build client socket handler
*/
ClientSocketHandler::ClientSocketHandler(Database * database,DigestManager* digest_manager,std::string web_path,std::string realm)
{
    this->realm=realm;
    this->database=database;
    this->digest_manager=digest_manager;
    this->web_path=web_path;
    unauthorized_page = QString("<HTML>\r\n").toStdString()
            + "<HEAD>\r\n"
            + "<TITLE>Error</TITLE>\r\n"
            + " <META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; charset=ISO-8859-1\">\r\n"
            + "</HEAD>\r\n"
            + "<BODY><H1>401 Unauthorized</H1></BODY>\r\n"
            + "</HTML>\r\n";

    internal_error_page = QString("<HTML>\r\n").toStdString()
            + "<HEAD>\r\n"
            + "<TITLE>Error</TITLE>\r\n"
            + " <META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; charset=ISO-8859-1\">\r\n"
            + "</HEAD>\r\n"
            + "<BODY><H1>500 Internal Server Error</H1></BODY>\r\n"
            + "</HTML>\r\n";

    not_found_page = QString("<HTML>\r\n").toStdString()
            + "<HEAD>\r\n"
            + "<TITLE>Error</TITLE>\r\n"
            + " <META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; charset=ISO-8859-1\">\r\n"
            + "</HEAD>\r\n"
            + "<BODY><H1>404 Not Found</H1></BODY>\r\n"
            + "</HTML>\r\n";

    ok_page = QString("<HTML>\r\n").toStdString()
            + "<HEAD>\r\n"
            + "<TITLE>Error</TITLE>\r\n"
            + " <META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; charset=ISO-8859-1\">\r\n"
            + "</HEAD>\r\n"
            + "<BODY><H1>200 Ok</H1></BODY>\r\n"
            + "</HTML>\r\n";
}

ClientSocketHandler::~ClientSocketHandler()
{
}

/**
 * called when an http response has been received from client
 *
 * @param client
 * 		client object
 * @param message
 * 		message delivered
 */
void ClientSocketHandler::onHttpResponseReceived(IHttpClient &client,Ihttpframe * frame,std::string peer_address){
}

/**
 * called when an http response has been received from client
 *
 * @param client
 * 		client object
 * @param message
 * 		message delivered
 */
void ClientSocketHandler::onHttpRequestReceived(IHttpClient &client,Ihttpframe * frame,std::string peer_address){

    QString api_str(frame->getUri().c_str());

    cout << frame->getUri().c_str() << endl;


    if (strcmp(frame->getUri().c_str(),"/login")==0){

        DigestInfo digest_info = digest_manager->process_digest(frame->getMethod(),frame->getUri(),frame->getHeaders(),realm);

        cout << "LOGIN Requesting uri : " << frame->getUri().data() << " - current code status : " << digest_info.get_status_code() << endl;

        if (digest_info.get_status_code()==401){

            string file_name = web_path+"/login.html";
            send_text_page(api_str,digest_info.get_headers(),file_name,client);
        }
        else if (digest_info.get_status_code()==200){

            string file_name = web_path+"/dashboard.html";
            send_text_page(api_str,digest_info.get_headers(),file_name,client);
        }
    }
    else if (strcmp("/logout",frame->getUri().data())==0){

        digest_manager->remove_session_for_headers(frame->getHeaders());

        client.sendHttpMessage(get_page_content("200 OK",ok_page,0));
    }
    else if (is_public_page(frame->getUri())){

        //ok free to process
        string file_name = web_path + api_str.toStdString();

        if (api_str.endsWith(".html") || api_str.endsWith(".css") || api_str.endsWith(".js"))
            send_text_page(api_str,0,file_name,client);
        else
            send_image_page(api_str,0,file_name,client);
    }
    else {

        DigestInfo digest_info = digest_manager->process_digest(frame->getMethod(),frame->getUri(),frame->getHeaders(),realm);

        cout << "Requesting uri : " << frame->getUri().data() << " - current code status : " << digest_info.get_status_code() << endl;

        //must be authorized
        if (strcmp(frame->getUri().c_str(),"/registration")==0 && digest_info.get_status_code()==401){

            cout << "Sending response with status code 401 Unauthorized" << endl;
            client.sendHttpMessage(get_page_content("401 Unauthorized",unauthorized_page,digest_info.get_headers()));
        }
        else if (digest_info.get_status_code()==401){
            string file_name = web_path+"/login.html";
            send_text_page(api_str,digest_info.get_headers(),file_name,client);
        }
        else if (digest_info.get_status_code()==500){

            cout << "Sending response with status code 500 Internal Server Error" << endl;
            client.sendHttpMessage(get_page_content("500 Internal Server Error",internal_error_page,digest_info.get_headers()));
        }
        else if (digest_info.get_status_code()==200){

            if (strcmp(frame->getUri().c_str(),"/registration")==0){

                cout << "Registration success" << endl;
                client.sendHttpMessage(get_page_content("200 OK",ok_page,digest_info.get_headers()));
            }
            else if (strcmp(frame->getUri().c_str(),"/dashboard")==0){

                string file_name = web_path+"/dashboard.html";
                send_text_page(api_str,digest_info.get_headers(),file_name,client);
            }
            else if (strcmp(frame->getUri().c_str(),"/userboard")==0){

                string file_name = web_path+"/users.html";
                send_text_page(api_str,digest_info.get_headers(),file_name,client);
            }
            else if (api_str.endsWith(".css") || api_str.endsWith(".js") || api_str.endsWith(".png") || api_str.endsWith(".jpg")){

                string file_name = web_path + api_str.toStdString();

                QFile file(file_name.data());

                if (!file.open(QIODevice::ReadOnly)) {

                    cerr << "Cannot open file for reading: "   << file_name.data() << endl;
                    client.sendHttpMessage(get_page_content("500 Internal Server Error",internal_error_page,digest_info.get_headers()));

                } else {

                    if (api_str.endsWith(".html") || api_str.endsWith(".css") || api_str.endsWith(".js")){
                        send_text_page(api_str,digest_info.get_headers(),file_name,client);
                    }
                    else if (api_str.endsWith(".png") || api_str.endsWith("jpg")){
                        send_image_page(api_str,digest_info.get_headers(),file_name,client);
                    }

                }
            }
            else if (strcmp(frame->getUri().c_str(),"/cert")==0){

                // create a Parser instance
                QJson::Parser parser;

                bool ok;

                // json is a QString containing the data to convert
                QVariantMap result = parser.parse(frame->getBody().data(), &ok).toMap();

                if (ok){

                    if (result["serial"].toInt()>=0){

                        cout << "Retrieving cert with serial " << result["serial"].toInt() << endl;

                        QVariantList sslCert = database->getGeneratedCertListBySerial(result["serial"].toInt());

                        QVariantMap response;
                        response.insert("data", sslCert);

                        QJson::Serializer serializer;

                        bool ok;
                        QByteArray json = serializer.serialize(response, &ok);

                        if (ok) {

                            stringstream contentLength;
                            contentLength << json.length();

                            string response = QString("HTTP/1.1 200 OK\r\n").toStdString() +
                                    "Content-Type: application/json\r\nContent-Length: " + contentLength.str() + "\r\n";

                            response+=QString("\r\n").toStdString() + json.data();

                            client.sendHttpMessage(response);

                            return;
                        }
                    }
                }
                client.sendHttpMessage(get_page_content("500 Internal Server Error",internal_error_page,digest_info.get_headers()));
            }
            else if (strcmp(frame->getUri().c_str(),"/users")==0){

                if (strcmp(frame->getMethod().c_str(),"DELETE")==0){

                    QJson::Parser parser;

                    bool ok;

                    // json is a QString containing the data to convert
                    QVariantMap result = parser.parse(frame->getBody().data(), &ok).toMap();

                    if (ok){

                        QString username = result["username"].toString();

                        cout << "delete user " << username.toStdString().data() << endl;

                        bool success=database->deleteUser(username.toStdString());

                        if (success){
                            client.sendHttpMessage(get_page_content("200 OK",ok_page,digest_info.get_headers()));
                            return;
                        }
                    }
                    client.sendHttpMessage(get_page_content("500 Internal Server Error",internal_error_page,digest_info.get_headers()));
                }
                else if (strcmp(frame->getMethod().c_str(),"GET")==0){

                    // create a Parser instance
                    QJson::Parser parser;

                    bool ok;

                    // json is a QString containing the data to convert
                    QVariantMap result = parser.parse(frame->getBody().data(), &ok).toMap();

                    if (ok){

                        cout << "Retrieving users" << endl;

                        QVariantList userList = database->get_user_list();

                        QVariantMap response;
                        response.insert("data", userList);

                        QJson::Serializer serializer;

                        bool ok;
                        QByteArray json = serializer.serialize(response, &ok);

                        if (ok) {

                            stringstream contentLength;
                            contentLength << json.length();

                            string response = QString("HTTP/1.1 200 OK\r\n").toStdString() +
                                    "Content-Type: application/json\r\nContent-Length: " + contentLength.str() + "\r\n";

                            response+=QString("\r\n").toStdString() + json.data();

                            client.sendHttpMessage(response);

                            return;
                        }
                    }
                    client.sendHttpMessage(get_page_content("500 Internal Server Error",internal_error_page,digest_info.get_headers()));
                }
                else if (strcmp(frame->getMethod().c_str(),"PUT")==0){

                    QJson::Parser parser;

                    bool ok;

                    // json is a QString containing the data to convert
                    QVariantMap result = parser.parse(frame->getBody().data(), &ok).toMap();

                    if (ok){

                        string username = result["username"].toString().toStdString();
                        string password = result["password"].toString().toStdString();
                        int role = result["role"].toInt();

                        if (strcmp(username.data(),"")==0 || strcmp(password.data(),"")==0 || role<0){

                            client.sendHttpMessage(get_page_content("500 Internal Server Error",internal_error_page,digest_info.get_headers()));
                            return;

                        }

                        cout << "create user " << username.data() << " with role " << role << endl;

                        bool success=database->insert_user(username,password,role);

                        if (success){
                            client.sendHttpMessage(get_page_content("200 OK",ok_page,digest_info.get_headers()));
                            return;
                        }
                    }
                    client.sendHttpMessage(get_page_content("500 Internal Server Error",internal_error_page,digest_info.get_headers()));
                }
                else if (strcmp(frame->getMethod().c_str(),"DELETE")==0){

                }
            }
            else if (strcmp(frame->getUri().c_str(),"/generateCA")==0){

                // create a Parser instance
                QJson::Parser parser;

                bool ok;

                // json is a QString containing the data to convert
                QVariantMap result = parser.parse(frame->getBody().data(), &ok).toMap();

                if (ok){

                    unsigned long long start_date = result["start_date"].toULongLong();
                    unsigned long long end_date = result["end_date"].toULongLong();

                    if (start_date>0 &&
                            end_date >0 &&
                            strcmp(result["common_name"].toString().toStdString().data(),"")!=0 &&
                            strcmp(result["country_name"].toString().toStdString().data(),"")!=0 &&
                            strcmp(result["province_name"].toString().toStdString().data(),"")!=0 &&
                            strcmp(result["locality_name"].toString().toStdString().data(),"")!=0 &&
                            strcmp(result["organization_name"].toString().toStdString().data(),"")!=0 &&
                            strcmp(result["organizational_unit_name"].toString().toStdString().data(),"")!=0){

                        cout << "Generating cert ..." << endl;

                        bool isCA = result["isCA"].toBool();
                        int caserial=-1;

                        if (!isCA && result["CAserial"].toInt()<=0){
                            cerr << "Error while reading generated cert from CA" << endl;
                            client.sendHttpMessage(get_page_content("500 Internal Server Error",internal_error_page,digest_info.get_headers()));
                            return;
                        }
                        else if (!isCA){
                            caserial=result["CAserial"].toInt();
                        }

                        struct tm  date_start_tm;
                        struct tm  date_end_tm;

                        time_t tt1 = static_cast<time_t>(start_date/1000);
                        date_start_tm=*localtime(&tt1);

                        time_t tt2 = static_cast<time_t>(end_date/1000);
                        date_end_tm=*localtime(&tt2);

                        //printf("%d/%d/%d %02d:%02d:%02d\r\n",date_start_tm.tm_mon+1,date_start_tm.tm_mday,1900 + date_start_tm.tm_year,date_start_tm.tm_hour, date_start_tm.tm_min, date_start_tm.tm_sec);
                        //printf("%d/%d/%d %02d:%02d:%02d\r\n",date_end_tm.tm_mon+1,date_end_tm.tm_mday,1900 + date_end_tm.tm_year,date_end_tm.tm_hour, date_end_tm.tm_min, date_end_tm.tm_sec);

                        /*set certificate entries*/
                        cert_entries entries;
                        entries.country_name=result["country_name"].toString().toStdString();
                        entries.state_province_name=result["province_name"].toString().toStdString();
                        entries.locality_name=result["locality_name"].toString().toStdString();
                        entries.organization_name=result["organization_name"].toString().toStdString();
                        entries.organizational_unit_name=result["organizational_unit_name"].toString().toStdString();

                        entries.common_name=result["common_name"].toString().toStdString();

                        certificate_raw certs;
                        certificate_raw *certs_ptr;
                        certs_ptr=&certs;
                        certs_ptr->public_key_pem="";
                        certs_ptr->private_key_pem="";

                        /*instanciate certificate generation lib*/
                        sslgen ssl_gen;

                        ssl_gen.setOutputPEM(true,"","");
                        ssl_gen.setOutputP12(true,"");

                        int serial = database->getNextCertificateSequenceNum();

                        if(isCA){
                            ssl_gen.create_standalone_keys(&entries,&date_start_tm,&date_end_tm,serial,"",2048,&certs);
                        }
                        else{

                            std::string root_ca_key_input = database->getPrivateKeyBySerial(caserial);
                            std::string root_ca_pub_input = database->getPublicKeyBySerial(caserial);

                            ca_cert ca;
                            ca.public_key_pem=root_ca_pub_input;
                            ca.private_key_pem=root_ca_key_input;
                            ca.pass="";

                            ssl_gen.create_signed_keys(&entries,&date_start_tm,&date_end_tm,serial,"",2048,&ca,&certs);
                        }

                        cout << "Inserting cert with serial " << serial << endl;

                        bool success=database->insertGeneratedCert(isCA,
                                                      certs_ptr->public_key_pem,
                                                      certs_ptr->private_key_pem,
                                                      certs_ptr->key_pkcs12,
                                                      start_date,
                                                      end_date,
                                                      serial,
                                                      entries.common_name,
                                                      caserial);
                        if (success){

                            stringstream serial_ss;
                            serial_ss <<"{\"serial\":" << serial << "}";

                            stringstream contentLength;
                            contentLength << serial_ss.str().length();

                            string response = QString("HTTP/1.1 200 OK\r\n").toStdString() +
                                    "Content-Type: application/json\r\nContent-Length: " + contentLength.str() + "\r\n";

                            response+=QString("\r\n").toStdString() + serial_ss.str();

                            client.sendHttpMessage(response);
                            return;
                        }
                        else{

                            cerr << "Cert insertion error" << endl;
                        }
                    }
                }
                client.sendHttpMessage(get_page_content("500 Internal Server Error",internal_error_page,digest_info.get_headers()));
            }
            else if (strcmp(frame->getUri().c_str(),"/sslconfig")==0){

                if (strcmp(frame->getMethod().c_str(),"DELETE")==0){

                    QJson::Parser parser;

                    bool ok;

                    // json is a QString containing the data to convert
                    QVariantMap result = parser.parse(frame->getBody().data(), &ok).toMap();

                    if (ok){

                        int serial = result["serial"].toInt();

                        cout << "delete cert with serial " << serial << endl;

                        bool success=database->deleteCertBySerial(serial);

                        if (success){
                            client.sendHttpMessage(get_page_content("200 OK",ok_page,digest_info.get_headers()));
                            return;
                        }
                    }
                    client.sendHttpMessage(get_page_content("500 Internal Server Error",internal_error_page,digest_info.get_headers()));
                }
                else{

                    QVariantList sslCertList = database->getGeneratedCertList();

                    QVariantMap response;
                    response.insert("data", sslCertList);

                    QJson::Serializer serializer;

                    bool ok;
                    QByteArray json = serializer.serialize(response, &ok);

                    if (ok) {
                        stringstream contentLength;
                        contentLength << json.length();

                        string response = QString("HTTP/1.1 200 OK\r\n").toStdString() +
                                "Content-Type: application/json\r\nContent-Length: " + contentLength.str() + "\r\n";

                        response+=QString("\r\n").toStdString() + json.data();

                        client.sendHttpMessage(response);
                    }
                }
            }
        }
    }
}

/**
 * @brief printHexFormattedCert
 *      print binary file
 * @param data
 *     binary data
 */
void ClientSocketHandler::printHexFormattedCert(std::vector<char> data)
{
    char const hex[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A',   'B','C','D','E','F'};

    char group=0;
    std::string str;
    for (int i = 0; i < data.size(); ++i) {
        const char ch = data[i];
        str.append(&hex[(ch  & 0xF0) >> 4], 1);
        str.append(&hex[ch & 0xF], 1);
        if (group==1){
            str.append(" ");
            group=0;
        }
        else{
            group++;
        }
    }
    cout << str.data() << endl;
}

/**
 * @brief is_public_page
 *      decide if uri is authrorized when unauthenticated
 * @param api_str
 *      uri
 * @return
 */
bool ClientSocketHandler::is_public_page(std::string api_str){

    QString api(api_str.c_str());

    if (!api.endsWith(".html") && api.contains(".")){
        return true;
    }
    return false;
}

/**
 * @brief send_image_page
 *      send an image (binary) to socket
 * @param api_str
 *      http uri
 * @param headers
 *      http headers
 * @param file_name
 *      file name
 * @param client
 *      object containing socket client
 */
void ClientSocketHandler::send_image_page(QString api_str,std::map<std::string,std::string> *headers,std::string file_name,IHttpClient &client){

    string contentType="text/html";

    if (api_str.endsWith(".png"))
        contentType="image/png";
    else if (api_str.endsWith(".jpg"))
        contentType="image/jpeg";
    else if (api_str.endsWith(".ico"))
        contentType="image/x-icon";

    std::string http_request = "HTTP/1.0 200 Ok\r\nContent-Type: " + contentType +"\r\n";

    if (headers!=0){
        if (headers->size()>0){
            for (std::map<std::string,std::string>::iterator it=headers->begin(); it!=headers->end(); ++it)
                http_request+=it->first+": " +it->second + "\r\n";
        }
    }

    std::vector<char> image = bufferize_image(file_name);

    char buffer_length[50];
    sprintf(buffer_length,"%d",image.size());

    http_request+=QString("Content-Length: ").toStdString() + buffer_length + "\r\n\r\n";

    client.writeStringToSocket(http_request);

    client.writeCharToSocket(image.data(),image.size());

    client.flush();
}

/**
 * @brief send_text_page
 *      send a text page to socket
 * @param api_str
 *      http uri
 * @param headers
 *          http headers
 * @param file_name
 *      file name
 * @param client
 *      object containing socket client
 */
void ClientSocketHandler::send_text_page(QString api_str,std::map<std::string,std::string> *headers,std::string file_name,IHttpClient &client){

    string contentType="text/html";

    if (api_str.endsWith(".css"))
        contentType="text/css";
    else if (api_str.endsWith(".js"))
        contentType="application/javascript";

    std::string http_request = "HTTP/1.0 200 Ok\r\nContent-Type: " + contentType +"\r\n";

    std::string headers_str="";

    if (headers!=0){
        if (headers->size()>0){
            for (std::map<std::string,std::string>::iterator it=headers->begin(); it!=headers->end(); ++it){
                http_request+=it->first+": " +it->second + "\r\n";
                if(strcmp(it->first.data(),"WWW-Authenticate")==0){
                    headers_str=it->second;
                }
            }
        }
    }

    QFile file(file_name.data());

    if (!file.open(QIODevice::ReadOnly)) {
        cerr << "Cannot open file for reading: "   << file_name.data() << endl;
        client.sendHttpMessage(get_page_content("404 Not Found",not_found_page,headers));
        return;
    }

    QTextStream in(&file);
    in.setCodec("UTF-8"); // with(out) this line
    QString content = in.readAll();

    std::string additional_header_script="";

    int length=0;

    if (strcmp(contentType.data(),"text/html")==0){
        additional_header_script+="<script>var headers='" + headers_str + "'</script>";
        length+=additional_header_script.length();
    }

    length+=content.toLatin1().length();

    char buffer_length[50];
    sprintf(buffer_length,"%d",length);


    http_request+=QString("Content-Length: ").toStdString() + buffer_length + "\r\n\r\n";

    client.writeStringToSocket(http_request);
    client.writeStringToSocket(additional_header_script);
    client.writeByteArrayToSocket(content.toLatin1());
    client.flush();
}

/**
 * @brief bufferize_image
 *      bufferize image to vector of char
 * @param fileName
 *      image file name
 * @return
 */
std::vector<char> ClientSocketHandler::bufferize_image(std::string fileName){

    FILE* file_stream = fopen(fileName.data(), "rb");
    size_t file_size;

    std::vector<char> buffer;

    if(file_stream != NULL){
        fseek(file_stream, 0, SEEK_END);
        long file_length = ftell(file_stream);
        rewind(file_stream);

        buffer.resize(file_length);

        file_size = fread(&buffer[0], 1, file_length, file_stream);
    }
    else{
        cerr << "Cannot open file for reading: "   << fileName.data() << endl;
    }
    return buffer;
}

/**
 * @brief get_page_content
 *      format a HTTP page from page content and http status with given headers
 * @param status_str
 *      http status
 * @param page
 *      page content
 * @param headers
 *      http headers
 * @return
 *      http full page
 */
std::string ClientSocketHandler::get_page_content(std::string status_str,std::string page,std::map<std::string,std::string> *headers){

    stringstream contentLength;

    std::string content = page;
    contentLength << content.size();

    string response = QString("HTTP/1.1 ").toStdString() + status_str + "\r\n" +
            "Content-Type: text/html\r\nContent-Length: " + contentLength.str() + "\r\n";

    response+=QString("\r\n").toStdString() + content;

    return response;
}

