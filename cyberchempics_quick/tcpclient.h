#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QElapsedTimer>

extern long int globalID;

enum ConnectionState
{
    DISCONNECTED,
    CONNECTED
};

class TcpClient : public QObject
{
    Q_OBJECT
public:
    TcpClient(QString, quint16);
    ~TcpClient();
    void SetHost(QString, quint16);

private:
    QString host;
    quint16 port;
    QTcpSocket* client;
    ConnectionState state;
    QTimer* timer;
    quint16 m_nNextBlockSize;
    quint16 server_status;
    QElapsedTimer timer_elapsed;

    void sendID();

signals:
    void signalAppendText(QString str, quint16 _server_stat);
    void signalDataReceived(QString, quint16);

private slots:
    void slotTryConnect();

private slots:
    bool slotWriteToServer(QByteArray);
    void slotConnected();
    void slotDisconnectd();
    void slotReadyRead();
};

#endif // TCPCLIENT_H
