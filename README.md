# SSL certificates certificates


Web dashboard of SSL certificates with QT4 non blocking http server / mongoDB back-end


<h4>SSL Dashboard</h4> 

* creation of CA self signed certificate
* creation of signed certificate (from a CA cert)
* download public/private key + PKCS12 cert containing CA + key pair
* listing of all created certificates (with grouping according to Certificate Authority)

<h4>Authentication/Session</h4>

* digest authentication
* session persistence with cookie

<h4>Back-end</h4>

* non blocking http server built with QT4 framework
* mongoDB client managing all database interactions

<h4>Database encryption</h4>

* XOR encryption for user password

<hr/>

Server can be launched with a configuration file letting you to change ssl settings / XOR encryption key / digest algorithm used in authentication

<hr/>

<h4>Prerequesites</h4>

* libboost-all-dev package installed
* mongod package installed

<h4>Build</h4>

```
cd ssl-dashboard

make 

```

<h4>Launch</h4>

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
    "digestRealm"    : "akinaru_realm",
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


![login](https://raw.github.com/akinaru/ssl-cert-dashboard/master/img/login.png)


<h4>Memory checking</h4>

Using valgrind :

```
valgrind --tool=memcheck --leak-check=full --suppressions=../memcheck.suppress ./ssl-dashboard ../config.json

```

<h4>External libraries</h4>

* OpenSSL 1.0.1p (ApacheV1)

* MongoDB C++ connector (https://github.com/mongodb/mongo-cxx-driver) (ApacheV2)

* SSL cert generator library https://github.com/akinaru/ssl-cert-generator-lib (MIT)

* Digest authentication library https://github.com/akinaru/digest-auth-session-cpp (MIT)

* Non blocking http server https://github.com/akinaru/socket-multiplatform/tree/master/server/server-socket/non-blocking (MIT)

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

<h4>Other views</h4>

* SSL cert dashboard

![ssl cert dashboard](https://raw.github.com/akinaru/ssl-cert-dashboard/master/img/dashboard.png)

* SSL cert creation

![ssl cert creation](https://raw.github.com/akinaru/ssl-cert-dashboard/master/img/createcert.png)

* User dashboard

![user dashboard](https://raw.github.com/akinaru/ssl-cert-dashboard/master/img/users.png)

<h4>License</h4>

[License MIT](https://github.com/akinaru/akinaru/ssl-cert-dashboard/blob/master/LICENSE.md)

<hr/>

* Project is Qt4 compliant
* Development on QtCreator
