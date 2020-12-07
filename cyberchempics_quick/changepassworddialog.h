#ifndef CHANGEPASSWORDDIALOG_H
#define CHANGEPASSWORDDIALOG_H

#include <QDialog>
#include <QMovie>
#include <QMessageBox>

namespace Ui {
class ChangePasswordDialog;
}

class ChangePasswordDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChangePasswordDialog(QWidget *parent = nullptr);
    ~ChangePasswordDialog();

private slots:
    void on_pbCancel_clicked();
    void on_pbSave_clicked();

public slots:
    void slotPasswordChanged(bool error);

private:
    Ui::ChangePasswordDialog *ui;
    QMovie* movie;

signals:
    signalAddMessage(QString mes, quint16 status);
};

#endif // CHANGEPASSWORDDIALOG_H
