#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QObject>
#include <QTcpSocket>
#include <QThread>
#include <QCoreApplication>
#include <QFileInfo>
#include <QFile>
#include <QString>
#include <QMutex>
#include <QDebug>

extern QString globalNickname;
extern long int globalID;

extern QMutex files_mtx;
extern QVector<QVector<QString>> files;

enum FTP_Status
{
    FTP_READY, HAVE_UPLOAD, UPLOADING, HAVE_DOWNLOAD, DOWNLOADING, GET_FILES_LIST
};

class FileManager : public QObject
{
    Q_OBJECT
public:
    FTP_Status ftp_status = FTP_Status::FTP_READY;
    FileManager(QString h, quint16 p);
    ~FileManager();

private:
    bool _run = true;
    quint64 file_size;
    QString filePath;
    QString fileName;
    QString fileID;
    QString host;
    quint16 port;

    void parseFilesString(QString &str);
    bool getPacket(QTcpSocket *ftp_socket, QByteArray &arr, quint16 &len, quint16 &status);
    bool createEmptyFile(QFile &file, QString fileName);

public slots:
    void Run();
    void slotSendFile(QString _path, QString _normName, quint64 _file_size);
    void slotChangeHost(QString h, quint16 p);
    void slotStop();
    void slotStartFileListProcessing();
    void slotReciveFile(QString id, QString dest);

signals:
    void signalSetProgressBarVisibility(bool visibility, bool error = false);
    void signalSetProgressBarValue(int value);
    void signalSetFilesListProgressBarValue(int value);
    void signalFileListProcessed();
    void signalFileReceived(bool error);
};

#endif // FILEMANAGER_H
