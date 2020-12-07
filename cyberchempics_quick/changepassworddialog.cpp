#include "changepassworddialog.h"
#include "ui_changepassworddialog.h"

ChangePasswordDialog::ChangePasswordDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChangePasswordDialog)
{
    ui->setupUi(this);
    ui->labelLoad->setVisible(false);
    //врубаем анимацию загрузки
    movie = new QMovie(":/img/loader.gif");
    ui->labelLoad->setMovie(movie);
}

ChangePasswordDialog::~ChangePasswordDialog()
{
    delete ui;
}

void ChangePasswordDialog::on_pbCancel_clicked()
{
    close();
}

void ChangePasswordDialog::on_pbSave_clicked()
{
    QString old = ui->leOldPass->text();
    QString newPass = ui->leNewPass->text();
    QString confirmPass = ui->leConfirmPass->text();
    if(old.length() < 5 || newPass.length() < 5 || confirmPass.length() < 5 || old.length() > 30 || newPass.length() > 30 || confirmPass.length() > 30){
        QMessageBox::warning(this, "Предупреждение", "Пароли должны состять более чем из 4 символов и менее 30.", QMessageBox::Ok);
        return;
    }
    if(old.indexOf(",") > -1 || old.indexOf("|") > -1 || old.indexOf("/") > -1 || old.indexOf("\\") > -1 || old.indexOf("\'") > -1 || old.indexOf("\"") > -1){
        QMessageBox::warning(this, "Предупреждение", "Пароль содержит недопустимые символы.", QMessageBox::Ok);
        return;
    }
    if(newPass.indexOf(",") > -1 || newPass.indexOf("|") > -1 || newPass.indexOf("/") > -1 || newPass.indexOf("\\") > -1 || newPass.indexOf("\'") > -1 || newPass.indexOf("\"") > -1){
        QMessageBox::warning(this, "Предупреждение", "Пароль содержит недопустимые символы.", QMessageBox::Ok);
        return;
    }
    if(newPass != confirmPass){
        QMessageBox::warning(this, "Предупреждение", "Подтверждение пароля и новый пароль не совпадают.", QMessageBox::Ok);
        return;
    }
    QString msg;
    msg = old + "|" + newPass + "|";
    emit signalAddMessage(msg, (quint16)13);

    ui->label->setVisible(false);
    ui->label_2->setVisible(false);
    ui->label_3->setVisible(false);
    ui->leConfirmPass->setVisible(false);
    ui->leNewPass->setVisible(false);
    ui->leOldPass->setVisible(false);

    ui->pbSave->setEnabled(false);
    ui->labelLoad->setVisible(true);
    movie->start();
}

void ChangePasswordDialog::slotPasswordChanged(bool error)
{
    if(error)
        QMessageBox::warning(this, "Ошибка", "Не удалось изменить пароль\nВозможно, вы ввели неверный старый пароль?", QMessageBox::Ok);
    else
        QMessageBox::information(this, "Успешно", "Пароль был успешно изменен.", QMessageBox::Ok);
    ui->pbSave->setEnabled(true);

    ui->label->setVisible(true);
    ui->label_2->setVisible(true);
    ui->label_3->setVisible(true);
    ui->leConfirmPass->setVisible(true);
    ui->leNewPass->setVisible(true);
    ui->leOldPass->setVisible(true);

    movie->stop();
    ui->labelLoad->setVisible(false);
}
