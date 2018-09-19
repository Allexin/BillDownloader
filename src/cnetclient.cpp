#include "cnetclient.h"
#include <QDebug>
#include <QCryptographicHash>
#include <QUuid>

cNetClient::cNetClient(QObject *parent) : QObject(parent)
{
    m_Socket.setProtocol(QSsl::AnyProtocol);

    QObject::connect(&m_Socket, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(processSslErrors(QList<QSslError>)));
    QObject::connect(&m_Socket, SIGNAL(disconnected()), this, SLOT(processDisconnected()));
    QObject::connect(&m_Socket, SIGNAL(encrypted()), this, SLOT(processEncrypted()));
    QObject::connect(&m_Socket, SIGNAL(readyRead()), this, SLOT(processReadyRead()));
    QObject::connect(&m_Socket, SIGNAL(modeChanged(QSslSocket::SslMode)), this, SLOT(processModeChanged(QSslSocket::SslMode)));
    QObject::connect(&m_Socket, SIGNAL(aboutToClose()), this, SLOT(processAboutClose()));
    QObject::connect(&m_Socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(processStateChanged(QAbstractSocket::SocketState)));
}

QString encodeBase64( QString data )
{
 QByteArray text;
 text.append(data);
 return text.toBase64();
}

void cNetClient::sendMessage(const QString& host, int port, const QString& URL, const QString& userPhone, const QString& userPassword)
{
    m_Data = "";



    QString POST;
    POST  = "GET "+URL+" HTTP/1.1\r\n";
    POST += "Host: "+host+":"+QString::number(port)+"\r\n";
    POST += "Authorization: Basic "+encodeBase64(userPhone+":"+userPassword)+"\r\n";
    POST += "Device-Id: "+QUuid::createUuid().toString()+"\r\n";
    POST += "Device-OS: "+QString("Adnroid 5.1")+"\r\n";
    POST += "Version: "+QString("2")+"\r\n";
    POST += "ClientVersion: "+QString("1.4.4.1")+"\r\n";
    POST += "Connection: "+QString("Keep-Alive")+"\r\n";
    POST += "Accept-Encoding: "+QString("gzip")+"\r\n";
    POST += "User-Agent: "+QString("okhttp/3.0.1")+"\r\n";

    m_Socket.connectToHostEncrypted(host,port);
    if (!m_Socket.waitForEncrypted(10000)){
        emit error(m_Socket.errorString(),NE_ENCRYPTED_FAIL);
    }

    /*m_Socket.connectToHost(host,port);
    if (!m_Socket.waitForConnected(10000)){
        emit error(m_Socket.errorString(),NE_ENCRYPTED_FAIL);
    }*/
    m_Socket.write(POST.toUtf8());
}

void cNetClient::processDisconnected()
{
    qWarning() << "NETCLIENT: " << " disconnected";
    //проверить данные, если их нет - вызвать ошибку. Если есть проверить корректность и действовать соответственно
    if (m_Data.isEmpty()){
        emit error("connection lost",NE_CONNECTION_LOST);
        return;
    }

    int pos = m_Data.indexOf("HTTP/1.1");
    if (pos==-1)
        pos = m_Data.indexOf("HTTP/1.0");
    if (pos==-1){
        emit error("incorrect response(not HTTP1.1 package)",NE_RESPONSE_INCORRECT_HTTP_HEADER);
        return;
    }

    pos = m_Data.indexOf(" ",pos);
    int pos2 = m_Data.indexOf("\r",pos);
    QString Status = m_Data.mid(pos+1,pos2-(pos+1));
    qDebug() << "STATUS " << Status;
    if (Status!="200 OK"){
        emit error("incorrect response status. "+Status,NE_RESPONSE_NOT_OK);
        return;
    }

    pos = m_Data.indexOf("Content-Length:");
    if (pos==-1){
        emit error("incorrect response(no Content-Length field)",NE_RESPONSE_NO_CONTENT);
        return;
    }

    pos = m_Data.indexOf(" ",pos);
    pos2 = m_Data.indexOf("\r",pos);
    int ContentLength= m_Data.mid(pos+1,pos2-(pos+1)).toInt();
    qDebug() << "CONTENT LENGTH " << ContentLength;

    if (ContentLength==0){
        emit error("incorrect respones(Content-Length==0)",NE_RESPONSE_CONTENT_EMPTY);
        return;
    }

    pos = m_Data.indexOf("\r\n\r\n",pos);
    if (pos==-1){
        emit error("incorrect response(data not found)",NE_RESPONSE_NO_DATA);
        return;
    }

    QString json = m_Data.mid(pos+4);
    qDebug() << "PACKAGE \n" << json;


    pos = json.indexOf("\"status\"");
    if (pos==-1){
        emit error("incorrect response(tag status not found)",NE_RESPONSE_INCORRECT_BODY);
        return;
    }

    emit processResponse(json);
}

void cNetClient::processReadyRead()
{
    QString read = m_Socket.readAll().data();
    m_Data += read;
    qWarning() << "NETCLIENT: " << "data ready:\n" << read;

    m_Socket.disconnectFromHost();
}


void cNetClient::processAboutClose()
{
}

void cNetClient::processStateChanged(QAbstractSocket::SocketState )
{
}

void cNetClient::processEncrypted()
{
}

void cNetClient::processModeChanged(QSslSocket::SslMode )
{
}

void cNetClient::processSslErrors(const QList<QSslError> &errors)
{
    qWarning() << "NETCLIENT: " << errors[0];
    qWarning() << "NETCLIENT: " << QSslError::SelfSignedCertificateInChain;
    if (errors[0].error()==QSslError::SelfSignedCertificateInChain){
        qWarning() << "NETCLIENT: " << "ignore ssl errors";
        m_Socket.ignoreSslErrors();
        return;
    }
    QString ErrorsList = "SSL Errors: ";
    for (int i = 0; i<errors.size(); i++)
        if (i==0)
            ErrorsList += errors[i].errorString();
        else
            ErrorsList += ", " +errors[i].errorString();
    qWarning() << "NETCLIENT: " << ErrorsList;
    emit error(ErrorsList,NE_SSL_ERROR);
}
