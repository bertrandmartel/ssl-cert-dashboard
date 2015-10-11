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
    ClientSocketHandler.h
    Client socket managing event handler

    @author Bertrand Martel
    @version 1.0
*/
#ifndef CLIENTSOCKETHANDLER_H
#define CLIENTSOCKETHANDLER_H

#include "httpserverinter/IClientEventListener.h"
#include "DigestManager.h"
#include "vector"
#include "QFile"
#include "database.h"
#include "vector"

/**
 * @brief The ClientSocketHandler class
 *      Client socket managing event handler
 */
class ClientSocketHandler :  public IClientEventListener
{
    public:

         /**
         * @brief ClientSocketHandler
         *      build client socket handler
         */
        ClientSocketHandler(Database* database,DigestManager * digest_manager,std::string web_path,std::string realm);

        ~ClientSocketHandler();

        /**
         * called when an http request has been received from client
         *
         * @param client
         * 		client object
         * @param message
         * 		message delivered
         */
        void onHttpRequestReceived(IHttpClient &client,Ihttpframe* consumer,std::string peer_address);

        /**
         * called when an http response has been received from client
         *
         * @param client
         * 		client object
         * @param message
         * 		message delivered
         */
        void onHttpResponseReceived(IHttpClient &client,Ihttpframe* consumer,std::string peer_address);

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
        std::string get_page_content(std::string status_str,std::string page,std::map<std::string,std::string> *headers);

        /**
         * @brief bufferize_image
         *      bufferize image to vector of char
         * @param fileName
         *      image file name
         * @return
         */
        std::vector<char> bufferize_image(std::string fileName);

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
        void send_text_page(QString api_str,std::map<std::string,std::string> *headers,std::string file_name,IHttpClient &client);

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
        void send_image_page(QString api_str,std::map<std::string,std::string> *headers,std::string file_name,IHttpClient &client);

        /**
         * @brief is_public_page
         *      decide if uri is authrorized when unauthenticated
         * @param api_str
         *      uri
         * @return
         */
        bool is_public_page(std::string api_str);

        /**
         * @brief printHexFormattedCert
         *      print binary file
         * @param data
         *     binary data
         */
        void printHexFormattedCert(std::vector<char> data);

private:

        /**
         * @brief digest_manager
         *      digest manager managing digest authentication and session
         */
        DigestManager * digest_manager;

        /**
         * @brief unauthorized_page
         *      unauthorized page content
         */
        std::string unauthorized_page;

        /**
         * @brief internal_error_page
         *      internal error page content
         */
        std::string internal_error_page;

        /**
         * @brief not_found_page
         *      not found page content
         */
        std::string not_found_page;

        /**
         * @brief web_path
         *      default web path
         */
        std::string web_path;

        /**
         * @brief ok_page
         *      200 OK page content
         */
        std::string ok_page;

        /**
         * @brief database
         *      object managing all database interactions
         */
        Database *database;

        /**
         * @brief realm
         *      digest realm
         */
        std::string realm;
};

#endif // CLIENTSOCKETHANDLER_H
