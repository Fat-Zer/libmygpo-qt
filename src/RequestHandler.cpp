/***************************************************************************
* This file is part of libmygpo-qt                                         *
* Copyright (c) 2010 Stefan Derkits <stefan@derkits.at>                    *
* Copyright (c) 2010 Christian Wagner <christian.wagner86@gmx.at>          *
* Copyright (c) 2010 Felix Winter <ixos01@gmail.com>                       *
*                                                                          *
* This library is free software; you can redistribute it and/or            *
* modify it under the terms of the GNU Lesser General Public               *
* License as published by the Free Software Foundation; either             *
* version 2.1 of the License, or (at your option) any later version.       *
*                                                                          *
* This library is distributed in the hope that it will be useful,          *
* but WITHOUT ANY WARRANTY; without even the implied warranty of           *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU        *
* Lesser General Public License for more details.                          *
*                                                                          *
* You should have received a copy of the GNU Lesser General Public         *
* License along with this library; if not, write to the Free Software      *
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 *
* USA                                                                      *
***************************************************************************/

#include "RequestHandler.h"
#include <QAuthenticator>
#include <QEventLoop>


using namespace mygpo;

RequestHandler::RequestHandler(const QString& username, const QString& password) : m_errorFlag(QNetworkReply::NoError), m_username(username), m_password(password), m_loginFailed(false)
{
    QObject::connect(&manager, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)), this,
                     SLOT(authenticate( QNetworkReply*, QAuthenticator*)));
}

RequestHandler::RequestHandler() : m_errorFlag(QNetworkReply::NoError), m_username(), m_password(), m_loginFailed(false)
{
}

int RequestHandler::getRequest(QByteArray& response, const QUrl& url)
{
    m_loginFailed = false;
    m_errorFlag = QNetworkReply::NoError;
    QNetworkRequest request(url);
    QNetworkReply *reply = manager.get(request);

    waitForReply(*reply);

    if (m_errorFlag == QNetworkReply::NoError) {
        response = reply->readAll();
    }
    return m_errorFlag;
}



int RequestHandler::postRequest( QByteArray& response, const QByteArray& data, const QUrl& url )
{
    /*NOTE: using the same variable for error handling as getRequest.
     * (If we want to implement asynchronous calls for get/post-Request, use
     * a new variable and error handling function (slot) for each request-function.)
     * Also use a copy of data for asynchronous calls!
     */
    m_loginFailed = false;
    m_errorFlag = QNetworkReply::NoError;
    QNetworkRequest request( url );
    QNetworkReply *reply = manager.post( request, data );

    waitForReply(*reply);

    if ( m_errorFlag == QNetworkReply::NoError ) {
        response = reply->readAll();
    }
    return m_errorFlag;
}

void RequestHandler::waitForReply( const QNetworkReply& reply )
{
    QEventLoop loop;
    QObject::connect( &reply, SIGNAL( finished() ), &loop, SLOT( quit() ) );
    QObject::connect( &reply, SIGNAL( error( QNetworkReply::NetworkError ) ), this,
                      SLOT( handleError( QNetworkReply::NetworkError ) ) );
    QObject::connect( &reply, SIGNAL( error( QNetworkReply::NetworkError ) ), &loop,
                      SLOT( quit() ) );
    loop.exec();
}

void RequestHandler::handleError(QNetworkReply::NetworkError code)
{
    m_errorFlag = code;
}


void RequestHandler::authenticate( QNetworkReply* reply, QAuthenticator* authenticator )
{
    if (m_loginFailed) {
        reply->abort();
    } else {
        m_loginFailed = true;
        authenticator->setUser(m_username);
        authenticator->setPassword(m_password);
    }
}





