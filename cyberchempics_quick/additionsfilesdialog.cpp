#include "additionsfilesdialog.h"
#include "ui_additionsfilesdialog.h"

AdditionsFilesDialog::AdditionsFilesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AdditionsFilesDialog)
{
    ui->setupUi(this);
    ui->pbFileDownload->setVisible(false);
    //врубаем анимацию загрузки
    movie = new QMovie(":/img/loader.gif");
    ui->lableLoad->setVisible(true);
    ui->lableLoad->setMovie(movie);
    movie->start();
    //таймер, который не допустить слишком долгое ожидание ответа от сервера
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &AdditionsFilesDialog::slotTimerEllapsed);
    timer->start(5000); //через 5 сек забиваем и отрубаемся
}

AdditionsFilesDialog::~AdditionsFilesDialog()
{
    delete movie;
    delete ui;
}

void AdditionsFilesDialog::saveFile(QString checkedDest)
{
    QItemSelectionModel *selection = ui->twFiles->selectionModel();
    QModelIndexList indexes = selection->selectedIndexes();
    int index = indexes.at(0).row();
    if(index >= files.size())
        return;
    long int id = files[index][0].toLong();
    if(id < 1)
        return;
    //Вызываем метод получения файла
    emit signalReceiveFile(QString::number(id), checkedDest);
    ui->pbClose->setEnabled(false);
    ui->pbSave->setEnabled(false);
    ui->pbSaveAs->setEnabled(false);
    ui->pbFileDownload->setValue(0);
    ui->pbFileDownload->setVisible(true);
}

void AdditionsFilesDialog::slotFileListProcessed()
{
    files_mtx.lock();
    if(timer->isActive())
        timer->stop();
    movie->stop();
    ui->lableLoad->setVisible(false);
    ui->verticalLayout->setEnabled(true);
    ui->twFiles->setColumnCount(4);
    ui->twFiles->setHorizontalHeaderLabels(QStringList() << "id" << "Пользователь" << "Название" << "Дата" );
    ui->twFiles->setColumnWidth(0, 50);     //ширина колонок
    ui->twFiles->setColumnWidth(1, 140);
    ui->twFiles->setColumnWidth(2, 170);
    ui->twFiles->setColumnWidth(3, 120);
    ui->twFiles->setRowCount(files.size()); //задание количества строк
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

void AdditionsFilesDialog::slotTimerEllapsed()
{
    QMessageBox::warning(this, tr("Ошибка сервера"), tr("Окно будет закрыто!"), QMessageBox::Ok);
    close();
}

void AdditionsFilesDialog::slotFileReceived(bool error)
{
    if(error)
        QMessageBox::warning(this, "Ошибка", "Возникла ошибка при получении файла", QMessageBox::Ok);
    ui->pbClose->setEnabled(true);
    ui->pbSave->setEnabled(true);
    ui->pbSaveAs->setEnabled(true);
    ui->pbFileDownload->setVisible(false);
}

void AdditionsFilesDialog::slotSetPogressBarValue(int val)
{
    if(val > 10000 || val < 0)
        return;
    ui->pbFileDownload->setValue(val);
}

void AdditionsFilesDialog::on_pbClose_clicked()
{
    close();
}

void AdditionsFilesDialog::on_pbSaveAs_clicked()
{
    //Проверка на выделенную строку
    QItemSelectionModel *selection = ui->twFiles->selectionModel();
    if(!selection->hasSelection())
    {
        QMessageBox::warning(this, "Предупреждение", "Нужно выделить файл!", QMessageBox::Ok);
        return;
    }
    //Открываем диалоговое окно для выбора папки сохранения
    QString dir = QFileDialog::getExistingDirectory(this,
                                                     tr("Выберите папку"),
                                                     "С:/",
                                                     QFileDialog::ShowDirsOnly
                                                     | QFileDialog::DontResolveSymlinks);
    if(dir.isEmpty())
        return;
    if(!QDir(dir).exists())
        return;
    saveFile(dir);
}

//Сохранить в один клик в указанную в настройках папку
void AdditionsFilesDialog::on_pbSave_clicked()
{
    //Проверка на выделенную строку
    QItemSelectionModel *selection = ui->twFiles->selectionModel();
    if(!selection->hasSelection())
    {
        QMessageBox::warning(this, "Предупреждение", "Нужно выделить файл!", QMessageBox::Ok);
        return;
    }
    if(QDir(globalFilesDirectory).exists()){
        //Папку существует, сохраняем новый файл
        saveFile(globalFilesDirectory);
    }
    else {
        QMessageBox::warning(this, "Предупреждение", "Невозможно сохраниться в папку, указанную в настройках!", QMessageBox::Ok);
        QString dir = QFileDialog::getExistingDirectory(this,
                                                         tr("Выберите папку"),
                                                         "С:/",
                                                         QFileDialog::ShowDirsOnly
                                                         | QFileDialog::DontResolveSymlinks);
        if(dir.isEmpty())
            return;
        if(!QDir(dir).exists())
            return;
        saveFile(dir);
    }
}

void AdditionsFilesDialog::on_pbFind_clicked()
{
    //поиск по списку файлов
    QString look_for = ui->leFind->text();
    for(int i = 0; i < files.size(); i++)
    {
        if(files[i][2].indexOf(look_for, 0, Qt::CaseInsensitive) > -1) //Нашли нужный элемент
        {
            ui->twFiles->selectRow(i);
            ui->twFiles->scrollTo(ui->twFiles->model()->index(i, 0));
            ui->twFiles->setFocus();
            break;
        }
    }
}

void AdditionsFilesDialog::on_pbOpenInExplorrer_clicked()
{
    QString dir = globalFilesDirectory.replace('/', '\\');
    QStringList params;
    params.append(dir);
    QProcess::startDetached("explorer", params);
}
