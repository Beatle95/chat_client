#ifndef CHATBUFFER_H
#define CHATBUFFER_H

#include <QObject>
#include <QTextCodec>
#include <QMutex>
#include <QEventLoop>
#include <QThread>
#include <QDataStream>
#include <QElapsedTimer>
#include <QString>
#include <QStringList>
#include "models.h"

extern long int globalID;
extern QString globalNickname;
extern UserList allUserList;

enum MessageState
{
    READY, SENT
};

struct CircleBuffer
{
    quint16 status;
    char buf[4096];
    quint16 size;
    CircleBuffer* next;
};

class ChatBuffer : public QObject
{
    Q_OBJECT
public:
    ChatBuffer();
    ~ChatBuffer();

private:
    bool isTasksCountRequested = false;
    quint16 status;
    QString lastMessage;
    bool _run;
    QMutex mutex;
    CircleBuffer buffer[10];
    CircleBuffer* first;
    CircleBuffer* last;
    int message_count;
    MessageState message_state;
    quint16 waitForMessage;
    QElapsedTimer timer_elapsed;

    //Methods
    void inline BufferInit();
    QByteArray PullMessage();
    bool inline isMessageReady();
    inline void buffer_copy(char* dest, QByteArray *source);
    inline void MoveMessageQueue();
    QVector<UserItem> processUsersString(QString *str);
    QString processFileBroadcast(QString *str);
    void user_added(QString str);
    void user_left(QString str);
    void processUsers(QString &str);
    void processTasks(QString &str);

public slots:
    void Run();
    void Stop();
    bool AddMessage(QString, quint16);

private slots:
    void slotDataReceived(QString,quint16);

signals:
    bool signalSendMessage(QByteArray);
    void signalAppendText(QString str, quint16 _server_stat);
    void signalAddUsers(QVector<UserItem> users, bool clear = false);
    void signalPasswordChanged(bool error = false);
    void signalServerAnswerTasks(QString ID, QString operation, bool error = false);
    void signalAddUsersTasksWnd(QVector<UserItem>);
    void signalAddTaskToView(TaskItem task);
};

#endif // CHATBUFFER_H
