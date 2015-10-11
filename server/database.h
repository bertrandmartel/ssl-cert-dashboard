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
    database.h

    Manage database insertion/deletion/update

    @author Bertrand Martel
    @version 1.0
*/
#ifndef DATABASE_H
#define DATABASE_H

#include <QList>
#include "mongo/client/dbclient.h"

#include <QObject>
#include <QTimer>
#include "QVariantList"
#include "QByteArray"
#include "vector"
#include "DigestManager.h"

#define DATABASE_LOCAL_STORAGE     "localhost"

#define CERT_TYPE_FIELD            "certType"
#define CERT_TYPE_LOCATION_FIELD   "certLocation"
#define CERT_EXPIRATION_DATE_FIELD "certExpirationDate"
#define CERT_START_DATE            "certStartDate"
#define CERT_END_DATE              "certEndDate"
#define CERT_RECORD_DATE_FIELD     "certRecordDate"
#define CERT_COMMON_NAME           "certCommonName"
#define CERT_IS_CA                 "certisCa"
#define CERT_USE                   "certUse"
#define CERT_PUBLIC_BODY           "certPublicBody"
#define CERT_P12                   "certP12Body"
#define CERT_PRIVATE_BODY          "certPrivateBody"
#define CERT_STATUS                "certStatus"
#define CERT_PASS                  "certPass"
#define CERT_SIGN_BY_SERIAL        "certSignBySerial"
#define CERT_SYMBOLIC_NAME         "certSymbolicName"
#define CERT_SEQ_NUM               "certSeqNum"

#define USER_USERNAME              "username"
#define USER_ROLE                  "role"
#define USER_KEY                   "key"
#define USER_CREATION              "creationDate"
#define USER_LAST_LOGIN            "lastLoginDate"

class Database : public QObject
{
    Q_OBJECT

public:

    Database(QObject *parent = 0);

    /**
     * @brief init_user
     *      insert one default user if no user are defined in user database
     */
    void init_user();

    /**
     * @brief setDigestParams
     *      set digest parameters
     * @param realm
     *      digest realm
     * @param digest_algo
     *      digest algorithm (ALGO_SHA1 / ALGO_MD5)
     */
    void setDigestParams(std::string realm,digest_algorithm digest_algo);

    /**
     * @brief setKey
     *      set XOR key for encryption
     * @param key
     *      key value
     */
    void setKey(QByteArray key);

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
    void encrypt(char * data,int length,QByteArray key,char *crypted_data);

    /**
     * @brief set_debug
     *      enable/disable debug
     * @param debug
     */
    void set_debug(bool debug);

    /**
     * @brief get_user_key
     *      retrieve user key value in database
     * @param username
     *      user name
     * @return
     */
    std::string get_user_key(std::string username);

    /**
     * @brief get_user_list
     *      retrieve user list
     * @return
     */
    QVariantList get_user_list();

    /**
     * @brief getGeneratedCertList
     *      retrieve certificates list
     * @return
     */
    QVariantList getGeneratedCertList();

    /**
     * @brief deleteUser
     *      delete a user by username
     * @param username
     *      user name
     * @return
     *      success status
     */
    bool deleteUser(std::string username);

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
    bool insert_user(std::string username,std::string key,int role);

    /**
     * @brief getNextCertificateSequenceNum
     *      get new incremented certificate sequential number
     * @return
     */
    int getNextCertificateSequenceNum();

    /**
     * @brief getNextUserSequenceNum
     *      get new incremented user sequential number
     * @return
     */
    int getNextUserSequenceNum();

    /**
     * @brief DigestManager::generateRandomNum
     *      Generate random alpha numeric string
     * @param num
     *      length of random string
     * @return
     */
    std::string generateRandomNum(unsigned int num);

    /**
     * @brief deleteCertBySerial
     *      delete a cert
     * @param serial
     *      cert serial number
     * @return
     *      success status
     */
    bool deleteCertBySerial(int serial);

    /**
     * @brief getPrivateKeyBySerial
     *      retrieve private key by serial number
     * @param serial
     *      cert serial number
     * @return
     *      private key
     */
    std::string getPrivateKeyBySerial(int serial);

    /**
     * @brief getPublicKeyBySerial
     *      retrieve public key by serial number
     * @param serial
     *      cert serial number
     * @return
     *      public key
     */
    std::string getPublicKeyBySerial(int serial);

    /**
     * @brief getGeneratedCertListBySerial
     *      retrieve certificate list signed with a given CA cert by serial number
     * @param serial
     *      cert serial number
     * @return
     *
     */
    QVariantList getGeneratedCertListBySerial(int serial);

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
    bool insertGeneratedCert(bool               isCaCert,
                             std::string        certPublic,
                             std::string        certPrivate,
                             std::vector<char>  certP12,
                             unsigned long long start_date,
                             unsigned long long end_date,
                             int                serial,
                             std::string        commonName,
                             int                signBySerialNum);

private:

    /**
      * @brief con
      *     mongo client connection object
      */
     mongo::DBClientConnection con;

     /**
      * @brief debug
      *     debug status
      */
     bool debug;

     /**
      * @brief key
      *     XOR encryption key
      */
     QByteArray key;

     /**
      * @brief realm
      *     digest realm
      */
     std::string realm;

     /**
      * @brief digest_algo
      *     algorithm used in digest
      */
     digest_algorithm digest_algo;
};

#endif // DATABASE_H
