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
    database.cpp

    Manage database insertion/deletion/update

    @author Bertrand Martel
    @version 1.0
*/
#include "database.h"
#include "QTextStream"
#include "iostream"
#include "QFile"
#include "QDateTime"
#include "mongo/client/dbclient.h"
#include "mongo/bson/bson.h"
#include "DigestManager.h"
#include "QCryptographicHash"

using namespace std;
using namespace mongo;
using namespace bson;

Database::Database(QObject *parent) : QObject(parent){

    debug=false;

    try {

        con.connect(DATABASE_LOCAL_STORAGE);
        cout << "Successfully connected to Mongo database..." << endl;

    } catch( const DBException &e ) {
        cout << "caught " << e.what() << endl;
    }
}

/**
 * @brief setDigestParams
 *      set digest parameters
 * @param realm
 *      digest realm
 * @param digest_algo
 *      digest algorithm (ALGO_SHA1 / ALGO_MD5)
 */
void Database::setDigestParams(std::string realm,digest_algorithm digest_algo){

    this->realm=realm;
    this->digest_algo=digest_algo;

}

/**
 * @brief setKey
 *      set XOR key for encryption
 * @param key
 *      key value
 */
void Database::setKey(QByteArray key){
    this->key=key;
}

/**
 * @brief set_debug
 *      enable/disable debug
 * @param debug
 */
void Database::set_debug(bool debugArg){
    debug=debugArg;
}

/**
 * @brief getGeneratedCertListBySerial
 *      retrieve certificate list signed with a given CA cert by serial number
 * @param serial
 *      cert serial number
 * @return
 *
 */
QVariantList Database::getGeneratedCertListBySerial(int serial){

    auto_ptr<DBClientCursor> cursor = con.query("ssldashboard.generatedcerts", BSON(CERT_SEQ_NUM<<serial));

    QVariantList listOfCerts;

    if (cursor->more()) {

        QVariantMap certElement;

        BSONObj currentObj =cursor->next();

        int len;
        const char* p12Cert = currentObj.getField(CERT_P12).binData(len);

        if (debug)
            cout << "p12 length : " << len << endl;

        QByteArray p12_array(p12Cert,len);

        certElement.insert(CERT_PRIVATE_BODY , currentObj.getStringField(CERT_PRIVATE_BODY));
        certElement.insert(CERT_PUBLIC_BODY  , currentObj.getStringField(CERT_PUBLIC_BODY));
        certElement.insert(CERT_P12          , p12_array.toBase64().data());

        listOfCerts << certElement;
    }
    return listOfCerts;
}

/**
 * @brief getGeneratedCertList
 *      retrieve certificates list
 * @return
 */
QVariantList Database::getGeneratedCertList(){

    auto_ptr<DBClientCursor> cursor = con.query("ssldashboard.generatedcerts", Query());

    QVariantList listOfCerts;

    while (cursor->more()){

        QVariantMap certElement;

        BSONObj currentObj =cursor->next();

        certElement.insert(CERT_START_DATE           ,currentObj.getField(CERT_START_DATE).date().toString().data());
        certElement.insert(CERT_END_DATE             ,currentObj.getField(CERT_END_DATE).date().toString().data());
        certElement.insert(CERT_RECORD_DATE_FIELD    ,currentObj.getField(CERT_RECORD_DATE_FIELD).date().toString().data());
        certElement.insert(CERT_IS_CA                ,currentObj.getBoolField(CERT_IS_CA));
        certElement.insert(CERT_COMMON_NAME          ,currentObj.getStringField(CERT_COMMON_NAME));
        certElement.insert(CERT_SEQ_NUM              ,currentObj.getIntField(CERT_SEQ_NUM));
        certElement.insert(CERT_SIGN_BY_SERIAL       ,currentObj.getIntField(CERT_SIGN_BY_SERIAL));
        listOfCerts << certElement;
    }
    return listOfCerts;
}


/**
 * @brief deleteUser
 *      delete a user by username
 * @param username
 *      user name
 * @return
 *      success status
 */
bool Database::deleteUser(std::string username)
{
    con.remove("ssldashboard.users", QUERY(USER_USERNAME<<username));

    string err = con.getLastError();

    bool ok = err.empty();

    if (!ok)
        cerr << err.data() << endl;

    return ok;
}

/**
 * @brief deleteCertBySerial
 *      delete a cert
 * @param serial
 *      cert serial number
 * @return
 *      success status
 */
bool Database::deleteCertBySerial(int serial)
{
    con.remove("ssldashboard.generatedcerts", QUERY(CERT_SEQ_NUM<<serial));

    string err = con.getLastError();

    bool ok = err.empty();

    if (!ok)
        cerr << err.data() << endl;

    return ok;
}

/**
 * @brief getNextCertificateSequenceNum
 *      get new incremented certificate sequential number
 * @return
 */
