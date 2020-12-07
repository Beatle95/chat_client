#include "chatbuffer.h"

void inline ChatBuffer::BufferInit()
{
    for(int i = 0; i < 9; i++)
    {
        buffer[i].next = &buffer[i + 1];
    }
    buffer[9].next = &buffer[0];
    first = &buffer[0];
    last = &buffer[0];
    message_count = 0;
}

ChatBuffer::ChatBuffer()
{
    status = 0;
    message_state = MessageState::READY;
    waitForMessage = 0;
    _run = true;
    isTasksCountRequested = false;
    BufferInit();
}

ChatBuffer::~ChatBuffer()
{
    delete &lastMessage;
}

void ChatBuffer::Run()
{
    QEventLoop* eventLoop = new QEventLoop();
    QByteArray array;
    while (_run) {
        if(isMessageReady() && message_state == MessageState::READY)
        {
            array = PullMessage();
            message_state = MessageState::SENT;

            QByteArray arrBlock(array.size()+64, 0);
            quint16 size = quint16(arrBlock.size() - sizeof(quint16));
            arrBlock[1] = *((char*)&size);
            arrBlock[0] = *(((char*)&size)+1);
            arrBlock[3] = *((char*)&status);
            arrBlock[2] = *(((char*)&status)+1);
            //Добавляем в сообщение ID
            QString strID = QString::number(globalID);
            QByteArray arrID = strID.toUtf8();
            int arrayEnd = arrID.size() < 60 ? arrID.size()+4 : 64;
            for(int i = 4; i < arrayEnd; i++)
            {
                arrBlock[i] = arrID[i-4];
            }
            //Копируем данные в буфер отправки
            for(int i = 64; i < array.size()+64; i++)
            {
                arrBlock[i] = array[i-64];
            }
            emit signalSendMessage(arrBlock);
            timer_elapsed.start();
        }
        else if(message_state == MessageState::SENT)
        {
            if(timer_elapsed.elapsed() > 5000)
            {
                if(this->status == 13)
                    emit signalPasswordChanged(true);
                else if(this->status == 20)
                    isTasksCountRequested = false;
                else if(this->status == 25)
                    emit signalServerAnswerTasks("", "25", true);
                else if(this->status == 23 || this->status == 40 || this->status == 41 || this->status == 50 ||
                        this->status == 70 || this->status == 75 || this->status == 80)   //Ошибка нет ответа от сервера
                    emit signalServerAnswerTasks("", "", true);
                else if(this->status == 101)
                    emit signalAppendText("*Ошибка доставки сообщения!*", quint16(255));
                //Всегда продвигаем очередь в этом случае
                MoveMessageQueue();
            }
        }
        eventLoop->processEvents();
        QThread::msleep(1);
    }
}

inline void ChatBuffer::MoveMessageQueue()
{
    mutex.lock();
    first = first->next;
    --message_count;
    mutex.unlock();
    message_state = MessageState::READY;
}

void ChatBuffer::slotDataReceived(QString str, quint16 status)
{
    switch (status) {
    case 0:     //Подключение к серверу успешно
        //Отправка данных на сервер
        AddMessage(globalNickname, quint16(1));
        break;
    case 2:     //Данные пользователя получены
        MoveMessageQueue();
        break;
    case 10:    //Пользователь присоединился к чату
        user_added(str);
        break;
    case 11:    //Пользователь покинул чат
        user_left(str);
        break;
    case 14:
        emit signalPasswordChanged();
        MoveMessageQueue();
        break;
    case 15:    //Получен синхронизирующий кадр с очисткой
        emit signalAddUsers(processUsersString(&str), true);
        break;
    case 26:
        processUsers(str);
        break;
    case 27:    //Передача списка зарегистрированных пользователей завершена
        emit signalServerAnswerTasks("", "27");
        MoveMessageQueue();
        break;
    case 30:    //Ответ: количество активных задач
        emit signalServerAnswerTasks(str, "30");
        isTasksCountRequested = false;
        MoveMessageQueue();
        break;
    case 42:    //Успешное изменение прав
        emit signalServerAnswerTasks("", QString::number(status));
        MoveMessageQueue();
        break;
    case 51:    //Передан список всех активных задач
        processTasks(str);
        break;
    case 52:    //Передан список всех активных задач(конец)
        emit signalServerAnswerTasks("", QString::number(status));
        MoveMessageQueue();
        break;
    case 71:
        emit signalServerAnswerTasks(str, "71", false);
        MoveMessageQueue();
        break;
    case 76:
        emit signalServerAnswerTasks(str, "76", false);
        MoveMessageQueue();
        break;
    case 81:
        emit signalServerAnswerTasks(str, "81", false);
        MoveMessageQueue();
        break;
    case 82:    //Была выставлена новая задача
        emit signalAppendText("#Получена новая задача.", status);
        MoveMessageQueue();
        break;
    case 100:
        if(first->status != 1)  //Если только пользователь не передал свои данные (устарело)
            emit signalAppendText(globalNickname + " > " + lastMessage, status);
        MoveMessageQueue();
        break;
    case 101:
        //Обработка и вывод принятых сообщений от другого клиента
        emit signalAppendText(str, status);
        break;
    case 115:   //Получен броадкаст о передаче файла
        emit signalAppendText(processFileBroadcast(&str), status);
        break;
    case 255:
        if(this->status == 13)
        {
            emit signalPasswordChanged(true);
            MoveMessageQueue();
        }
        else if(this->status == 23)
        {
            emit signalServerAnswerTasks("", "23", true);
            MoveMessageQueue();
        }
        else if(this->status == 25)
        {
            emit signalServerAnswerTasks("", "25", true);
            MoveMessageQueue();
        }
        else if(this->status == 40 || this->status == 41)
        {
            emit signalServerAnswerTasks("", "40||41", true);
            MoveMessageQueue();
        }
        else if(this->status == 50)
        {
            emit signalServerAnswerTasks("", "50", true);
            MoveMessageQueue();
        }
        else if(this->status == 70)
        {
            emit signalServerAnswerTasks("", "70", true);
            MoveMessageQueue();
        }
        else if(this->status == 75)
        {
            emit signalServerAnswerTasks("", "75", true);
            MoveMessageQueue();
        }
        else if(this->status == 80)
        {
            emit signalServerAnswerTasks("", "80", true);
            MoveMessageQueue();
        }
        break;
    default:
        break;
    }

}

