#ifndef MAIN_H
#define MAIN_H

#include <QGuiApplication>
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QQmlContext>
#include <QMetaObject>
#include <QString>
#include <QFile>
#include <QIcon>
#include <QDataStream>
#include <QSharedMemory>
#include <QMessageBox>
#include "models.h"
#include "mainwindow.h"

//GLOBALS
QString globalNickname;
long int globalID = 0;
int globalBoss = 0;
QString server_ip;
QString globalFilesDirectory;

//Создаем область с делимой памятью для запуска только одного приложение
QSharedMemory* sharedMemory;

//Списки, связанные с моделями
UserList userList;
UserList allUserList;
TaskList taskList;

QMutex files_mtx;
QVector<QVector<QString>> files;

int ReadOptions();

bool isRunning();

#endif // MAIN_H