int Database::getNextCertificateSequenceNum(){

    con.update("ssldashboard.generatedCertSeqNum", QUERY("_id"<<"certId"),
             BSON("$inc"<<BSON("seq"<<1)), true, true);

    auto_ptr<DBClientCursor> cursor = con.query("ssldashboard.generatedCertSeqNum", BSON("_id"<<"certId"));

    if (cursor->more()){

        BSONObj currentObj =cursor->next();

        if (currentObj.hasField("seq") && currentObj.getField("seq").isNumber()){

           return currentObj.getIntField("seq");
        }
    }
    else{

        BSONObjBuilder box;
        box.append("_id","certId");
        box.append("seq",0);

        BSONObj seqNum = box.obj();

        con.insert("ssldashboard.generatedCertSeqNum", seqNum);
    }

    string err = con.getLastError();

    if( !err.empty()){
        cerr << err.data() << endl;
        return -1;
    }

    return -1;
}

/**
 * @brief getNextUserSequenceNum
 *      get new incremented user sequential number
 * @return
 */
int Database::getNextUserSequenceNum(){

    con.update("ssldashboard.userSeqNum", QUERY("_id"<<"userId"),
             BSON("$inc"<<BSON("seq"<<1)), true, true);

    auto_ptr<DBClientCursor> cursor = con.query("ssldashboard.userSeqNum", BSON("_id"<<"userId"));

    if (cursor->more()){

        BSONObj currentObj =cursor->next();

        if (currentObj.hasField("seq") && currentObj.getField("seq").isNumber()){

           return currentObj.getIntField("seq");
        }
    }
    else{

        BSONObjBuilder box;
        box.append("_id","userId");
        box.append("seq",0);

        BSONObj seqNum = box.obj();

        con.insert("ssldashboard.userSeqNum", seqNum);
    }

    string err = con.getLastError();

    if( !err.empty()){
        cerr << err.data() << endl;
        return -1;
    }

    return -1;
}

/**
 * @brief insertGeneratedCert
 *      insert a new certificate in database
 *
 * @param isCaCert
 *        cert is CA or not
 * @param certPublic
 *      public key value
 * @param certPrivate
 *      private key value
 * @param certP12
 *      p12 cert value
 * @param start_date
 *      cert starting date
 * @param end_date
 *      cert ending date
 * @param serial
 *      cert serial number
 * @param commonName
 *      cert common name
 * @param signBySerialNum
 *      cert is sign with a given cert identified by serial number (-1 if self signed)
 * @return
 *      success status
 */
bool Database::insertGeneratedCert(bool isCaCert,
     std::string certPublic,
     std::string certPrivate,
     std::vector<char> certP12,
     unsigned long long  start_date,
     unsigned long long  end_date,
     int serial,
     std::string commonName,
     int signBySerialNum
     ){

    QDateTime currentDate = QDateTime::currentDateTime();

    BSONObjBuilder generatedCert;
    generatedCert.append(CERT_SEQ_NUM              ,serial);
    generatedCert.append(CERT_COMMON_NAME          ,commonName);
    generatedCert.append(CERT_IS_CA                ,isCaCert);
    generatedCert.append(CERT_SIGN_BY_SERIAL       ,signBySerialNum);
    generatedCert.append(CERT_START_DATE           ,Date_t(start_date));
    generatedCert.append(CERT_END_DATE             ,Date_t(end_date));
    generatedCert.append(CERT_RECORD_DATE_FIELD    ,Date_t(currentDate.toMSecsSinceEpoch()));
    generatedCert.append(CERT_PUBLIC_BODY          ,certPublic);
    generatedCert.append(CERT_PRIVATE_BODY         ,certPrivate);
    generatedCert.appendBinData(CERT_P12           ,certP12.size()        ,BinDataGeneral,certP12.data());

    BSONObj generatedCertObj = generatedCert.obj();
    con.insert("ssldashboard.generatedcerts", generatedCertObj);

    string e = con.getLastError();

    if( !e.empty() ) {
        cerr << e << endl;
        return false;
    }

    return  true;
}

/**
 * @brief encrypt
 *      XOR encryption
 * @param data
 *      data to encode
 * @param length
 *      length of data
 * @param key
 *      key to encrypt
 * @param crypted_data
 *      crypted data
 */
void Database::encrypt(char * data,int length,QByteArray key,char *crypted_data){

    int i;
    for (i=0;i < length;i++){

        crypted_data[i]=data[i] ^ key.at(i);
    }
}

/**
 * @brief DigestManager::generateRandomNum
 *      Generate random alpha numeric string
 * @param num
 *      length of random string
 * @return
 */
std::string Database::generateRandomNum(unsigned int num){

    std::string str;

    const char alphanum[] =
       "0123456789"
       "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
       "abcdefghijklmnopqrstuvwxyz";

    int stringLength = sizeof(alphanum) - 1;

    for(unsigned int i = 0; i < num;i ++){

        str += alphanum[rand() % stringLength];
    }
    return str;
}

