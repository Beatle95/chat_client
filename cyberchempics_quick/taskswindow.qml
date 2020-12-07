import QtQuick 2.0
import QtQuick.Window 2.10
import QtQuick.Controls 2.10
import QtQuick.Controls.Universal 2.12
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.2
import Models 1.0

Window {
    id:tasksWindow
    objectName: "tasksWindow"
    visible: false
    width: (paneUsersTasks.visible) ? 700 : 490
    height: 700
    title: qsTr("Чат")
    flags: Qt.FramelessWindowHint
    modality: Qt.ApplicationModal
    Universal.theme: Universal.Dark

    signal qsignalTaskButtonClick(string msg, string id)    

    // Declare properties that will store the position of the mouse cursor
    property int previousX
    property int previousY
    Pane{
        anchors.fill: parent
        anchors.topMargin: -11
        anchors.leftMargin: -11
        anchors.rightMargin: -11

        Pane{
            id: paneTasks
            width: 490
            anchors{
                top: dragBarTasks.bottom
                left: parent.left
                bottom: parent.bottom
            }
            Rectangle{
                id: listViewTasksRect
                anchors{
                    top: parent.top
                    left: parent.left
                    right: parent.right
                    bottom: parent.bottom
                }
                anchors.topMargin: 30
                border.width: 2
                border.color: "#858585"
                color: "transparent"
                ListView{
                    anchors.margins: 3
                    id: listViewTasks
                    objectName: "listViewTasks"
                    anchors.fill: parent
                    highlight: Rectangle {
                        color: "transparent"
                        border.color: Universal.color(Universal.Taupe)
                        border.width: 2
                        radius: 5
                    }
                    spacing: 5
                    clip: true

                    model: TaskModel {
                        list: taskList
                    }

                    delegate: RowLayout{
                        id:tasksRowLayout
                        width:listViewTasks.width
                        Column{
                            width: 5
                        }
                        //Действие при клике
                        MouseArea{
                            hoverEnabled: false
                            Layout.preferredWidth: 350
                            Layout.preferredHeight: colTasks2.height
                            onClicked: {
                                listViewTasks.currentIndex = index
                            }
                            Column{
                                id: colTasks2
                                spacing: 2
                                Layout.preferredWidth: 350
                                Item{
                                    height: 10
                                    width: 350
                                }
                                Text {
                                    id: lTextTaskID
                                    text: mID
                                    visible: false
                                }
                                Text {
                                    text: "<font color=\"#FA6800\"><b>" + mTitle + "</b></font>"
                                    color: "#ffffff"
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 14
                                    horizontalAlignment: Text.AlignHCenter
                                    width: 350
                                }
                                Text {
                                    text: "<font color=\"#FA6800\"><b>Состояние: </b></font>" + ((mState == "Выполнено") ? "<font color='green'>" : "<font color='red'>") + mState + "</font>"
                                    color: "#ffffff"
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 12
                                    width: 350
                                }
                                Text {
                                    text: "<font color=\"#FA6800\"><b>Поставивший задачу: </b></font>" + mAdminName
                                    color: "#ffffff"
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 12
                                    width: 350
                                }
                                Text {
                                    text: "<font color=\"#FA6800\"><b>Исполнитель: </b></font>" + mUserName
                                    color: "#ffffff"
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 12
                                    width: 350
                                }
                                Text {
                                    text: "<font color=\"#FA6800\"><b>Описание: </b></font>" + mText
                                    color: "#ffffff"
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 12
                                    width: 350
                                }
                                Text {
                                    text: "<font color=\"#FA6800\"><b>Прикрепленный файл: </b></font>" + mFile
                                    color: "#ffffff"
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 12
                                    width: 350
                                }
                                Text {
                                    text: "<font color=\"#FA6800\"><b>Дата получения: </b></font>" + mDate
                                    color: "#ffffff"
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 12
                                    width: 350
                                }
                                Item{
                                    height: 10
                                    width: 350
                                }
                            }
                        }
                        Column{
                            spacing: 60
                            Button{
                                id: lbDone
                                Layout.preferredHeight: 50
                                width: 85
                                text: "Завершить"
                                visible: (mState === "Выполнено") ? false : true
                                contentItem: Text {
                                    text: lbDone.text
                                    font.pointSize: 10
                                    color: "#ffffff"
                                    verticalAlignment: Text.AlignVCenter
                                    horizontalAlignment: Text.AlignHCenter
                                }
                                onClicked: qsignalTaskButtonClick("markCompleted", lTextTaskID.text);
                            }

                            Button{
                                id: lbDownload
                                Layout.preferredHeight: 50
                                width: 85
                                visible: (mFile == "нет прикрепленного файла") ? false : true
                                text: "Загрузить"
                                contentItem: Text {
                                    text: lbDownload.text
                                    font.pointSize: 10
                                    color: "#ffffff"
                                    verticalAlignment: Text.AlignVCenter
                                    horizontalAlignment: Text.AlignHCenter
                                }
                                onClicked: qsignalTaskButtonClick("downloadFile", lTextTaskID.text);
                            }
                        }
                        Column{
                            width: 5
                        }
                    }
                }
            }

            ProgressBar{
                id: progBarDownloadFile
                objectName: "progBarDownloadFile"
                anchors.top: parent.top
                anchors.topMargin: 5
                visible: false
                height: 10
                width: listViewTasksRect.width
                from: 0
                to: 10000
            }
        }

        Pane{
            id: paneUsersTasks
            objectName: "paneUsersTasks"
            width: 210
            anchors{
                top: dragBarTasks.bottom
                left: paneTasks.right
                bottom: parent.bottom
            }

            Label{
                anchors{
                    left: parent.left
                    right: parent.right
                }
                width: parent.width
                text: "           Пользователи"
                height: 30
            }

            Rectangle{
                anchors.fill: parent
                anchors.topMargin: 30
                anchors.bottomMargin: 90
                border.width: 2
                border.color: "#858585"
                color: "transparent"

                ListView{
                    anchors.margins: 3
                    id: listViewUsersTasks
                    objectName: "listViewUsersTasks"
                    anchors.fill: parent
                    highlight: Rectangle {
                        color: "transparent"
                        border.color: Universal.color(Universal.Taupe)
                        border.width: 2
                        radius: 5
                    }
                    spacing: 5
                    clip: true

                    model: UserModel {
                        list: allUserList
                    }

                    delegate: RowLayout{
                        id: rowLayoutTasks
                        width: listViewUsersTasks.width
                        Column{
                            width: 5
                        }

                        MouseArea{
                            hoverEnabled: false
                            Layout.preferredWidth: rowLayoutTasks.width
                            Layout.preferredHeight: col1.height
                            acceptedButtons: Qt.LeftButton | Qt.RightButton
                            onClicked: {
                                if(mouse.button === Qt.RightButton){
                                    contextMenu.popup();
                                }
                                listViewUsersTasks.currentIndex = index
                                qsignalTaskButtonClick("usersViewSelectionChanged", lTextID.text)
                            }
                            onDoubleClicked: {
                                lTextUserToolTip.show(lTextUser.text, 2000)
                            }
                            Row{
                                id:col1
                                Layout.preferredHeight: 25
                                Text {
                                    id: lTextUser
                                    width: 145
                                    height: 25
                                    verticalAlignment: Text.AlignVCenter
                                    text: (boss) ? ("<font color=\"#F0A30A\"><b>Имя(п): </b></font>") + name : ("<font color=\"#F0A30A\"><b>Имя: </b></font>" + name)
                                    color: "#ffffff"
                                    wrapMode: Text.Wrap
                                    font.pointSize: 9
                                    ToolTip{
                                        id: lTextUserToolTip
                                    }
                                }
                                Text {
                                    id: lTextID
                                    visible: false
                                    text: id
                                    color: "#ffffff"
                                }
                                Button{
                                    width: 20
                                    height: 25
                                    text: "+"
                                    hoverEnabled: true
                                    ToolTip.text: "Добавить задачу"
                                    ToolTip.delay: 1000
                                    ToolTip.visible: hovered
                                    ToolTip.timeout: 2000
                                    onClicked: {
                                        qsignalTaskButtonClick("addTask", lTextID.text);
                                    }
                                }
                            }
                        }

                        Column{
                            width: 5
                        }
                        Menu {
                            id: contextMenu
                            MenuItem {
                                text: "Назначить помощником"
                                onClicked: qsignalTaskButtonClick("makeAdmin", lTextID.text);
                            }
                            MenuItem {
                                text: "Назначить сотрудником"
                                onClicked: qsignalTaskButtonClick("makeWorker", lTextID.text);
                            }
                        }
                    }
                }
            }
            Button{
                id:bActive
                anchors{
                    bottom: bShowCompleted.top
                    left: parent.left
                    right: parent.right
                    bottomMargin: 5
                }
                height: 25
                text: "Показать активные"
                onClicked: {
                    listViewUsersTasks.currentIndex = -1;
                    qsignalTaskButtonClick("bActive", "");
                }
            }
            Button{
                id:bShowCompleted
                anchors{
                    bottom: bRemoveAllDone.top
                    left: parent.left
                    right: parent.right
                    bottomMargin: 5
                }
                height: 25
                text: "Показать выполненные"
                onClicked: {
                    listViewUsersTasks.currentIndex = -1;
                    qsignalTaskButtonClick("bShowCompleted", "");
                }
            }
            Button{
                id:bRemoveAllDone
                anchors{
                    bottom: parent.bottom
                    left: parent.left
                    right: parent.right
                }
                height: 25
                text: "Удалить выполненные"
                onClicked: {
                    listViewUsersTasks.currentIndex = -1;
                    qsignalTaskButtonClick("bRemoveAllDone", "");
                }
            }
        }

        Rectangle{
            id:dragBarTasks
            anchors{
                top: parent.top
                left: parent.left
                right: parent.right
            }
            gradient: Gradient{
                GradientStop { position: 0.0; color: "#303030" }
                GradientStop { position: 1.0; color: "#202020" }
            }
            height: 28
            // The central area for moving the application window
            // Here you already need to use the position both along the X axis and the Y axis
            MouseArea {
                anchors.fill: parent

                onPressed: {
                    previousX = mouseX
                    previousY = mouseY
                }

                onMouseXChanged: {
                    var dx = mouseX - previousX
                    tasksWindow.setX(tasksWindow.x + dx)
                }

                onMouseYChanged: {
                    var dy = mouseY - previousY
                    tasksWindow.setY(tasksWindow.y + dy)
                }
            }
            Label{
                anchors.centerIn: parent
                text: "Задачи"
            }

            Button{
                id: quitButtonTasks
                ToolTip.visible: hovered
                ToolTip.delay: 750
                ToolTip.text: "Закрыть"
                anchors{
                    right: parent.right
                    top: parent.top
                    rightMargin: 5
                    topMargin: 3
                }
                background: Rectangle{
                    color: quitButtonTasks.hovered ? "#8c0e07" : "#6c0905"
                    width: 22
                    height: 22
                    radius: 5
                }
                text: "<font size='1' color='#000000'>x</font>"
                onClicked: {
                    tasksWindow.hide();
                    qsignalTaskButtonClick("quitButtonTasks", "")
                }
            }
        }
    }
}
