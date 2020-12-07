import QtQuick 2.0
import QtQuick.Window 2.10
import QtQuick.Controls 2.10
import QtQuick.Controls.Universal 2.12
import QtQuick.Dialogs 1.2

Window {
    id: mainWindow
    height: 340
    width: 300
    visible: true
    flags: Qt.MSWindowsFixedSizeDialogHint

    signal qsignalOnButtonClicked(string msg)

    Pane{
        anchors.fill: parent
        Universal.theme: Universal.Dark
        Column{
            id: column1
            anchors.fill: parent
            anchors.topMargin: 30
            anchors.leftMargin: 30
            anchors.rightMargin: 30
            spacing: 4
            Label{
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Логин"
            }
            TextField{
                id: tfLogin
                objectName: "tfLogin"
                anchors.horizontalCenter: parent.horizontalCenter
                width: 200
                height: 30

                Keys.onEnterPressed: {
                    qsignalOnButtonClicked("login");
                }
                Keys.onReturnPressed: {
                    qsignalOnButtonClicked("login");
                }
            }
            Label{
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Пароль"
            }
            TextField{
                id: tfPassword
                objectName: "tfPassword"
                echoMode: TextInput.Password
                anchors.horizontalCenter: parent.horizontalCenter
                width: 200
                height: 30

                Keys.onEnterPressed: {
                    qsignalOnButtonClicked("login");
                }
                Keys.onReturnPressed: {
                    qsignalOnButtonClicked("login");
                }
            }
            CheckBox{
                id: cbRemember
                objectName: "cbRemember"
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Запомнить меня"
            }
            Row{
                spacing: 2
                Button{
                    text: "Войти"
                    width: column1.width/2 - 1
                    onClicked: { qsignalOnButtonClicked("login"); }
                }
                Button{
                    text: "Выход"
                    width: column1.width/2 - 1
                    onClicked: { Qt.quit() }
                }
            }
            Button{
                width: parent.width
                text: "Регистрация"
                onClicked: { qsignalOnButtonClicked("reg"); }
            }
            Button{
                width: parent.width
                text: "Настройки сервера"
                onClicked: { qsignalOnButtonClicked("loginOpt"); }
            }
        }
    }

    // Declare properties that will store the position of the mouse cursor
    property int previousX
    property int previousY

    MouseArea {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 30

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

    MessageDialog{
        id: loginDialog
        objectName: "loginDialog"
        title: "Ошибка"
    }
}
