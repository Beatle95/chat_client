#include "filemanager.h"

FileManager::FileManager(QString h, quint16 p)
{
    host = h;
    port = p;
}

FileManager::~FileManager()
{
    files_mtx.tryLock(0);
    files_mtx.unlock();
}

//Обрабатываем поступившие данные о файлах
void FileManager::parseFilesString(QString &str)
{
    files.clear();
    QStringList list = str.split("|", Qt::SkipEmptyParts);
    for(int i = 0; i < list.size(); i+=4){
        if( (i+3) > (list.size()-1) ){
            return;
        }
        files.append( {list[i], list[i+1], list[i+2], list[i+3]} );
    }
}

bool FileManager::getPacket(QTcpSocket *ftp_socket, QByteArray &arr, quint16 &len, quint16 &status)
{
    arr.clear();
    len = 0;
    status = 0;
    for(int i = 0; i < 3000; i++) //Делаем так чтобы подождать 3 секунды маленькими задержками
    {
        if(ftp_socket->bytesAvailable() < 2)
        {
            QThread::msleep(1);
            QCoreApplication::processEvents();
        }
        else
        {
            qDebug() << i;
            break;
        }
    }
    if(ftp_socket->bytesAvailable() < 2)
    {   //Не дождались
        return false;
    }
    QByteArray ret = ftp_socket->read(2);
    len = ((unsigned short)(unsigned char)ret[0] << 8) | (unsigned short)(unsigned char)ret[1];
    int m_waiting = 0;
    while(ftp_socket->bytesAvailable() < len)
    {
        QThread::msleep(1);
        QCoreApplication::processEvents();
        m_waiting++;
        if(m_waiting > 3000)
            break;
    }
    if(m_waiting > 3000)
    {
        return false;
    }
    ret = ftp_socket->read(2);
    status = ((unsigned short)(unsigned char)ret[0] << 8) | (unsigned short)(unsigned char)ret[1];
    if(len > 2)
        arr = ftp_socket->read(len-2);
    return true;
}

bool FileManager::createEmptyFile(QFile &file, QString fileName)
{
    QString tmpFilePath = filePath + "/" + fileName;
    if(!QFile::exists(tmpFilePath))    //Если файл не существует
    {
        file.setFileName(tmpFilePath);
        file.open(QIODevice::WriteOnly);
        filePath = tmpFilePath;
        return true;
    }
    else
    {
        QString fileMainPart, fileExt;
        quint16 digit = 1;
        QStringList list = fileName.split(".", Qt::SkipEmptyParts);
        if(list.size() == 1)
        {
            fileMainPart = fileName;
            fileExt = "";
        }
        else if(list.size() == 2)
        {
            fileMainPart = list[0];
            fileExt = list[1];
        }
        else if(list.size() > 2)
        {
            for(int i = 0; i < list.size()-1; i++)
            {
                fileMainPart += list[i];
            }
            fileExt = list[list.size()-1];
        }
        //Пытаемся создать уникальное имя для файла
        for(; digit < 65535; digit++)
        {
            tmpFilePath = QString("%1/%2(%3).%4").arg(filePath, fileMainPart, QString::number(digit), fileExt);
            if(!QFile::exists(tmpFilePath))
            {
                file.setFileName(tmpFilePath);
                file.open(QIODevice::WriteOnly);
                filePath = tmpFilePath;
                return true;
            }
        }
    }
    return false;
}

