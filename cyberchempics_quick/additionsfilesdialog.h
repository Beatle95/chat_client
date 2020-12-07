#ifndef ADDITIONSFILESDIALOG_H
#define ADDITIONSFILESDIALOG_H

#include <QObject>
#include <QDialog>
#include <QTimer>
#include <QMovie>
#include <QMessageBox>
#include <QMutex>
#include <QFileDialog>
#include <QProcess>

extern QString globalFilesDirectory;
extern QMutex files_mtx;
extern QVector<QVector<QString>> files;


namespace Ui {
class AdditionsFilesDialog;
}


class AdditionsFilesDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AdditionsFilesDialog(QWidget *parent = nullptr);
    ~AdditionsFilesDialog();
private:
    Ui::AdditionsFilesDialog *ui;
    QTimer* timer;
    QMovie* movie;

    void saveFile(QString checkedDest);

public slots:
    void slotFileListProcessed();
    void slotTimerEllapsed();
    void slotFileReceived(bool error);
    void slotSetPogressBarValue(int val);

signals:
    void signalReceiveFile(QString checkedID, QString fileName);

private slots:
    void on_pbClose_clicked();
    void on_pbSaveAs_clicked();
    void on_pbSave_clicked();
    void on_pbFind_clicked();
    void on_pbOpenInExplorrer_clicked();
};

#endif // ADDITIONSFILESDIALOG_H
