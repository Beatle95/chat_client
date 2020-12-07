#include "mainwindow.h"

MainWindow::MainWindow(QObject *parent, QObject *loginR, QObject *registerR, QObject *optionsR, QObject *tasksR) : QObject(parent)
{
    mainWindowRoot = parent;
    loginRoot = loginR;
    registerRoot = registerR;
    optionsRoot = optionsR;
    tasksRoot = tasksR;
    tasksCountTimer = new QTimer();
    connect(tasksCountTimer, &QTimer::timeout, this, &MainWindow::requestTasksCount);
    tasksCountTimer->start(50000);
    new_message_sound = new QSound("qrc:/sound/message.wav");
    newMessageTimer = new QTimer();
    connect(newMessageTimer, &QTimer::timeout, this, &MainWindow::newMessageTimerCallback);
}

void MainWindow::slotButtonClick(const QString msg)
{
    //Обработчики событий нажатия не самых часто нажимаемых клавишь
    if(msg == "reg")
    {
        clearRegisterForm();
        loginRoot->setProperty("visible", false);
        registerRoot->setProperty("visible", true);
    }
    else if(msg == "login")
    {
        logged = false;
        QObject* loginDialog = loginRoot->findChild<QObject*>("loginDialog");
        QString _login = loginRoot->findChild<QObject*>("tfLogin")->property("text").toString();
        QString _password = loginRoot->findChild<QObject*>("tfPassword")->property("text").toString();
        if(_login.length() < 1)
        {
            loginDialog->setProperty("text", "Введите логин.");
            QMetaObject::invokeMethod(loginDialog, "open");
            return;
        }
        if(_password.length() < 1)
        {
            loginDialog->setProperty("text", "Введите пароль.");
            QMetaObject::invokeMethod(loginDialog, "open");
            return;
        }
        long int i = login(_login, _password);
        if(i > 0)
        {
            //логин успешен
            globalID = i;
            logged = true;
        }
        else if(i == -3)
        {
            loginDialog->setProperty("text", "Сервер не доступен.");
            QMetaObject::invokeMethod(loginDialog, "open");
            return;
        }
        else
        {
            loginDialog->setProperty("text", "Неверные логин и/или пароль.");
            QMetaObject::invokeMethod(loginDialog, "open");
            return;
        }
        if(loginRoot->findChild<QObject*>("cbRemember")->property("checked").toString() == "true")
        {   //Если запомнить пароль выбрана, то сохраняем
            save_logon(_login, _password);
        }
        else
        {   //Иначе очищаем логон
            QFile logon_file("logon");
            logon_file.open(QIODevice::ReadOnly | QIODevice::Text | QIODevice::Truncate);
            logon_file.close();
        }
        if(logged)
        {
            launchMainWnd();
        }
    }
    else if(msg == "cancelReg")
    {
        loginRoot->setProperty("visible", true);
        registerRoot->setProperty("visible", false);
    }
    else if(msg == "loginOpt")
    {
        loginRoot->setProperty("visible", false);
        optionsRoot->setProperty("visible", true);
        optionsRoot->findChild<QObject*>("lNickname")->setProperty("visible", false);
        optionsRoot->findChild<QObject*>("tfNickname")->setProperty("visible", false);
        optionsRoot->findChild<QObject*>("bSaveNickname")->setProperty("visible", false);
        optionsRoot->findChild<QObject*>("bChangePassword")->setProperty("visible", false);
        fillEditsOptionsForm();
    }
    else if(msg == "mainOptions")
    {
        optionsRoot->setProperty("visible", true);
        optionsRoot->findChild<QObject*>("lNickname")->setProperty("visible", true);
        optionsRoot->findChild<QObject*>("tfNickname")->setProperty("visible", true);
        optionsRoot->findChild<QObject*>("bSaveNickname")->setProperty("visible", true);
        optionsRoot->findChild<QObject*>("bChangePassword")->setProperty("visible", true);
        fillEditsOptionsForm();
    }
    else if(msg == "optChangeNickname")
    {
        QString nickname = optionsRoot->findChild<QObject*>("tfNickname")->property("text").toString();
        if(nickname.size() < 3 || nickname.size() > 30)
        {
            QMessageBox::warning(nullptr, tr("Предупреждение"), tr("Псевдоним должен быть длиннее 3 и меньше 30 символов."), QMessageBox::Ok);
            return;
        }
        if(nickname.indexOf("|") > -1 || nickname.indexOf(",") > -1 || nickname.indexOf(";") > -1)
        {
            QMessageBox::warning(nullptr, tr("Предупреждение"), tr("Псевдоним содержит недопустимые символы!"), QMessageBox::Ok);
            return;
        }
        emit signalAddMessage(nickname, 12);
        QMessageBox::warning(nullptr, tr("Изменение псевдонима"), tr("Запрос на изменение передан на сервер."), QMessageBox::Ok);
        globalNickname = nickname;
    }
    else if(msg == "cancelMainOpt")
    {
        optionsRoot->setProperty("visible", false);
    }
    else if(msg == "cancelOpt")
    {
        loginRoot->setProperty("visible", true);
        optionsRoot->setProperty("visible", false);
    }
    else if(msg == "saveFromLoginOpt")  //Была нажата клавиша сохранить из меню логина
    {
        QString addr = (optionsRoot->findChild<QObject*>("tfIP"))->property("text").toString();
        QString folder = (optionsRoot->findChild<QObject*>("tfFilesDirectory"))->property("text").toString();
        QHostAddress address(addr);
        if(address.protocol() != QAbstractSocket::IPv4Protocol)
        {
            QObject* optionsDialog = optionsRoot->findChild<QObject*>("optionsDialog");
            optionsDialog->setProperty("text", "Указан неверный IP адрес.");
            QMetaObject::invokeMethod(optionsDialog, "open");
            return;
        }
        QFile opt_f("options.opt");
        if(!opt_f.open(QIODevice::WriteOnly | QIODevice::Truncate))
            return;      //no such file or directory
        QDataStream outputStream(&opt_f);
        server_ip = addr;
        globalFilesDirectory = folder;
        outputStream << server_ip << folder;
        opt_f.close();
        loginRoot->setProperty("visible", true);
        optionsRoot->setProperty("visible", false);
    }
    else if(msg == "register")  //Регистрация нового пользователя в системе
    {
        QObject* msgDialog = registerRoot->findChild<QObject*>("msgDialog");
        QString login = registerRoot->findChild<QObject*>("tfLogin")->property("text").toString();
        QString nickname = registerRoot->findChild<QObject*>("tfNickname")->property("text").toString();
        QString password = registerRoot->findChild<QObject*>("tfPassword")->property("text").toString();
        QString passwordConfirm = registerRoot->findChild<QObject*>("tfPasswordConfirm")->property("text").toString();
        if(login.length() > 30 || login.length() < 3 || nickname.length() > 30 || nickname.length() < 3)
        {
            msgDialog->setProperty("text", "Лолгин и псевдоним должны быть больше 3 символов и менее 30.");
            QMetaObject::invokeMethod(msgDialog, "open");
            return;
        }
        if(password.length() < 5 || password.length() > 30)
        {
            msgDialog->setProperty("text", "Пароль должен быть быть больше 4 символов и менее 30 символов.");
            QMetaObject::invokeMethod(msgDialog, "open");
            return;
        }
        if(password.indexOf(",") > -1 || password.indexOf("|") > -1 || password.indexOf("/") > -1 || password.indexOf("\\") > -1 || password.indexOf("\'") > -1 || password.indexOf("\"") > -1)
        {
            msgDialog->setProperty("text", "В пароле имеются недопустимые символы.");
            QMetaObject::invokeMethod(msgDialog, "open");
            return;
        }
        if(password != passwordConfirm)
        {
            msgDialog->setProperty("text", "Пароли не совпадают!");
            QMetaObject::invokeMethod(msgDialog, "open");
            return;
        }
        globalID = register_user();
        if(globalID > 0)
        {
            loginRoot->setProperty("visible", true);
            registerRoot->setProperty("visible", false);
        }
        else
        {
            msgDialog->setProperty("text", "Регистрация не удалась!");
            QMetaObject::invokeMethod(msgDialog, "open");
            return;
        }
    }
    else if(msg == "files additions")
    {
        AdditionsFilesDialog additions;
        connect(file_manager, SIGNAL(signalFileListProcessed()), &additions, SLOT(slotFileListProcessed()));
        connect(&additions, SIGNAL(signalReceiveFile(QString, QString)), file_manager, SLOT(slotReciveFile(QString, QString)));
        connect(file_manager, SIGNAL(signalFileReceived(bool)), &additions, SLOT(slotFileReceived(bool)));
        connect(file_manager, SIGNAL(signalSetFilesListProgressBarValue(int)), &additions, SLOT(slotSetPogressBarValue(int)));
        emit signalStartFileListProcessing(); //вызываем обновление
        additions.setFixedSize(500, 600);
        additions.exec();
    }
    else if(msg == "logout")
    {
        QFile f("logon");
        f.open(QIODevice::Text | QIODevice::WriteOnly | QIODevice::Truncate);
        f.close();
        exit(0);
    }
    else if(msg == "saveFromMainOpt")
    {
        QString addr = (optionsRoot->findChild<QObject*>("tfIP"))->property("text").toString();
        QString folder = (optionsRoot->findChild<QObject*>("tfFilesDirectory"))->property("text").toString();
        QHostAddress address(addr);
        if(address.protocol() != QAbstractSocket::IPv4Protocol)
        {
            QObject* optionsDialog = optionsRoot->findChild<QObject*>("optionsDialog");
            optionsDialog->setProperty("text", "Указан неверный IP адрес.");
            QMetaObject::invokeMethod(optionsDialog, "open");
            return;
        }
        QFile opt_f("options.opt");
        if(!opt_f.open(QIODevice::WriteOnly | QIODevice::Truncate))
            return;      //no such file or directory
        QDataStream outputStream(&opt_f);
        server_ip = addr;
        globalFilesDirectory = folder;
        outputStream << server_ip << folder;
        opt_f.close();
        optionsRoot->setProperty("visible", false);
    }
    else if(msg == "about")
    {
        QMessageBox::about(nullptr, "О программе", "Разработчик: Тыщенко Владислав.\nКонтактный e-mail: tishenko.vlad.apc@gmail.com.\nПрограмма разработана в 2020 году.\nИспользуется на основе условий бесплатного лицензионного соглашения Qt.\nНе предназначена для коммерческого использования.");
    }
    else if(msg == "bChangePassword")
    {
        //Смена пароля
        ChangePasswordDialog passDialog;
        passDialog.setFixedSize(320, 240);
        connect(&passDialog, SIGNAL(signalAddMessage(QString, quint16)), chat_buffer, SLOT(AddMessage(QString,quint16)));
        connect(chat_buffer, SIGNAL(signalPasswordChanged(bool)), &passDialog, SLOT(slotPasswordChanged(bool)));
        passDialog.exec();
    }
    else if(msg == "tasks")
    {
        if(globalBoss > 0)
        {
            QObject* paneUsers = tasksRoot->findChild<QObject*>("paneUsersTasks");
            paneUsers->setProperty("visible", QVariant(true));
            allUserList.removeAll();
            taskList.removeAll();
            tasksRoot->setProperty("visible", true);
            emit signalAddMessage("", quint16(25)); //Запрос списка пользователей
        }
        else
        {
            QObject* paneUsers = tasksRoot->findChild<QObject*>("paneUsersTasks");
            paneUsers->setProperty("visible", QVariant(false));
            allUserList.removeAll();
            taskList.removeAll();
            tasksRoot->setProperty("visible", true);
            emit signalAddMessage("", quint16(21)); //Запрос списка активных задач пользователя
        }
    }
    else if(msg == "tryIcon")
    {
        newMessage = false;
    }
}

