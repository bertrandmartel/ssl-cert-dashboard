# SSL certificates certificates


Web dashboard of SSL certificates with QT4 non blocking http server / mongoDB back-end

## Features

* SSL Dashboard
  * creation of CA self signed certificate
  * creation of signed certificate (from a CA cert)
  * download public/private key + PKCS12 cert containing CA + key pair
  * listing of all created certificates (with grouping according to Certificate Authority)

* Authentication/Session
  * digest authentication
  * session persistence with cookie

* Back-end
  * non blocking http server built with QT4 framework
  * mongoDB client managing all database interactions

* Database encryption
  *XOR encryption for user password

<hr/>

Server can be launched with a configuration file letting you change ssl settings / XOR encryption key / digest algorithm used in authentication

<hr/>

##Prerequesites

* libboost-all-dev package installed
* mongod package installed

##Build

```
cd ssl-dashboard

make 

```

##Launch

Edit to your liking file ``config.json``, here is default configuration :

```
{
    "port"           : 4343,
    "ip"             : "127.0.0.1",
    "useSSL"         : true,
    "publicCert"     : "~/ssl-dashboard/certs/server/server.crt",
    "privateCert"    : "~/ssl-dashboard/certs/server/server.key",
    "caCerts"        : "~/ssl-dashboard/certs/ca.crt",
    "privateKeyPass" : "12345",
    "digestRealm"    : "bertrandmartel_realm",
    "webPath"        : "/ssl-dashboard/web",
    "keyFile"        : "~/ssl-dashboard/rand.txt",
    "digestAlgo"     : "MD5"
}
```

Then to launch server check ``https://127.0.0.1:4343/login`` :

```
cd ssl-dashboard/release
./ssl-dashboard ../config.json

```
Then in a browser :


![login](https://raw.github.com/bertrandmartel/ssl-cert-dashboard/master/img/login.png)


##Memory checking

Using valgrind :

```
valgrind --tool=memcheck --leak-check=full --suppressions=../memcheck.suppress ./ssl-dashboard ../config.json

```

##External libraries

* OpenSSL 1.0.1p (ApacheV1)

* MongoDB C++ connector (https://github.com/mongodb/mongo-cxx-driver) (ApacheV2)

* SSL cert generator library https://github.com/bertrandmartel/ssl-cert-generator-lib (MIT)

* Digest authentication library https://github.com/bertrandmartel/digest-auth-session-cpp (MIT)

* Non blocking http server https://github.com/bertrandmartel/socket-multiplatform/tree/master/server/server-socket/non-blocking (MIT)

* Json decoding encoding is managed by QJson http://qjson.sourceforge.net/ (LGPLv2)

* QT4 (LGPLv2)

* Twitter Bootstrap (ApacheV2)

* JQuery (MIT)

* Datatable https://www.datatables.net (MIT) 

* Blob.js https://github.com/eligrey/Blob.js (MIT)

* FileSaver.js https://github.com/eligrey/FileSaver.js (MIT)

* Base64.js Daniel Guerrero (BSD)

* Clockpicker https://github.com/weareoutman/clockpicker (MIT)

<h4>Specifications</h4>

* OpenSSL is built with -DPURIFY option to avoid valgrind to complain about uninitialized data ("Conditional jump or move depends on uninitialised value(s)" warnings).

https://www.openssl.org/docs/faq.html#PROG14

* MongoDB C++ connector used is built from https://github.com/mongodb/mongo-cxx-driver/tree/26compat

<hr/>

##Other views

* SSL cert dashboard

![ssl cert dashboard](https://raw.github.com/bertrandmartel/ssl-cert-dashboard/master/img/dashboard.png)

* SSL cert creation

![ssl cert creation](https://raw.github.com/bertrandmartel/ssl-cert-dashboard/master/img/createcert.png)

* User dashboard

![user dashboard](https://raw.github.com/bertrandmartel/ssl-cert-dashboard/master/img/users.png)

##License

[License MIT](https://github.com/bertrandmartel/ssl-cert-dashboard/blob/master/LICENSE.md)

<hr/>

* Project is Qt4 compliant
* Development on QtCreator
