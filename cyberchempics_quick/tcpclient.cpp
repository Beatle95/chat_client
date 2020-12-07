#include "tcpclient.h"

TcpClient::TcpClient(QString h, quint16 p)
{
    m_nNextBlockSize = 0;
    server_status = 0;

    host = h;
    port = p;
    timer = new QTimer();
    timer->setInterval(1000);
    connect(timer, SIGNAL(timeout()), this, SLOT(slotTryConnect()));

    state = ConnectionState::DISCONNECTED;
    client = new QTcpSocket(this);
    connect(client, SIGNAL(connected()), this, SLOT(slotConnected()), Qt::DirectConnection);
    connect(client, SIGNAL(disconnected()), this, SLOT(slotDisconnectd()), Qt::DirectConnection);
    connect(client, SIGNAL(readyRead()), this, SLOT(slotReadyRead()), Qt::DirectConnection);

    timer->start();
}

void TcpClient::slotTryConnect()
{
    if(state == ConnectionState::DISCONNECTED)
    {
        client->connectToHost(host, port);
    }
}

void TcpClient::slotConnected()
{
    state = ConnectionState::CONNECTED;
    sendID();
    emit signalAppendText("#Соединение с " + client->peerName() + " установлено.", (quint16)0U);
}

void TcpClient::sendID()
{
    QByteArray arrBlock(64, 0);
    quint16 size = 62;
    quint16 status = 8;
    arrBlock[1] = *((char*)&size);
    arrBlock[0] = *(((char*)&size)+1);
    arrBlock[3] = *((char*)&status);
    arrBlock[2] = *(((char*)&status)+1);
    //Добавляем в сообщение ID
    QString strID = QString::number(globalID);
    QByteArray arrID = strID.toUtf8();
    int arrayEnd = arrID.size() < 60 ? arrID.size()+4 : 64;
    for(int i = 4; i < arrayEnd; i++)
    {
        arrBlock[i] = arrID[i-4];
    }
    client->write(arrBlock);
}

void TcpClient::slotDisconnectd()
{
    state = ConnectionState::DISCONNECTED;
    emit signalAppendText("#Соединение разорвано.", (quint16)200U);
}

bool TcpClient::slotWriteToServer(QByteArray arrBlock)
{
    if(state == ConnectionState::DISCONNECTED)  //if disconnected return false
        return false;
    client->write(arrBlock);
    return true;
}

void TcpClient::slotReadyRead()
{
    char block[4096];
    memset(block, 0, 4096);
    for(;;)
    {
        if (!m_nNextBlockSize) {
            if ((quint64)client->bytesAvailable() < sizeof(quint16)) {
                break;
            }
            client->read(block, 2);
            m_nNextBlockSize = ((unsigned short)(unsigned char)block[0] << 8) | (unsigned short)(unsigned char)block[1];
            if(m_nNextBlockSize > 4096) //Битые данные, пропускаем полученный блок
            {
                client->readAll();
                m_nNextBlockSize = 0;
                break;
            }
        }
        if (client->bytesAvailable() < m_nNextBlockSize) {
            if(timer_elapsed.isValid())
            {
                if(timer_elapsed.elapsed() > 6000){
                    timer_elapsed.invalidate();
                    m_nNextBlockSize = 0;
                    client->readAll();
                }
            }
            else
                timer_elapsed.start();
            break;
        }
        client->read(block, m_nNextBlockSize);
        server_status = ((unsigned short)(unsigned char)block[0] << 8) | (unsigned short)(unsigned char)block[1];
        QString str(&block[2]);
        m_nNextBlockSize = 0;
        emit signalDataReceived(str, server_status);
    }
}

void TcpClient::SetHost(QString h, quint16 p)
{
    client->close();
    host = h;
    port = p;
}

TcpClient::~TcpClient()
{
    timer->stop();
    client->close();
    delete timer;
    delete client;
}
