/*
 * This File is part of Davix, The IO library for HTTP based protocols
 * Copyright (C) CERN 2013
 * Author: Adrien Devresse <adrien.devresse@cern.ch>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
*/

#ifndef DAVIX_NEONREQUEST_H
#define DAVIX_NEONREQUEST_H

#include <vector>
#include <utility>
#include <memory>
#include <string>

#include <davix_internal.hpp>

#include <ne_request.h>
#include <ne_auth.h>
#include <neon/neonsessionfactory.hpp>

#include <request/httprequest.hpp>
#include <neon/neonsession.hpp>

#include "../backend/BackendRequest.hpp"

namespace Davix {

#define NEON_BUFFER_SIZE        65000
#define NEON_REDIRECT_LIMIT 5

class NEONSessionFactory;
class NEONSession;
class HttpRequest;
class NEONSessionExtended;

class NEONRequest : public BackendRequest
{
public:
    NEONRequest(HttpRequest &h, Context& f, const Uri & uri_req);
    virtual ~NEONRequest();

    void setParameters(const RequestParams &p ){
        params = p;
    }


    //  Execute the given request and return result to the buffer result
    //  @return 0 on success
    int executeRequest(DavixError** err) ;

    int beginRequest(DavixError** err);
    /**
      read a block of a maximum size bytes in the request
      @param buffer : buffer to fill
      @param max_size : maximum number of bytes to set
    */
    dav_ssize_t readBlock(char* buffer, dav_size_t max_size,DavixError** err);

    dav_ssize_t readSegment(char* buffer, dav_size_t size_read, bool stop_at_line_boundary, DavixError** err);

    dav_ssize_t readLine(char* buffer, dav_size_t max_size, DavixError** err);

    dav_ssize_t readToFd(int fd, dav_size_t read_size, DavixError** err);

    /**
      finish an already started request
     */
    int endRequest(DavixError** err);
    /**
      get a reference to the current result for synchronous full request
     */
    const char* getAnswerContent();

    std::vector<char> & getAnswerContentVec();

    /**
     * get content length
     **/
    dav_ssize_t getAnswerSize() const;

    /**
     * get last modified
     **/
    time_t getLastModified() const;

    /**
      clear the current result
    */
    void clearAnswerContent();

    int getRequestCode();

    bool getAnswerHeader(const std::string &header_name, std::string &value) const;

    size_t getAnswerHeaders( std::vector<std::pair<std::string, std::string > > & vec_headers) const;

private:

    // request parameters
    RequestParams params;
    // neon internal field
    std::unique_ptr<NEONSession> _neon_sess;


    ne_request * _req;
    // req info
    int _number_try;
    // number of redirects so far
    int _redirects;
    // read info
    dav_ssize_t _total_read_size, _last_read;

    std::vector<char> _vec;
    std::vector<char> _vec_line;

    // answer length
    mutable dav_ssize_t _ans_size;

    // timeout management
    Chrono::TimePoint _expiration_time;

    HttpRequest & _h;
    Context& _c;
    bool req_started, req_running;
    int _last_request_flag;

    int _accepted_202_retries;

    bool _early_termination;
    DavixError* _early_termination_error;

    ////////////////////////////////////////////
    // Private Members
    dav_ssize_t getAnswerSizeFromHeaders() const;

    int startRequest(DavixError** err);

    int processRedirection(int neonCode, DavixError** err); // analyze and process redirection if needed

    void resetRequest();

    int instanceSession(DavixError** err);

    bool checkTimeout(DavixError** err);

    void configureRequest();

    void configureS3params();

    void configureAzureParams();

    void configureGcloudParams();

    void cancelSessionReuse();

    // create initial neon request object
    int createRequest(DavixError** err);

    // negociate the request : authentification, redirection, name resolution
    int negotiateRequest(DavixError** err);

    // redirection logic
    int redirectRequest(DavixError** err);

    // redirecttion caching cleaning function
    bool requestCleanup();

    void freeRequest();


    void createError(int ne_status, DavixError** err);

    /**
      internal, try to authentification with pkcs12 credential
    */
    int try_pkcs12_authentification(ne_session *sess, const ne_ssl_dname *const *dnames, DavixError** err);

    NEONRequest(const NEONRequest & req);
    NEONRequest & operator=(const NEONRequest & req);

	static ssize_t neon_body_content_provider(void* userdata, char* buffer, size_t buflen);

    static void neon_hook_pre_send(ne_request *req, void *userdata,
                                           ne_buffer *header);

    static void neon_hook_pre_rec(ne_request *req, void *userdata,
                                        const ne_status *status);


    friend class HttpRequest;
    friend class NEONSessionExtended;
};


//
// translate a neon error code to a davix one
//
void neon_to_davix_code(int ne_status,ne_session* sess, const std::string & scope, DavixError** err);


void neon_simple_req_code_to_davix_code(int ne_status, ne_session* sess, const std::string & scope, DavixError** err);

void configureRequestParamsProto(const Uri &uri, RequestParams &params);


} // namespace Davix

#endif // DAVIX_NEONREQUEST_H