void ChatBuffer::user_left(QString str)
{
    QStringList list = str.split('|');
    if(list.length() < 2)
        return;
    emit signalAppendText("#Пользователь <" + list[1] + "> покинул чат.", quint16(10));
}

void ChatBuffer::processUsers(QString &str)
{
    QStringList list = str.split('|', Qt::SkipEmptyParts);
    QVector<UserItem> users;
    for(int i = 0; i < list.size(); i+=3)
    {
        if(i + 2 >= list.size())
            break;
        UserItem item;
        item.ID = list[i];
        item.name = list[i+1];
        item.boss = (QString(list[i+2]).toInt() > 0) ? true : false;
        users.append(item);
    }
    emit signalAddUsersTasksWnd(users);
}

void ChatBuffer::processTasks(QString &str)
{
    if(str.isEmpty())
        return;
    QStringList list = str.split('|');
    if(list.size() != 11)
        return;

    TaskItem item;
    item.ID = QString(list[0]).toLong();
    item.user_id = QString(list[1]).toLong();
    item.user_name = list[2];
    item.admin_id = QString(list[3]).toLong();
    item.admin_name = list[4];
    item.title = list[5];
    item.text = list[6];
    item.file_id = QString(list[7]).toLong();
    if(list[8] == "")
        item.file_name = "нет прикрепленного файла";
    else
        item.file_name = list[8];
    item.date = list[9];
    item.done = QString(list[10]).toInt();
    emit signalAddTaskToView(item);
}

void ChatBuffer::user_added(QString str)
{
    QStringList user_data = str.split('|');
    if(user_data.count() < 2)
        return;
    emit signalAppendText("#Пользователь <" + user_data[1] + "> присоединился к чату.", quint16(10));
}

inline void ChatBuffer::buffer_copy(char* dest, QByteArray *source)
{
    for(int i = 0; i < source->size(); i++)
        dest[i] = source->at(i);
}

//return true, if success. If false returned, messagestack are full
bool ChatBuffer::AddMessage(QString str, quint16 stat = 101)
{
    if(message_count > 9)
        return false;

    if(message_count < 0)
        message_count = 0;

    if(str.size() > 4000)
        str.truncate(4000);

    //Проверка на обновление количества задач
    if(stat == 20)
    {
        if(isTasksCountRequested)
            return false;
        isTasksCountRequested = true;
    }
    QByteArray byteStr = str.toUtf8();
    mutex.lock();
    memset(last->buf, 0, 4096);
    buffer_copy(last->buf, &byteStr);
    last->size = byteStr.size();
    last->status = stat;
    last = last->next;
    ++message_count;
    mutex.unlock();
    return true;
}

QByteArray ChatBuffer::PullMessage()
{
    QByteArray array;
    mutex.lock();
    status = first->status;
    for(quint16 i = 0; i < first->size; i++)
    {
        array.append(first->buf[i]);
    }
    mutex.unlock();

    lastMessage = array;
    return array;
}

QVector<UserItem> ChatBuffer::processUsersString(QString *str)
{
    QVector<UserItem> retVect;
    QStringList list = str->split('|', Qt::SkipEmptyParts);
    int complete = list.length() / 2;
    if(complete < 1)
    {
        list.clear();
        return retVect;
    }
    for(int user = 0; user < complete*2; user+=2)
    {
        UserItem item;
        item.ID = list[user];
        item.name = list[user+1];
        retVect.append(item);
    }
    return retVect;
}

QString ChatBuffer::processFileBroadcast(QString *str)
{
    QStringList list = str->split("|");
    if(list.size() < 4)
        return QString("Получен файл.");
    return QString("Пользователь <%1> загрузил файл [%2].").arg(list[1], list[2]);
}

bool inline ChatBuffer::isMessageReady()
{
    if(message_count > 0)
        return true;
    else
        return false;
}

void ChatBuffer::Stop()
{
    _run = false;
}
