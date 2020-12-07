#include "addtaskdialog.h"
#include "ui_addtaskdialog.h"

AddTaskDialog::AddTaskDialog(TaskItem *t, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddTaskDialog)
{
    ui->setupUi(this);
    task = t;
    ui->twFiles->setColumnCount(4);
    ui->twFiles->setHorizontalHeaderLabels(QStringList() << "id" << "Пользователь" << "Название" << "Дата" );
    ui->twFiles->setColumnWidth(0, 50);     //ширина колонок
    ui->twFiles->setColumnWidth(1, 140);
    ui->twFiles->setColumnWidth(2, 170);
    ui->twFiles->setColumnWidth(3, 120);
}

AddTaskDialog::~AddTaskDialog()
{
    delete ui;
}

void AddTaskDialog::on_pbClose_clicked()
{
    done(0);
}

void AddTaskDialog::on_pbSave_clicked()
{
    QString title = ui->leTitle->text();
    QString text = ui->teText->toPlainText();
    if(title.length() > 255 || title.length() < 5)
    {
        QMessageBox::warning(this, "Ошибка", "Заголовок должен быть длинне 5 и менее 255 символов.", QMessageBox::Ok);
        return;
    }
    if(text.length() < 5 || text.length() > 2000)
    {
        QMessageBox::warning(this, "Ошибка", "Текст должен быть длинне 5 и менее 2000 символов.", QMessageBox::Ok);
        return;
    }
    QItemSelectionModel *selection = ui->twFiles->selectionModel();
    QModelIndexList indexes = selection->selectedIndexes();
    if(indexes.isEmpty())
    {
        task->file_id = 0;
    }
    else
    {
        int index = indexes.at(0).row();
        if(index >= files.size())
            return;
        task->file_id = files[index][0].toLong();
    }
    task->title = title;
    task->text = text;
    done(1);
}

void AddTaskDialog::slotFileListProcessed()
{
    files_mtx.lock();
    ui->twFiles->setRowCount(files.size()); //задание количества строк
    for(int i = 0; i < ui->twFiles->rowCount(); i++)    //задание высоты каждой строки
        ui->twFiles->setRowHeight(i, 10);
    //Заполнение данными
    for(int i = 0; i < ui->twFiles->rowCount(); i++)
    {
        ui->twFiles->setRowHeight(i, 10);   //задание высоты каждой строки
        for(int j = 0; j < 4; j++)
        {
            ui->twFiles->setItem(i, j, new QTableWidgetItem(files[i][j]));
        }
    }
    files_mtx.unlock();
}