/**
 * @brief getPrivateKeyBySerial
 *      retrieve private key by serial number
 * @param serial
 *      cert serial number
 * @return
 *      private key
 */
std::string Database::getPrivateKeyBySerial(int serial){

    auto_ptr<DBClientCursor> cursor = con.query("ssldashboard.generatedcerts", BSON(CERT_SEQ_NUM << serial));

    if (cursor->more()){

        BSONObj currentObj =cursor->next();

        if (currentObj.hasField(CERT_PRIVATE_BODY)){
            return currentObj.getStringField(CERT_PRIVATE_BODY);
        }
    }
    return "";
}

/**
 * @brief getPublicKeyBySerial
 *      retrieve public key by serial number
 * @param serial
 *      cert serial number
 * @return
 *      public key
 */
std::string Database::getPublicKeyBySerial(int serial){

    auto_ptr<DBClientCursor> cursor = con.query("ssldashboard.generatedcerts", BSON(CERT_SEQ_NUM << serial));

    if (cursor->more()){

        BSONObj currentObj =cursor->next();

        if (currentObj.hasField(CERT_PUBLIC_BODY)){
            return currentObj.getStringField(CERT_PUBLIC_BODY);
        }
    }
    return "";
}

/**
 * @brief insert_user
 *      insert a new user in database
 * @param username
 *      user name
 * @param key
 *      XOR key to encrypt password
 * @param role
 *      user role (0 = normal / 1 = admin)
 * @return
 *      success status
 */
bool Database::insert_user(std::string username,std::string key_data,int role) {

    QDateTime currentDate = QDateTime::currentDateTime();

    std::string hash = username + ":" + realm + ":" + key_data;
    QByteArray convert_hash;

    if (digest_algo==ALGO_MD5)
        convert_hash = QCryptographicHash::hash(hash.data(), QCryptographicHash::Md5).toHex();
    else if (digest_algo==ALGO_SHA1)
        convert_hash = QCryptographicHash::hash(hash.data(), QCryptographicHash::Sha1).toHex();

    char * crypted_data = new char[convert_hash.length()];

    encrypt((char*)convert_hash.data(),convert_hash.length(),key,crypted_data);

    BSONObjBuilder generatedCert;
    generatedCert.append(USER_USERNAME   ,username);
    generatedCert.append(USER_ROLE       ,role);
    generatedCert.appendBinData(USER_KEY ,convert_hash.length(),BinDataGeneral,crypted_data);
    generatedCert.append(USER_CREATION   ,Date_t(currentDate.toMSecsSinceEpoch()));
    generatedCert.append(USER_LAST_LOGIN ,Date_t(currentDate.toMSecsSinceEpoch()));

    delete[] crypted_data;

    BSONObj userObj = generatedCert.obj();
    con.insert("ssldashboard.users", userObj);

    string e = con.getLastError();

    if( !e.empty() ) {

        cerr << e << endl;
        return false;
    }

    return  true;
}

/**
 * @brief get_user_list
 *      retrieve user list
 * @return
 */
QVariantList Database::get_user_list(){

    auto_ptr<DBClientCursor> cursor = con.query("ssldashboard.users", Query());

    QVariantList userList;

    while (cursor->more()){

        QVariantMap userElement;

        BSONObj currentObj =cursor->next();

        userElement.insert(USER_USERNAME   ,currentObj.getStringField(USER_USERNAME));
        userElement.insert(USER_ROLE       ,currentObj.getIntField(USER_ROLE));
        userElement.insert(USER_CREATION   ,currentObj.getField(USER_CREATION).date().toString().data());
        userElement.insert(USER_LAST_LOGIN ,currentObj.getField(USER_LAST_LOGIN).date().toString().data());

        userList << userElement;
    }
    return userList;
}

/**
 * @brief init_user
 *      insert one default user if no user are defined in user database
 */
void Database::init_user(){

    QVariantList list = get_user_list();

    if (list.size()==0){

        //insert default user
        insert_user("demo","demo",1);
        cout << "insert default user" << endl;
    }
}

/**
 * @brief get_user_key
 *      retrieve user key value in database
 * @param username
 *      user name
 * @return
 */
std::string Database::get_user_key(std::string username){

    auto_ptr<DBClientCursor> cursor = con.query("ssldashboard.users", BSON("username" << username.data()));

    if (cursor->more()){

        BSONObj currentObj =cursor->next();

        if (currentObj.hasField("key")){

            int len;
            const char* user_key = currentObj.getField("key").binData(len);

            char * crypted_data = new char[len];
            encrypt((char*)user_key,len,key,crypted_data);

            string ret_str(crypted_data,crypted_data+len);

            delete[] crypted_data;
            crypted_data=0;

            return ret_str;
        }
    }
    return "";
}
