#include "models.h"

ListViewModel::ListViewModel(QObject *parent)
    : QAbstractListModel(parent),
      mList(nullptr)
{
}

int ListViewModel::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid() || !mList)
        return 0;

    return mList->items().size();
}

QVariant ListViewModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || !mList)
        return QVariant();

    const UserItem item = mList->items().at(index.row());
    switch (role) {
    case IDRole:
        return QVariant(item.ID);
    case NameRole:
        return QVariant(item.name);
    case BossRole:
        return QVariant(item.boss);
    }
    return QVariant();
}

bool ListViewModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(!mList)
        return false;

    UserItem item = mList->items().at(index.row());
    switch (role) {
    case IDRole:
        item.ID = value.toString();
        break;
    case NameRole:
        item.name = value.toString();
        break;
    case BossRole:
        item.boss = value.toBool();
        break;
    }

    if(mList->setItemAt(index.row(), item)){
        emit dataChanged(index, index, QVector<int>() << role);
        return true;
    }
    return false;
}

Qt::ItemFlags ListViewModel::flags(const QModelIndex &index) const
{
    if(!index.isValid())
        return Qt::NoItemFlags;
    return Qt::NoItemFlags;
}

bool ListViewModel::insertRows(int row, int count, const QModelIndex &parent)
{
    beginInsertRows(parent, row, row + count - 1);
    // FIXME: Implement me!
    endInsertRows();
    return false;
}

bool ListViewModel::removeRows(int row, int count, const QModelIndex &parent)
{
    beginRemoveRows(parent, row, row + count - 1);
    //mList->removeItems(row, count);
    endRemoveRows();
    return false;
}

QHash<int, QByteArray> ListViewModel::roleNames() const
{
    QHash<int, QByteArray> names;
    names[IDRole] = "id";
    names[NameRole] = "name";
    names[BossRole] = "boss";
    return names;
}

UserList *ListViewModel::list() const
{
    return mList;
}

void ListViewModel::setList(UserList *list)
{
    beginResetModel();
    if(mList)
        mList->disconnect(this);
    mList = list;
    if(mList)
    {
        connect(mList, &UserList::preItemAppend, this, [=](){
            const int index = mList->items().size();
            beginInsertRows(QModelIndex(), index, index);
        });
        connect(mList, &UserList::postItemAppend, this, [=](){
            endInsertRows();
        });
        connect(mList, &UserList::preItemRemoved, this, [=](int index, int lastIndex){
            beginRemoveRows(QModelIndex(), index, lastIndex);
        });
        connect(mList, &UserList::postItemRemoved, this, [=](){
            endRemoveRows();
        });
    }
    endResetModel();
}

//################# UserList Class ##############
UserList::UserList(QObject *parent) : QObject(parent)
{
}

QVector<UserItem> UserList::items() const
{
    return mItems;
}

bool UserList::setItemAt(int index, const UserItem &item)
{
    if(index < 0 || index >= mItems.size())
        return false;

    const UserItem &oldItem = mItems.at(index);
    if(item.ID == oldItem.ID && item.name == oldItem.name && item.boss == oldItem.boss)
        return false;

    mItems[index] = item;
    return true;
}

void UserList::removeAll()
{
    emit preItemRemoved(0, mItems.size()-1);
    mItems.clear();
    emit postItemRemoved();
}

void UserList::appendItem(const UserItem &item)
{
    emit preItemAppend();
    mItems.append(item);
    emit postItemAppend();
}
//#################UserList Class##############

//################  Task List ############

TaskList::TaskList(QObject *parent) : QObject(parent)
{
}

QVector<TaskItem> TaskList::items() const
{
    return mItems;
}

bool TaskList::setItemAt(int index, const TaskItem &item)
{
    if(index < 0 || index >= mItems.size())
        return false;

    const TaskItem &oldItem = mItems.at(index);
    if(item.ID == oldItem.ID && item.user_id == oldItem.user_id && item.user_name == oldItem.user_name &&
            item.admin_id == oldItem.admin_id && item.admin_name == oldItem.admin_name && item.title == oldItem.title &&
            item.text == oldItem.text && item.file_id == oldItem.file_id && item.file_name == oldItem.file_name &&
            item.date == oldItem.date && item.done == oldItem.done)
        return false;

    mItems[index] = item;
    return true;
}

void TaskList::removeItem(int index)
{
    if(index < 0)
        return;
    if(index > mItems.size() - 1)
        return;

    emit preItemRemoved(index, index);
    mItems.remove(index);
    emit postItemRemoved();
}

void TaskList::removeAll()
{
    emit preItemRemoved(0, mItems.size()-1);
    mItems.clear();
    emit postItemRemoved();
}

void TaskList::appendItem(const TaskItem &item)
{
    emit preItemAppend();
    mItems.append(item);
    emit postItemAppend();
}
//################  Task List ############

//################  Task Model ############

TaskViewModel::TaskViewModel(QObject *parent)
    : QAbstractListModel(parent),
      mList(nullptr)
{
}

int TaskViewModel::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid() || !mList)
        return 0;

    return mList->items().size();
}

QVariant TaskViewModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || !mList)
        return QVariant();

    const TaskItem item = mList->items().at(index.row());
    switch (role) {
    case IDRole:
        return QVariant(QString::number(item.ID));
    case StateRole:
        if(item.done == 0)
            return QVariant("В процессе выполнения...");
        else
            return QVariant("Выполнено");
    case AdminNameRole:
        return QVariant(item.admin_name);
    case UserNameRole:
        return QVariant(item.user_name);
    case TitleRole:
        return QVariant(item.title);
    case TextRole:
        return QVariant(item.text);
    case FileNameRole:
        return QVariant(item.file_name);
    case DateRole:
        return QVariant(item.date);
    }
    return QVariant();
}

Qt::ItemFlags TaskViewModel::flags(const QModelIndex &index) const
{
    if(!index.isValid())
        return Qt::NoItemFlags;
    return Qt::NoItemFlags;
}

bool TaskViewModel::insertRows(int row, int count, const QModelIndex &parent)
{
    beginInsertRows(parent, row, row + count - 1);
    // FIXME: Implement me!
    endInsertRows();
    return false;
}

bool TaskViewModel::removeRows(int row, int count, const QModelIndex &parent)
{
    beginRemoveRows(parent, row, row + count - 1);
    //mList->removeItems(row, count);
    endRemoveRows();
    return false;
}

QHash<int, QByteArray> TaskViewModel::roleNames() const
{
    QHash<int, QByteArray> names;
    names[IDRole] = "mID";
    names[StateRole] = "mState";
    names[AdminNameRole] = "mAdminName";
    names[UserNameRole] = "mUserName";
    names[TitleRole] = "mTitle";
    names[TextRole] = "mText";
    names[FileNameRole] = "mFile";
    names[DateRole] = "mDate";
    return names;
}

TaskList *TaskViewModel::list() const
{
    return mList;
}

void TaskViewModel::setList(TaskList *list)
{
    beginResetModel();
    if(mList)
        mList->disconnect(this);
    mList = list;
    if(mList)
    {
        connect(mList, &TaskList::preItemAppend, this, [=](){
            const int index = mList->items().size();
            beginInsertRows(QModelIndex(), index, index);
        });
        connect(mList, &TaskList::postItemAppend, this, [=](){
            endInsertRows();
        });
        connect(mList, &TaskList::preItemRemoved, this, [=](int index, int lastIndex){
            beginRemoveRows(QModelIndex(), index, lastIndex);
        });
        connect(mList, &TaskList::postItemRemoved, this, [=](){
            endRemoveRows();
        });
    }
    endResetModel();
}