void MainWindow::optionsMiss()
{
    loginRoot->setProperty("visible", false);
    optionsRoot->setProperty("visible", true);
    QObject* dialogOptionsMiss = optionsRoot->findChild<QObject*>("dialogOptionsMiss");
    QMetaObject::invokeMethod(dialogOptionsMiss, "open");
    optionsRoot->findChild<QObject*>("lNickname")->setProperty("visible", false);
    optionsRoot->findChild<QObject*>("tfNickname")->setProperty("visible", false);
    optionsRoot->findChild<QObject*>("bSaveNickname")->setProperty("visible", false);
    optionsRoot->findChild<QObject*>("bChangePassword")->setProperty("visible", false);
    fillEditsOptionsForm();
}

int MainWindow::register_user()
{
    QTcpSocket sock;
    sock.connectToHost(server_ip, quint16(14141));
    if(!sock.waitForConnected(3000))
        return -3;
    QByteArray arrBlock(184, 0);
    quint16 status = 5;
    quint16 size = (quint16)182;
    arrBlock[1] = *((char*)&size);
    arrBlock[0] = *(((char*)&size)+1);
    arrBlock[3] = *((char*)&status);
    arrBlock[2] = *(((char*)&status)+1);
    QByteArray login = (registerRoot->findChild<QObject*>("tfLogin")->property("text")).toString().toUtf8();
    QByteArray password = (registerRoot->findChild<QObject*>("tfPassword")->property("text")).toString().toUtf8();
    QByteArray nickname = (registerRoot->findChild<QObject*>("tfNickname")->property("text")).toString().toUtf8();
    int c = 0;
    for(int i = 4; i < login.size() + 4; i++)
    {
        arrBlock[i] = login[c];
        ++c;
    }
    c = 0;
    for(int i = 64; i < password.size() + 64; i++)
    {
        arrBlock[i] = password[c];
        ++c;
    }
    c = 0;
    for(int i = 124; i < nickname.size() + 124; i++)
    {
        arrBlock[i] = nickname[c];
        ++c;
    }
    sock.write(arrBlock);               //Отправили данные и ждем ответ
    if(!sock.waitForBytesWritten(3000))
        return -1;
    if(!sock.waitForReadyRead(3000))
        return -1;

    if(sock.bytesAvailable() < 5)
        QThread::msleep(3000);
    if(sock.bytesAvailable() < 5)
        return -1;
    QByteArray ret = sock.readAll();
    sock.close();
    ret = ret.remove(0, 4);
    QString sID = ret;
    long int iID = sID.toLong();
    if(iID > 0)
        return iID;
    return -2;      //-2 преобразование не удалось
}

