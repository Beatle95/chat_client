#include "main.h"

int main(int argc, char *argv[])
{

    QApplication app(argc, argv);
    //Проверка, запущено ли уже приложение
    if(isRunning())
    {
        QMessageBox::warning(nullptr, "Внимание!", "Приложение уже запущено!", QMessageBox::Ok);
        app.exit();
        return 0;
    }
    //1. Регистрируем в QML класс модели
    qmlRegisterType<ListViewModel>("Models", 1, 0, "UserModel");
    qmlRegisterType<TaskViewModel>("Models", 1, 0, "TaskModel");

    //2. Регистрируем в QML необъявляемый класс списка
    qmlRegisterUncreatableType<UserList>("Models", 1, 0, "UserList",
        QStringLiteral("UserList should not be created in QML!"));
    qmlRegisterUncreatableType<TaskList>("Models", 1, 0, "TaskList",
        QStringLiteral("UserList should not be created in QML!"));

    //3. Регистрируем в QML мета тип <QVector<...>> иначе QML его не поймет
    qRegisterMetaType<QVector<UserItem> >("QVector<UserItem>");
    qRegisterMetaType<QVector<TaskItem> >("QVector<TaskItem>");
    qRegisterMetaType<TaskItem>("TaskItem");

    QQuickStyle::setStyle("Universal");
    QQmlApplicationEngine engine;

    //4. Регистрируем в QML экземпляры классов, реализующих списки моделей
    engine.rootContext()->setContextProperty(QStringLiteral("userList"), &userList);
    engine.rootContext()->setContextProperty(QStringLiteral("allUserList"), &allUserList);
    engine.rootContext()->setContextProperty(QStringLiteral("taskList"), &taskList);

    const QUrl urlMainWindow(QStringLiteral("qrc:/mainwindow.qml"));
    const QUrl urlLoginWindow(QStringLiteral("qrc:/loginuserform.qml"));
    const QUrl urlRegisterWindow(QStringLiteral("qrc:/registerform.qml"));
    const QUrl urlOptionsWindow(QStringLiteral("qrc:/optionsform.qml"));
    const QUrl urlTasksWindow(QStringLiteral("qrc:/taskswindow.qml"));

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                    &app,
                    [urlLoginWindow](QObject *obj, const QUrl &objUrl) {
                        if (!obj && urlLoginWindow == objUrl)
                        QCoreApplication::exit(-1);
                    },
                    Qt::QueuedConnection);

    engine.load(urlMainWindow);
    engine.load(urlLoginWindow);
    engine.load(urlRegisterWindow);
    engine.load(urlOptionsWindow);
    engine.load(urlTasksWindow);

    //Запуск основного окна
    QObject* main_window_root = engine.rootObjects()[0];
    QObject* login_window_root = engine.rootObjects()[1];
    QObject* register_window_root = engine.rootObjects()[2];
    QObject* option_window_root = engine.rootObjects()[3];
    QObject* tasks_window_root = engine.rootObjects()[4];

    MainWindow* main_window = new MainWindow(main_window_root, login_window_root, register_window_root, option_window_root, tasks_window_root);

    QObject::connect(main_window_root, SIGNAL(qsignalSendButtonClicked(QString)), main_window, SLOT(slotMessageSent(QString)));
    QObject::connect(main_window_root, SIGNAL(qsignalButtonClicked(const QString)), main_window, SLOT(slotButtonClick(const QString)));
    QObject::connect(tasks_window_root, SIGNAL(qsignalTaskButtonClick(const QString, const QString)), main_window, SLOT(slotTaskButtonClick(const QString, const QString)));
    QObject::connect(login_window_root, SIGNAL(qsignalOnButtonClicked(const QString)), main_window, SLOT(slotButtonClick(const QString)));
    QObject::connect(register_window_root, SIGNAL(qsignalOnButtonClicked(const QString)), main_window, SLOT(slotButtonClick(const QString)));
    QObject::connect(option_window_root, SIGNAL(qsignalOnButtonClicked(const QString)), main_window, SLOT(slotButtonClick(const QString)));
    QObject::connect(main_window_root, SIGNAL(qsignalFileChosen(const QString)), main_window, SLOT(slotFileChosen(const QString)));

    int ret = ReadOptions();
    if(ret < 0)
        main_window->optionsMiss();
    else
    {
        bool logged = false;
        QString _login;
        QString _password;
        main_window->check_logon(&_login, &_password);
        if(_login != "" && _password != "")
        {
            //значит данные входа были сохранены
            long int i = main_window->login(_login, _password);
            if(i > 0)
            {
                //логин успешен
                globalID = i;
                logged = true;
            }
            else
            {
                QObject* loginDialog = login_window_root->findChild<QObject*>("loginDialog");
                loginDialog->setProperty("text", "Не удалось войти при помощи сохраненных данных!");
                QMetaObject::invokeMethod(loginDialog, "open");
            }
        }
        if(logged)
        {
            main_window->launchMainWnd();
        }
    }
    int return_value = app.exec();
    delete sharedMemory;    //Не забываем явно удалить разлеяемый участок памяти
    return return_value;
}

int ReadOptions()
{
    QFile opt_f("options.opt");
    if(!opt_f.open(QIODevice::ReadOnly))
        return -1;      //no such file or directory
    QDataStream inputStream(&opt_f);
    inputStream >> server_ip;
    inputStream >> globalFilesDirectory;
    opt_f.close();
    return 0;
}

bool isRunning()
{
    sharedMemory = new QSharedMemory;
    QString name = qgetenv("USER");
        if (name.isEmpty())
            name = qgetenv("USERNAME");

    sharedMemory->setKey("cyberchemp_chat_" + name);
    if(sharedMemory->create(1) == false)
        return true;
    return false;
}