void FileManager::Run()
{
    QTcpSocket* ftp_socket = new QTcpSocket(this);

    while(_run)
    {
        QCoreApplication::processEvents();
        if(ftp_status == FTP_Status::HAVE_UPLOAD)
        {
            ftp_status = FTP_Status::UPLOADING;
            emit signalSetProgressBarValue(0);
            emit signalSetProgressBarVisibility(true);
            ftp_socket->connectToHost(host, port);
            if(!ftp_socket->waitForConnected(5000))
            {
                ftp_status = FTP_Status::FTP_READY;
                ftp_socket->close();
                emit signalSetProgressBarVisibility(false, true);
                continue;
            }
            //Соединение установлено
            QFile file(filePath);
            if(!file.open(QIODevice::ReadOnly | QIODevice::ExistingOnly))
            {
                ftp_status = FTP_Status::FTP_READY;
                ftp_socket->close();
                emit signalSetProgressBarVisibility(false, true);
                continue;
            }
            QByteArray byteFileName = fileName.toUtf8();
            QByteArray arrBlock(64 + byteFileName.size(), 0);
            quint16 size = byteFileName.size() + 62;
            quint16 status = 110;

            arrBlock[1] = *((char*)&size);
            arrBlock[0] = *(((char*)&size)+1);
            arrBlock[3] = *((char*)&status);
            arrBlock[2] = *(((char*)&status)+1);
            QByteArray byteID = QString::number(globalID).toUtf8();
            int tmp_c = 4;
            for(int i = 0; i < byteID.size(); i++)
            {
                arrBlock[tmp_c] = byteID[i];
                ++tmp_c;
            }
            tmp_c = 64;
            for(int i = 0; i < byteFileName.size(); i++)
            {
                arrBlock[tmp_c] = byteFileName[i];
                ++tmp_c;
            }
            //Отправляем первое сообщение с именем файла
            ftp_socket->write(arrBlock);
            if(!ftp_socket->waitForBytesWritten(5000))
            {
                ftp_status = FTP_Status::FTP_READY;
                ftp_socket->close();
                emit signalSetProgressBarVisibility(false, true);
                continue;
            }
            //Начинаем отправлять файл частями по 4000 байт
            quint64 file_parts = file_size / 4000;
            int last_part_size = file_size % 4000;
            double part_value = 0;
            if(file_parts > 0)
                part_value = 100000 / file_parts;
            else part_value = 100000;
            double value = 0;
            quint32 was_read = 0;
            QByteArray data = file.readAll();
            size = 4002;
            status = 111;
            arrBlock.resize(4004);
            for(quint64 i = 0; i < file_parts; i++)
            {
                if(ftp_socket->state() != QAbstractSocket::ConnectedState)
                {
                    ftp_status = FTP_Status::FTP_READY;
                    break;
                }
                arrBlock[1] = *((char*)&size);
                arrBlock[0] = *(((char*)&size)+1);
                arrBlock[3] = *((char*)&status);
                arrBlock[2] = *(((char*)&status)+1);
                for(int j = 4; j < 4004; j++)
                {
                    arrBlock[j] = data[was_read];
                    ++was_read;
                }
                ftp_socket->write(arrBlock);
                if(!ftp_socket->waitForBytesWritten(5000))
                {
                    ftp_status = FTP_Status::FTP_READY;
                    break;
                }
                value += part_value;
                emit signalSetProgressBarValue((int)value);
            }
            //Проверяем не произошел ли краш
            if(ftp_status == FTP_Status::FTP_READY)
            {
                ftp_socket->close();
                emit signalSetProgressBarVisibility(false, true);
                continue;
            }
            //Отправляем последний блок
            size = last_part_size + 2;
            status = 112;
            arrBlock.resize(last_part_size+4);
            arrBlock[1] = *((char*)&size);
            arrBlock[0] = *(((char*)&size)+1);
            arrBlock[3] = *((char*)&status);
            arrBlock[2] = *(((char*)&status)+1);
            for(int j = 4; j < last_part_size + 4; j++)
            {
                arrBlock[j] = data[was_read];
                ++was_read;
            }
            ftp_socket->write(arrBlock);
            if(!ftp_socket->waitForBytesWritten(5000))
            {
                ftp_status = FTP_Status::FTP_READY;
                ftp_socket->close();
                emit signalSetProgressBarVisibility(false, true);
                continue;
            }
            ftp_status = FTP_Status::FTP_READY;
            ftp_socket->close();            
            emit signalSetProgressBarVisibility(false);
        }

        else if(ftp_status == FTP_Status::GET_FILES_LIST)
        {
            files_mtx.lock();
            //проверяем нужно ли нам обновить список файлов
            ftp_socket->connectToHost(host, port);
            if(!ftp_socket->waitForConnected(5000))
            {
                ftp_status = FTP_Status::FTP_READY;
                ftp_socket->close();
                files_mtx.unlock();
                continue;
            }
            //Соединение установлено
            QByteArray arrFileId;
            if(files.size() > 0){
                arrFileId = files[0][0].toUtf8();
            }
            else{   //Если пусто, запрашивае новый список, путем передачи "0" в качестве ID
                arrFileId = QString("0").toUtf8();
            }
            QByteArray arrBlock(4 + arrFileId.size(), (char)0);
            quint16 len = 2 + arrFileId.size();
            quint16 status = 117;
            arrBlock[1] = *((char*)&len);
            arrBlock[0] = *(((char*)&len)+1);
            arrBlock[3] = *((char*)&status);
            arrBlock[2] = *(((char*)&status)+1);
            for(int i = 0; i < arrFileId.size(); i++)
            {
                arrBlock[i+4] = arrFileId[i];
            }
            //Отправляем первое сообщение с id последнего имеющегося элемента
            ftp_socket->write(arrBlock);
            if(!ftp_socket->waitForBytesWritten(5000))
            {
                ftp_status = FTP_Status::FTP_READY;
                ftp_socket->close();
                files_mtx.unlock();
                continue;
            }
            //Получаем ответ
            QByteArray ret;
            if(!getPacket(ftp_socket, ret, len, status))
            {
                ftp_status = FTP_Status::FTP_READY;
                ftp_socket->close();
                files_mtx.unlock();
                continue;
            }
            if(status == 118)
            {   //Если список не нужно обновлять
                ftp_socket->close();
                emit signalFileListProcessed();
            }
            else if(status == 119)
            {
                //Список нужно обновить, готовимся к приему данных
                quint16 len = 0, status = 0;
                if(!getPacket(ftp_socket, ret, len, status))
                {
                    ftp_status = FTP_Status::FTP_READY;
                    ftp_socket->close();
                    files_mtx.unlock();
                    continue;
                }
                ftp_socket->close();
                QString str(ret);
                parseFilesString(str);
                emit signalFileListProcessed(); //Данный сигнал сообщает о том, что данные были получены и позволяет отобразить список пользователей
            }
            else
            {   //Если получили хрень
                ftp_socket->close();
            }
            files_mtx.unlock();
        }

        else if(ftp_status == FTP_Status::HAVE_DOWNLOAD)    //Получили сигнал о необходимости загрузить файл
        {
            ftp_status = FTP_Status::DOWNLOADING;
            quint16 len, status;
            //Устанавливаем соединение и запрашиваем файл
            ftp_socket->connectToHost(host, port);
            if(!ftp_socket->waitForConnected(5000))
            {
                ftp_status = FTP_Status::FTP_READY;
                ftp_socket->close();
                emit signalFileReceived(true);
                continue;
            }
            QByteArray byteID = fileID.toUtf8();
            QByteArray arrBlock(4 + byteID.size(), 0);
            quint16 size = byteID.size() + 2;
            status = 125;

            arrBlock[1] = *((char*)&size);
            arrBlock[0] = *(((char*)&size)+1);
            arrBlock[3] = *((char*)&status);
            arrBlock[2] = *(((char*)&status)+1);
            int tmp_c = 4;
            for(int i = 0; i < byteID.size(); i++)
            {
                arrBlock[tmp_c] = byteID[i];
                ++tmp_c;
            }
            ftp_socket->write(arrBlock);
            if(!ftp_socket->waitForBytesWritten(5000))
            {
                ftp_status = FTP_Status::FTP_READY;
                ftp_socket->close();
                emit signalFileReceived(true);
                continue;
            }
            if(!getPacket(ftp_socket, arrBlock, len, status))
            {
                ftp_status = FTP_Status::FTP_READY;
                ftp_socket->close();
                emit signalFileReceived(true);
                continue;
            }
            if(status != 126)
            {
                ftp_status = FTP_Status::FTP_READY;
                ftp_socket->close();
                emit signalFileReceived(true);
                continue;
            }
            QString str(arrBlock);
            QStringList list = str.split("|", Qt::SkipEmptyParts);
            if(list.size() < 2)
            {
                ftp_status = FTP_Status::FTP_READY;
                ftp_socket->close();
                emit signalFileReceived(true);
                continue;
            }
            file_size = QString(list[1]).toULongLong();
            //Попытка создания файла
            QFile fileToWrite;
            if(!createEmptyFile(fileToWrite, list[0]))
            {
                ftp_status = FTP_Status::FTP_READY;
                ftp_socket->close();
                emit signalFileReceived(true);
                continue;
            }
            quint32 bytesReceived = 0;
            QByteArray arrFile(file_size, 0);
            double progBarAddVal = (10000.0 / ((double)file_size / 4000.0));
            double value = 0;
            while(true)
            {
                if(!getPacket(ftp_socket, arrBlock, len, status))
                {
                    ftp_socket->close();
                    emit signalFileReceived(true);
                    break;
                }
                len -= 2;
                if(status != 127)
                {
                    ftp_socket->close();
                    emit signalFileReceived(true);
                    break;
                }
                //Получили порцию данных
                for(int i = 0; i < len; i++)
                {
                    arrFile[bytesReceived] = arrBlock[i];
                    ++bytesReceived;
                }
                //Продвигаем progress bar
                value += progBarAddVal;
                emit signalSetFilesListProgressBarValue((int)value);
                if(bytesReceived >= file_size)
                {
                    ftp_socket->close();
                    fileToWrite.write(arrFile);
                    fileToWrite.close();
                    emit signalFileReceived(false);
                    break;
                }
            }
        }
        ftp_status = FTP_Status::FTP_READY;
        QThread::msleep(5);
    }
}

void FileManager::slotChangeHost(QString h, quint16 p)
{
    host = h;
    port = p;
}

void FileManager::slotSendFile(QString _path, QString _normName, quint64 _file_size)
{
    if(ftp_status == FTP_Status::FTP_READY)
    {
        ftp_status = FTP_Status::HAVE_UPLOAD;
        filePath = _path;
        fileName = _normName;
        file_size = _file_size;
    }
}

void FileManager::slotStop()
{
    _run = false;
}

void FileManager::slotStartFileListProcessing()
{
    if(ftp_status == FTP_Status::FTP_READY)
    {
        ftp_status = GET_FILES_LIST;
    }
}

void FileManager::slotReciveFile(QString id, QString dest)
{
    if(ftp_status == FTP_Status::FTP_READY)
    {
        ftp_status = FTP_Status::HAVE_DOWNLOAD;
        filePath = dest;
        fileID = id;
    }
}
