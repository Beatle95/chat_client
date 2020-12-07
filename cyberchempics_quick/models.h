#ifndef LISTVIEWMODEL_H
#define LISTVIEWMODEL_H

#include <qqml.h>
#include <QAbstractListModel>
#include <QAbstractTableModel>
#include <QObject>
#include <QVector>

struct UserItem
{
    QString ID;
    QString name;
    bool boss = false;
};

struct TaskItem{
    long int ID;
    long int user_id;
    QString user_name;
    long int admin_id;
    QString admin_name;
    QString title;
    QString text;
    long int file_id;
    QString file_name;
    QString date;
    int done;
};

class UserList : public QObject
{
    Q_OBJECT
public:
    explicit UserList(QObject *parent = nullptr);
    QVector<UserItem> items() const;
    bool setItemAt(int index, const UserItem &item);
    void removeAll();

signals:
    void preItemAppend();
    void postItemAppend();
    void preItemRemoved(int index, int endIndex);
    void postItemRemoved();

public slots:
    void appendItem(const UserItem &item);

private:
    QVector<UserItem> mItems;
};

class ListViewModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(UserList *list READ list WRITE setList)

    enum{
        IDRole,
        NameRole,
        BossRole
    };

public:
    explicit ListViewModel(QObject *parent = nullptr);

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    Qt::ItemFlags flags(const QModelIndex &index) const;
    // Add data:
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    // Remove data:
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    virtual QHash<int, QByteArray> roleNames() const override;

    UserList *list() const;
    void setList(UserList *list);

private:
    UserList *mList;
};

//######################### Task List #############################
class TaskList : public QObject
{
    Q_OBJECT
public:
    explicit TaskList(QObject *parent = nullptr);
    QVector<TaskItem> items() const;
    bool setItemAt(int index, const TaskItem &item);
    void removeItem(int index);
    void removeAll();

signals:
    void preItemAppend();
    void postItemAppend();
    void preItemRemoved(int index, int endIndex);
    void postItemRemoved();

public slots:
    void appendItem(const TaskItem &item);

private:
    QVector<TaskItem> mItems;
};
//######################### Task List #############################

//######################### Task Model #############################
class TaskViewModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(TaskList *list READ list WRITE setList)

    enum{
        IDRole,
        StateRole,
        AdminNameRole,
        UserNameRole,
        TitleRole,
        TextRole,
        FileNameRole,
        DateRole
    };

public:
    explicit TaskViewModel(QObject *parent = nullptr);

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    // Add data:
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    // Remove data:
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    virtual QHash<int, QByteArray> roleNames() const override;

    TaskList *list() const;
    void setList(TaskList *list);

private:
    TaskList *mList;
};
//######################### Task Model #############################

#endif // LISTVIEWMODEL_H
