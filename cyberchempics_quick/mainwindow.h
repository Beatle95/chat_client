#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMetaObject>
#include <QObject>
#include <QHostAddress>
#include <QFile>
#include <QDataStream>
#include <QTcpSocket>
#include <QByteArray>
#include <QThread>
#include <QTime>
#include <QFileInfo>
#include <QDialog>
#include <QWidget>
#include <QMessageBox>
#include <QFileDialog>
#include <QSound>
#include "filemanager.h"
#include "tcpclient.h"
#include "chatbuffer.h"
#include "models.h"
#include "additionsfilesdialog.h"
#include "changepassworddialog.h"
#include "addtaskdialog.h"

extern QString globalNickname;
extern long int globalID;
extern int globalBoss;
extern QString server_ip;
extern UserList userList;
extern UserList allUserList;
extern TaskList taskList;
extern QString globalFilesDirectory;

class MainWindow : public QObject
{
    Q_OBJECT
public:
    MainWindow(QObject *parent, QObject *loginR, QObject *registerR, QObject *optionsR, QObject *tasksR);
    ~MainWindow();
    void optionsMiss();
    //Функции логина
    void save_logon(QString l, QString p);
    long int login(QString l, QString p);
    bool check_logon(QString *login, QString *password);
    void launchMainWnd();

private:
    bool logged = false;
    bool newMessage = false;
    QSound* new_message_sound;
    QObject *mainWindowRoot;
    QObject *loginRoot;
    QObject *registerRoot;
    QObject *optionsRoot;
    QObject *tasksRoot;
    QObject *progBarDownloadFile;
    QObject *trayIcon;
    QThread* fileManagerThread;
    QTimer* tasksCountTimer;
    QTimer* newMessageTimer;
    FileManager* file_manager;
    TcpClient* tcp_client;
    QThread* chatBufferThread;
    ChatBuffer* chat_buffer;

    //Функции подготовки содержимого окон
    void clearRegisterForm();
    void fillEditsOptionsForm();

    int register_user();
    inline void addUser(UserItem user);
    inline void requestTasksCount();
    void newMessageTimerCallback();

signals:
    void signalStopFileManager();
    void signalFTPSendFile(QString _path, QString _normName, quint64 _file_size);
    void signalFTPChangeHost(QString h, quint16 p);
    void signalAddMessage(QString, quint16);
    void StopChatBuffer();
    void signalStartFileListProcessing();
    void signalReceiveFile(QString id, QString checkedDest);

public slots:
    void slotAppendText(QString msg, quint16 status);
    void slotButtonClick(const QString msg);
    void slotSetProgressBarVisibility(bool visibility, bool error);
    void slotSetProgressBarValue(int value);
    void slotAddUsers(QVector<UserItem> users, bool clear = false);
    void slotMessageSent(const QString msg);
    void slotFileChosen(const QString path);
    void slotTaskButtonClick(const QString msg, const QString id);
    void slotServerAnswerTasks(QString ID, QString operation, bool error);
    void slotAddUsersTaskWnd(QVector<UserItem> users);
    void slotAddTaskToView(TaskItem task);
    void slotSetPogressBarValue(int value);
    void slotFileReceived(bool error);
};

#endif // MAINWINDOW_H
