/*
 * Copyright (C) 1996-2014 The Squid Software Foundation and contributors
 *
 * Squid software is distributed under GPLv2+ license and includes
 * contributions from numerous individuals and organizations.
 * Please see the COPYING and CONTRIBUTORS files for details.
 */

#ifndef _SQUID_SRC_AUTH_NEGOTIATE_USERREQUEST_H
#define _SQUID_SRC_AUTH_NEGOTIATE_USERREQUEST_H

#include "auth/UserRequest.h"
#include "helper/forward.h"

class ConnStateData;
class HttpReply;
class HttpRequest;

namespace Auth
{
namespace Negotiate
{

/// \ingroup AuthNegotiateAPI
class UserRequest : public Auth::UserRequest
{
    MEMPROXY_CLASS(Auth::Negotiate::UserRequest);

public:
    UserRequest();
    virtual ~UserRequest();
    virtual int authenticated() const;
    virtual void authenticate(HttpRequest * request, ConnStateData * conn, http_hdr_type type);
    virtual Direction module_direction();
    virtual void startHelperLookup(HttpRequest *request, AccessLogEntry::Pointer &al, AUTHCB *, void *);
    virtual const char *credentialsStr();

    virtual void addAuthenticationInfoHeader(HttpReply * rep, int accel);

    virtual const char * connLastHeader();

    /* we need to store the helper server between requests */
    helper_stateful_server *authserver;
    void releaseAuthServer(void); ///< Release the authserver helper server properly.

    /* what connection is this associated with */
    /* ConnStateData * conn;*/

    /* our current blob to pass to the client */
    char *server_blob;
    /* our current blob to pass to the server */
    char *client_blob;

    /* currently waiting for helper response */
    unsigned char waiting;

    /* need access to the request flags to mess around on pconn failure */
    HttpRequest *request;

private:
    static HLPCB HandleReply;
};

} // namespace Negotiate
} // namespace Auth

#endif /* _SQUID_SRC_AUTH_NEGOTIATE_USERREQUEST_H */
