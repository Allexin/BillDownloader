#ifndef CNETCLIENT_H
#define CNETCLIENT_H

#include <QObject>
#include <QString>
#include <QSslSocket>

enum eNetWorkError{
    NE_ENCRYPTED_FAIL,
    NE_SSL_ERROR,
    NE_CONNECTION_LOST,
    NE_RESPONSE_INCORRECT_HTTP_HEADER,
    NE_RESPONSE_NOT_OK,
    NE_RESPONSE_NO_CONTENT,
    NE_RESPONSE_CONTENT_EMPTY,
    NE_RESPONSE_NO_DATA,
    NE_RESPONSE_INCORRECT_BODY
};

class cNetClient : public QObject
{
    Q_OBJECT
protected:
    QSslSocket          m_Socket;
    QString             m_Data;
public:
    explicit cNetClient(QObject *parent = 0);

signals:
    void error(QString error, int errorCode);
    void processResponse(const QString& json);
private slots:
    void processSslErrors( const QList<QSslError> & errors );
    void processDisconnected();
    void processEncrypted();
    void processReadyRead();
    void processModeChanged(QSslSocket::SslMode);
    void processAboutClose();
        void processStateChanged(QAbstractSocket::SocketState);
public slots:
    void sendMessage(const QString& host, int port, const QString& URL, const QString& userPhone, const QString& userPassword);\
};

#endif // CNETCLIENT_H