//Методы для формы логина###########################################################
void MainWindow::save_logon(QString l, QString p)
{
    QFile logon_file("logon");
    if(!logon_file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
        return;      //no such file or directory
    QTextStream out(&logon_file);
    out << l << "\n";
    out << p;
    logon_file.close();
}

long int MainWindow::login(QString l, QString p)
{
    if(l.length() > 30) l.truncate(30);
    if(p.length() > 30) p.truncate(30);
    QTcpSocket sock;
    sock.connectToHost(server_ip, quint16(14141));
    if(!sock.waitForConnected(3000))
        return -3;
    QByteArray arrBlock(124, 0);
    quint16 status = 1;
    quint16 size = (quint16)122;
    arrBlock[1] = *((char*)&size);
    arrBlock[0] = *(((char*)&size)+1);
    arrBlock[3] = *((char*)&status);
    arrBlock[2] = *(((char*)&status)+1);
    QByteArray login = l.toUtf8();
    QByteArray password = p.toUtf8();
    int c = 0;
    for(int i = 4; i < login.size() + 4; i++)
    {
        arrBlock[i] = login[c];
        ++c;
    }
    c = 0;
    for(int i = 64; i < password.size() + 64; i++)
    {
        arrBlock[i] = password[c];
        ++c;
    }
    sock.write(arrBlock);               //Отправили данные и ждем ответ
    if(!sock.waitForBytesWritten(3000))
        return -1;
    if(!sock.waitForReadyRead(3000))
        return -1;

    if(sock.bytesAvailable() < 64)
        QThread::msleep(3000);
    if(sock.bytesAvailable() < 64)
        return -1;
    QByteArray ret = sock.readAll();
    sock.close();
    QString sID = ret.mid(4, 60);
    long int iID = sID.toLong();
    if(iID > 0)
    {
        globalNickname = ret.mid(64, 60);
        globalBoss = (int)ret.at(124);
        return iID;
    }
    return -2;      //-2 преобразование не удалось
}

bool MainWindow::check_logon(QString *login, QString *password)
{
    QFile logon_file("logon");
    if(!logon_file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;      //no such file or directory
    QTextStream in(&logon_file);
    while(!in.atEnd())
    {
        *login = in.readLine();
        *password = in.readLine();
    }
    logon_file.close();
    if(*login == "" || *password == "")
        return false;
    return true;
}
//Методы для формы логина###########################################################

void MainWindow::fillEditsOptionsForm()
{
    optionsRoot->findChild<QObject*>("tfIP")->setProperty("text", server_ip);
    optionsRoot->findChild<QObject*>("tfFilesDirectory")->setProperty("text", globalFilesDirectory);
    optionsRoot->findChild<QObject*>("tfNickname")->setProperty("text", globalNickname);
}

void MainWindow::clearRegisterForm()
{
    QMetaObject::invokeMethod(registerRoot->findChild<QObject*>("tfLogin"), "clear");
    QMetaObject::invokeMethod(registerRoot->findChild<QObject*>("tfNickname"), "clear");
    QMetaObject::invokeMethod(registerRoot->findChild<QObject*>("tfPassword"), "clear");
    QMetaObject::invokeMethod(registerRoot->findChild<QObject*>("tfPasswordConfirm"), "clear");
}

//Запуск основного окна со всеми потоками
void MainWindow::launchMainWnd()
{
    //Поток файлового менеджера
    progBarDownloadFile = tasksRoot->findChild<QObject*>("progBarDownloadFile");
    trayIcon = mainWindowRoot->findChild<QObject*>("trayIcon");
    fileManagerThread = new QThread;
    file_manager = new FileManager(server_ip, quint16(14140));
    file_manager->moveToThread(fileManagerThread);
    connect(fileManagerThread, SIGNAL(started()), file_manager, SLOT(Run()));
    connect(this, SIGNAL(signalStopFileManager()), file_manager, SLOT(slotStop()));
    connect(this, SIGNAL(signalFTPSendFile(QString,QString,quint64)), file_manager, SLOT(slotSendFile(QString,QString,quint64)));
    connect(this, SIGNAL(signalFTPChangeHost(QString,quint16)), file_manager, SLOT(slotChangeHost(QString,quint16)));
    connect(this, SIGNAL(signalStartFileListProcessing()), file_manager, SLOT(slotStartFileListProcessing()));
    connect(file_manager, SIGNAL(signalSetProgressBarValue(int)), this, SLOT(slotSetProgressBarValue(int)));
    connect(file_manager, SIGNAL(signalSetProgressBarVisibility(bool, bool)), this, SLOT(slotSetProgressBarVisibility(bool, bool)));

    //Поток кольцевого буфера
    chatBufferThread = new QThread;
    chat_buffer = new ChatBuffer();
    chat_buffer->moveToThread(chatBufferThread);
    connect(chatBufferThread, SIGNAL(started()), chat_buffer, SLOT(Run()));
    connect(chat_buffer, SIGNAL(signalAppendText(QString,quint16)), this, SLOT(slotAppendText(QString,quint16)));
    connect(chat_buffer, SIGNAL(signalAddUsers(QVector<UserItem>,bool)), this, SLOT(slotAddUsers(QVector<UserItem>,bool)));
    connect(chat_buffer, SIGNAL(signalServerAnswerTasks(QString,QString,bool)), this, SLOT(slotServerAnswerTasks(QString,QString,bool)));
    connect(chat_buffer, SIGNAL(signalAddUsersTasksWnd(QVector<UserItem>)), this, SLOT(slotAddUsersTaskWnd(QVector<UserItem>)));
    connect(chat_buffer, SIGNAL(signalAddTaskToView(TaskItem)), this, SLOT(slotAddTaskToView(TaskItem)));
    connect(this, SIGNAL(StopChatBuffer()), chat_buffer, SLOT(Stop()), Qt::DirectConnection);
    connect(this, SIGNAL(signalAddMessage(QString, quint16)), chat_buffer, SLOT(AddMessage(QString, quint16)));

    //TСР Клиент
    tcp_client = new TcpClient(server_ip, quint16(14141));
    connect(tcp_client, SIGNAL(signalAppendText(QString,quint16)), this, SLOT(slotAppendText(QString,quint16)));

    connect(tcp_client, SIGNAL(signalDataReceived(QString,quint16)), chat_buffer, SLOT(slotDataReceived(QString,quint16)));
    connect(chat_buffer, SIGNAL(signalSendMessage(QByteArray)), tcp_client, SLOT(slotWriteToServer(QByteArray)));
    connect(this, SIGNAL(signalReceiveFile(QString,QString)), file_manager, SLOT(slotReciveFile(QString,QString)));
    connect(file_manager, SIGNAL(signalFileReceived(bool)), this, SLOT(slotFileReceived(bool)));
    connect(file_manager, SIGNAL(signalSetFilesListProgressBarValue(int)), this, SLOT(slotSetPogressBarValue(int)));

    //Запуск фоновых потоков
    fileManagerThread->start();
    chatBufferThread->start();
    //Смена окна
    loginRoot->setProperty("visible", false);
    mainWindowRoot->setProperty("visible", true);

    //Запуск таймера всплывающего сообщения о новых сообщениях (как ни парадоксально)
    newMessageTimer->start(180000);
    //Запрос количества задач
    QTimer::singleShot(2000, this, &MainWindow::requestTasksCount);
}

void MainWindow::slotMessageSent(const QString msg)
{
    emit signalAddMessage(msg, 101);
}

void MainWindow::slotFileChosen(const QString path)
{
    QStringList parts = path.split('/', Qt::SkipEmptyParts);
    QString fileName = parts.last();
    QFileInfo info(path);
    quint64 file_size = info.size();
    if(file_size > 200000000 || info.isDir())
    {
        QObject* msgDialog = mainWindowRoot->findChild<QObject*>("msgDialog");
        msgDialog->setProperty("text", "Можно указать только файл, размером не более 200 мегайбайт.");
        QMetaObject::invokeMethod(msgDialog, "open");
        return;
    }
    if(!info.isReadable())
    {
        QObject* msgDialog = mainWindowRoot->findChild<QObject*>("msgDialog");
        msgDialog->setProperty("text", "Указанный файл не доступен для чтения.");
        QMetaObject::invokeMethod(msgDialog, "open");
        return;
    }
    emit signalFTPSendFile(path, fileName, file_size);
}

void MainWindow::slotTaskButtonClick(const QString msg, const QString id)
{
    if(msg == "makeAdmin")  //Сделать админом
    {
        if(globalID != 1)
        {
            QMessageBox::warning(nullptr, "Ошибка", "Нужно войти под учетной записью администратора (admin).", QMessageBox::Ok);
            return;
        }
        emit signalAddMessage(id, quint16(40));
    }
    else if(msg == "makeWorker")    //Сделать сотрудником
    {
        if(globalID != 1)
        {
            QMessageBox::warning(nullptr, "Ошибка", "Нужно войти под учетной записью администратора (admin).", QMessageBox::Ok);
            return;
        }
        emit signalAddMessage(id, quint16(41));
    }
    else if(msg == "quitButtonTasks")
    {
        requestTasksCount();
    }
    else if(msg == "bActive")   //показать активные
    {
        taskList.removeAll();
        emit signalAddMessage("", quint16(50));
    }
    else if(msg == "bShowCompleted")    //показать выполненные
    {
        taskList.removeAll();
        emit signalAddMessage("", quint16(53));
    }
    else if(msg == "bRemoveAllDone")    //удалить все выполненные
    {
        taskList.removeAll();
        emit signalAddMessage("", quint16(75));
    }
    else if(msg == "markCompleted")
    {
        long int iID = id.toLong();
        if(iID < 1)
            return;
        emit signalAddMessage(QString::number(iID), quint16(70));
    }
    else if(msg == "usersViewSelectionChanged")
    {
        long int iID = id.toLong();
        if(iID < 1)
            return;
        emit signalAddMessage(QString::number(iID), quint16(23));
        taskList.removeAll();
    }
    else if(msg == "addTask")
    {
        long int user_id = id.toLong();
        if(user_id < 1)
            return;
        TaskItem item;
        AddTaskDialog atd(&item);
        item.user_id = id.toLong();
        if(item.user_id < 1)
        {
            QMessageBox::warning(nullptr, "Error", "Internal error.", QMessageBox::Ok);
        }
        emit signalStartFileListProcessing(); //вызываем обновление
        connect(file_manager, SIGNAL(signalFileListProcessed()), &atd, SLOT(slotFileListProcessed()));
        atd.setFixedSize(600, 450);
        atd.exec();
        if(atd.result() == 1)
        {
            QString _user_id = QString::number(item.user_id);
            QString _file_id = QString::number(item.file_id);
            QString message = QString("%1|%2|%3|%4").arg(_user_id, item.title, item.text, _file_id);
            emit signalAddMessage(message, quint16(80));
        }
    }
    else if(msg == "downloadFile")
    {
        long int lID = id.toLong();
        if(lID < 1)
            return;
        QVector<TaskItem> list = taskList.items();
        for(int i = 0; i < list.size(); i++)
        {
            if(list[i].ID == lID)
            {
                //нашли нужный id
                QString dir = QFileDialog::getExistingDirectory(nullptr,
                                                                 tr("Выберите папку"),
                                                                 "С:/",
                                                                 QFileDialog::ShowDirsOnly
                                                                 | QFileDialog::DontResolveSymlinks);
                if(dir.isEmpty())
                    return;
                if(!QDir(dir).exists())
                    return;
                progBarDownloadFile->setProperty("visible", QVariant(true));
                emit signalReceiveFile(QString::number(list[i].file_id), dir);
            }
        }
    }
}

void MainWindow::slotServerAnswerTasks(QString ID, QString operation, bool error)
{
    if(error)
    {
        QMessageBox::warning(nullptr, "Ошибка", "Произошла ошибка при обмене данными с серверм.", QMessageBox::Ok);
        return;
    }
    if(operation == "30")
    {
        if(ID.length() < 1)
            return;
        QObject* bTasks = mainWindowRoot->findChild<QObject*>("bTasks");
        int count = ID.toLong();
        QString str = "";
        if(count > 9)
            str = ">9";

        if(count >= 0 && count < 10)
            str = QString::number(count);
        else
            str = " ";
        bTasks->setProperty("text", QString("%1 (%2)").arg("Задачи", str));
    }
    else if(operation == "42")
    {
        QMessageBox::information(nullptr, "Успешно", "Права назначены.", QMessageBox::Ok);
        return;
    }
    else if(operation == "71")
    {
        long int id = ID.toLong();
        if(id < 1)
            return;
        QVector<TaskItem> items = taskList.items();
        for(int i = 0; i < items.size(); i++)
        {
            if(id == items[i].ID)
            {
                taskList.removeItem(i);
                break;
            }
        }
    }
    else if(operation == "76")
    {
        QMessageBox::information(nullptr, "Успешно", "Выполненные задачи удалены.", QMessageBox::Ok);
        return;
    }
}

void MainWindow::slotAddUsersTaskWnd(QVector<UserItem> users)
{
    for(int i = 0; i < users.size(); i++)
    {
        allUserList.appendItem(users[i]);
    }
    QObject* listView = tasksRoot->findChild<QObject*>("listViewUsersTasks");
    listView->setProperty("currentIndex", QVariant(-1));
}

void MainWindow::slotAddTaskToView(TaskItem task)
{
    taskList.appendItem(task);
}

void MainWindow::slotSetPogressBarValue(int value)
{
    if(value < 0 && value > 10000)
        return;
    progBarDownloadFile->setProperty("value", value);
}

void MainWindow::slotFileReceived(bool error)
{
    if(error)
    {
        QMessageBox::warning(nullptr, "Ошибка", "Произошла ошибка при загрузке файла", QMessageBox::Ok);
    }
    progBarDownloadFile->setProperty("visible", QVariant(false));
}

void MainWindow::slotAddUsers(QVector<UserItem> users, bool clear)
{
    if(clear)
        userList.removeAll();
    for(int i = 0; i < users.size(); i++)
    {
        addUser(users[i]);
    }
}

inline void MainWindow::addUser(UserItem user)
{
    userList.appendItem(user);
}

inline void MainWindow::requestTasksCount()
{
    emit signalAddMessage("", quint16(20));
}

void MainWindow::newMessageTimerCallback()
{
    if(mainWindowRoot->property("visible") == QVariant(false) && newMessage)
    {
        QMetaObject::invokeMethod(trayIcon, "showMessage",
                                  Q_ARG(QString, QString("Уведомление")),
                                  Q_ARG(QString, QString("Получено новое сообщение")));
    }
}

void MainWindow::slotAppendText(QString msg, quint16 status)
{
    if(mainWindowRoot->property("visible") == QVariant(false) && (status == 101 || status == 82))
        newMessage = true;

    QObject* mainTextArea = mainWindowRoot->findChild<QObject*>("mainTextArea");
    if(status == 0)
    {
        UserItem item;
        item.ID = QString::number(globalID);
        item.name = globalNickname;
        item.boss = false;
        addUser(item);
    }
    else if(status == 200)
    {
        userList.removeAll();
    }
    QTime time = QTime::currentTime();
    QString time_str = QString("[%1] ").arg(time.toString());

    QMetaObject::invokeMethod(mainTextArea, "append", Q_ARG(QString, QString("%1 %2").arg(time_str, msg)));

    if(status == 101 || status == 82)
        new_message_sound->play();
}

//Взаимодействие с progressbar'ом########################
void MainWindow::slotSetProgressBarVisibility(bool visibility, bool error)
{    
    QObject* progBar = mainWindowRoot->findChild<QObject*>("progressBarFile");
    if(visibility)
    {
        progBar->setProperty("width", 100);
    }
    else
    {
        progBar->setProperty("width", 0);
    }
    if(error)
    {
        QObject* msgDialog = mainWindowRoot->findChild<QObject*>("msgDialog");
        msgDialog->setProperty("text", "Произошла ошибка при загрузке файла.");
        QMetaObject::invokeMethod(msgDialog, "open");
    }
}
void MainWindow::slotSetProgressBarValue(int value)
{
    if(value > 100000 || value < 0) return;
    QObject* progBar = mainWindowRoot->findChild<QObject*>("progressBarFile");
    progBar->setProperty("value", value);
}
//Взаимодействие с progressbar'ом########################

MainWindow::~MainWindow()
{
    if(logged)
    {
        emit signalStopFileManager();
        emit StopChatBuffer();
        fileManagerThread->quit();
        fileManagerThread->wait();
        chatBufferThread->quit();
        chatBufferThread->wait();
        tasksCountTimer->stop();
        delete tasksCountTimer;
        delete tcp_client;
        delete file_manager;
    }
    delete new_message_sound;
}
