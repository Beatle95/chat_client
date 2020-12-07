#ifndef ADDTASKDIALOG_H
#define ADDTASKDIALOG_H

#include <QDialog>
#include <QMutex>
#include <QMessageBox>
#include "models.h"

extern QMutex files_mtx;
extern QVector<QVector<QString>> files;

namespace Ui {
class AddTaskDialog;
}

class AddTaskDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddTaskDialog(TaskItem *t, QWidget *parent = nullptr);
    ~AddTaskDialog();

private:
    Ui::AddTaskDialog *ui;
    TaskItem* task;

private slots:
    void on_pbClose_clicked();
    void on_pbSave_clicked();

public slots:
    void slotFileListProcessed();
};

#endif // ADDTASKDIALOG_H
