import QtQuick 2.10
import QtQuick.Window 2.10
import QtQuick.Controls 2.10
import Qt.labs.platform 1.0 as Platform
import QtQuick.Controls.Universal 2.12
import QtGraphicalEffects 1.0
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.2

import Models 1.0

Window {
    id:mainWindow
    objectName: "mainWindow"
    visible: false
    width: 970
    height: 600
    title: qsTr("Чат")
    //500 - основная зона
    //260 - файлы
    //210 - пользователи

    flags: Qt.FramelessWindowHint | Qt.Window
    Universal.theme: Universal.Dark

    //СИГНАЛЫ
    signal qsignalButtonClicked(string msg)
    signal qsignalSendButtonClicked(string msg)
    signal qsignalFileChosen(string file)

    // Declare properties that will store the position of the mouse cursor
    property int previousX
    property int previousY

    onSceneGraphInitialized: setWindowSize()

    Platform.SystemTrayIcon{
        id:trayIcon
        objectName: "trayIcon"
        visible: true
        iconSource: "qrc:/img/mainicon.png"

        menu: Platform.Menu {
            id:trayMenu
            Platform.MenuItem {
                text: "Выход"
                onTriggered: Qt.quit()
            }
        }
        onActivated: {
            if(reason === 1){
                trayMenu.open();
            }
            else{
                if(mainWindow.visibility == Window.Hidden)
                {
                    mainWindow.show();
                    mainWindow.raise();
                    qsignalButtonClicked("tryIcon");
                }
            }
        }
    }

    Pane {
        id: paneMenuBar
        anchors.right: parent.right
        anchors.left: parent.left
        anchors.top: dragBar.bottom
        anchors.topMargin: -10
        anchors.leftMargin: -10
        height: 50
        MenuBar {
            MenuBarItem{
                text: "Настройки"
                onClicked: qsignalButtonClicked("mainOptions");
            }
            MenuBarItem{
                text: "Очистить чат"
                onClicked: mainTextArea.clear()
            }
            MenuBarItem{
                text: "Выйти из аккаунта"
                onClicked: qsignalButtonClicked("logout");
            }
        }
    }

    Pane{
        id: paneMainArea
        width: 500
        anchors{
            top: paneMenuBar.bottom
            left: parent.left
            right: paneUsers.left
            bottom: parent.bottom
        }
        Row {
            id: topRow
            height: 35
            anchors.right: parent.right
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.topMargin: 5
            anchors.leftMargin: 5
            anchors.rightMargin: 5
            spacing: 2
            Button{
                id: bUsers
                Universal.accent: Universal.Taupe
                text: "Пользователи  "
                width: 154
                checkable: true
                onClicked: {
                    qsignalButtonClicked("users")
                    setWindowSize()
                }
                Image {
                    id: usersButtonImage
                    anchors{
                        right: parent.right
                        top: parent.top
                        bottom: parent.bottom
                        rightMargin: 5
                        topMargin: 5
                        bottomMargin: 5
                    }
                    width: 20
                    source: "qrc:/img/users.png"
                }
            }
            Button{
                id: bFiles
                Universal.accent: Universal.Taupe
                text: "Вложения  "
                width: 154
                onClicked: {
                    qsignalButtonClicked("files additions");
                }
                Image {
                    id: filesButtonImage
                    anchors{
                        right: parent.right
                        top: parent.top
                        bottom: parent.bottom
                        rightMargin: 5
                        topMargin: 5
                        bottomMargin: 5
                    }
                    width: 20
                    source: "qrc:/img/additions.png"
                }
            }
            Button{
                id: bTasks
                objectName: "bTasks"
                Universal.accent: Universal.Taupe
                text: "Задачи  "
                width: 154
                onClicked: {
                    qsignalButtonClicked("tasks");
                }
                Image{
                    anchors{
                        right: parent.right
                        top: parent.top
                        bottom: parent.bottom
                        rightMargin: 5
                        topMargin: 5
                        bottomMargin: 5
                    }
                    width: 20
                    source: "qrc:/img/task.png"
                }
            }
        }

        Row {
            id: mainAreaRow
            anchors.bottomMargin: 5
            anchors.topMargin: 5
            anchors.leftMargin: 5
            anchors.rightMargin: 5
            anchors.bottom: bottomAreaRow.top
            anchors.right: parent.right
            anchors.left: parent.left
            anchors.top: topRow.bottom
            //Скролящаяся TextArea хороший пример с автопрокруткой
            Flickable{
                id: flickableTextArea
                height: parent.height
                width: parent.width
                clip: true
                flickDeceleration: Flickable.VerticalFlick
                ScrollBar.vertical: ScrollBar{}
                ScrollBar.horizontal: null

                TextArea.flickable: TextArea{
                    id: mainTextArea
                    objectName: "mainTextArea"
                    Universal.theme: Universal.Dark
                    Universal.accent: Universal.Steel
                    wrapMode: TextEdit.Wrap
                    readOnly: true
                    selectByMouse: true

                    onTextChanged: {
                        mainTextArea.cursorPosition = mainTextArea.length
                    }
                }
            }
        }

        Row {
            id: bottomAreaRow
            height: 40
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: 5
            anchors.rightMargin: 5
            spacing: 2
            TextField{
                id: mainInput
                objectName: "mainInput"
                Universal.theme: Universal.Dark
                Universal.accent: Universal.Steel
                height: 40
                width: parent.width - sendButton.width - addFileButton.width - progressBarFile.width - 4 - ((progressBarFile.width > 0) ? 2 : 0)
                font.pointSize: 12

                Keys.onEnterPressed: {
                    qsignalSendButtonClicked(mainInput.text)
                    mainInput.clear()
                }
                Keys.onReturnPressed: {
                    qsignalSendButtonClicked(mainInput.text)
                    mainInput.clear()
                }
            }
            ProgressBar{
                id:progressBarFile
                objectName: "progressBarFile"
                width: 0
                height: 40
                from: 0
                to: 100000
            }

            Button{
                id:sendButton
                height: 40
                width: 40
                ToolTip.text: qsTr("Отправить")
                ToolTip.visible: hovered
                onClicked: {
                    qsignalSendButtonClicked(mainInput.text)
                    mainInput.clear()
                }
                Image{
                    anchors.fill: parent
                    anchors.margins: 5
                    source: "qrc:/img/send.png"
                }
            }
            Button{
                id:addFileButton
                height: 40
                width: 40
                ToolTip.text: qsTr("Отправить файл")
                ToolTip.visible: hovered
                onClicked: {
                    fileDialog.open()
                }
                Image{
                    anchors.fill: parent
                    anchors.margins: 8
                    source: "qrc:/img/add.png"
                }
            }
        }
    }

    Pane{
        id: paneUsers
        width: 210
        anchors{
            top: paneMenuBar.bottom
            left: paneMainArea.right
            bottom: parent.bottom
        }

        Label{
            id:lUsers
            anchors{
                left: parent.left
                right: parent.right
            }
            width: parent.width
            text: "    Пользователи онлайн"
            height: 30
        }

        Rectangle{
            anchors.fill: parent
            anchors.topMargin: 30
            border.width: 2
            border.color: "#858585"
            color: "transparent"

            ListView{
                anchors.margins: 3
                id: listViewUsers
                objectName: "listViewUsers"
                anchors.fill: parent
                clip: true

                model: UserModel {
                    list: userList
                }
                delegate: RowLayout{
                    id: rowLayout
                    width: listViewUsers.width
                    Column{
                        width: 1
                    }
                    MouseArea{
                        id: lMouseAreaUser
                        hoverEnabled: false
                        Layout.preferredWidth: lColumnUsers.width
                        Layout.preferredHeight: lColumnUsers.height
                        onClicked: {
                            listViewUsers.currentIndex = index                            
                        }
                        onDoubleClicked: {
                            lTextUserToolTip.show(lTextUser.text, 2000)
                        }
                        Column{
                            id: lColumnUsers
                            width: 180
                            Text {
                                id: lTextUser
                                height: 20
                                verticalAlignment: Text.AlignVCenter
                                text: "<font color=\"#F0A30A\"><b>Имя: </b></font>" + name
                                color: "#ffffff"
                                wrapMode: Text.Wrap
                                font.pointSize: 9
                                ToolTip{
                                    id: lTextUserToolTip
                                }
                            }
                        }

                    }
                    Column{
                        width: 1
                    }
                }
            }
        }
    }

    //мышинные области
    MouseArea {
        id: bottomArea
        height: 2
        anchors {
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }
        cursorShape: Qt.SizeVerCursor

        onPressed: {
            previousY = mouseY
        }

        onMouseYChanged: {
            var dy = mouseY - previousY
            mainWindow.setHeight(mainWindow.height + dy)
        }
    }

    Rectangle{
        id:dragBar
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
                mainWindow.setX(mainWindow.x + dx)
            }

            onMouseYChanged: {
                var dy = mouseY - previousY
                mainWindow.setY(mainWindow.y + dy)
            }
        }
        Label{
            anchors.centerIn: parent
            text: "Чат QML Version 2.0"
        }

        Button{
            id: quitButton
            ToolTip.visible: hovered
            ToolTip.delay: 750
            ToolTip.text: "Свернуть в трей"
            anchors{
                right: parent.right
                top: parent.top
                rightMargin: 5
                topMargin: 3
            }
            background: Rectangle{
                color: quitButton.hovered ? "#8c0e07" : "#6c0905"
                width: 22
                height: 22
                radius: 5
            }
            text: "<font size='1' color='#000000'>x</font>"
            onClicked: mainWindow.hide()
        }
        Button{
            id: minimizeButton
            ToolTip.visible: hovered
            ToolTip.delay: 750
            ToolTip.text: "Свернуть"
            anchors{
                right: parent.right
                top: parent.top
                rightMargin: 30
                topMargin: 3
            }
            background: Rectangle{
                color: minimizeButton.hovered ? "#9c5e07" : "#7c5905"
                width: 22
                height: 22
                radius: 5
            }
            text: "<font size='1' color='#000000'>_</font>"
            onClicked: mainWindow.visibility = Window.Minimized
        }
        Button{
            id: aboutButton
            ToolTip.visible: hovered
            ToolTip.delay: 750
            ToolTip.text: "О программе"
            anchors{
                right: parent.right
                top: parent.top
                rightMargin: 55
                topMargin: 3
            }
            background: Rectangle{
                color: aboutButton.hovered ? "#9c5e07" : "#7c5905"
                width: 22
                height: 22
                radius: 5
            }
            text: "<font size='1' color='#000000'>?</font>"
            onClicked: qsignalButtonClicked("about");
        }
    }

    MouseArea {
        id: topArea
        height: 2
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        // We set the shape of the cursor so that it is clear that this resizing
        cursorShape: Qt.SizeVerCursor

        onPressed: {
            // We memorize the position along the Y axis
            previousY = mouseY
        }

        // When changing a position, we recalculate the position of the window, and its height
        onMouseYChanged: {
            var dy = mouseY - previousY
            mainWindow.setY(mainWindow.y + dy)
            mainWindow.setHeight(mainWindow.height - dy)
        }
    }

    FileDialog{
        id: fileDialog
        folder: "."
        title: "Выберите файл для отправки"
        selectMultiple: false
        nameFilters: ["All files (*)"]
        onAccepted: {
            var path = fileDialog.fileUrl.toString();
            // remove prefixed "file:///"
            path = path.replace(/^(file:\/{3})/,"");
            // unescape html codes like '%23' for '#'
            var clean_path = decodeURIComponent(path);
            qsignalFileChosen(clean_path);
        }
    }

    MessageDialog{
        id:msgDialog
        objectName: "msgDialog"
        title: "Ошибка!"
    }

    DropArea{
        id: wndDropArea
        anchors.fill: parent
        onEntered: {
            drag.accept(Qt.LinkAction)
        }
        onDropped: {
            var count = 0;
            if(drop.hasUrls){
                for(var i in drop.urls){
                    count += 1;
                }
                if(count > 1)
                {
                    msgDialog.text = "Можно отправить только по 1 файлу!";
                    msgDialog.open();
                }
                else
                {
                    var path = drop.urls[0].toString();
                    // remove prefixed "file:///"
                    path = path.replace(/^(file:\/{3})/,"");
                    // unescape html codes like '%23' for '#'
                    var clean_path = decodeURIComponent(path);
                    qsignalFileChosen(clean_path);
                }
            }
        }
    }

    function setWindowSize(){
        if(!bUsers.checked)
        {
            mainWindow.width = 500
            paneUsers.width = 0
            paneUsers.enabled = false
        }
        else if(bUsers.checked)
        {
            mainWindow.width = 710
            paneUsers.width = 210
            paneUsers.enabled = true
        }
    }
}
