import QtQuick 2.0
import QtQuick.Window 2.10
import QtQuick.Controls 2.10
import QtQuick.Controls.Universal 2.12
import QtQuick.Dialogs 1.2

Window {
    id: registerWnd
    height: 340
    width: 300
    visible: false
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
            }
            Label{
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Псевдоним"
            }
            TextField{
                id: tfNickname
                objectName: "tfNickname"
                anchors.horizontalCenter: parent.horizontalCenter
                width: 200
                height: 30
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
            }
            Label{
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Подтверждение пароля"
            }
            TextField{
                id: tfPasswordConfirm
                objectName: "tfPasswordConfirm"
                echoMode: TextInput.Password
                anchors.horizontalCenter: parent.horizontalCenter
                width: 200
                height: 30
            }

            Row{
                spacing: 2
                Button{
                    text: "Регистрация"
                    width: column1.width/2 - 1
                    onClicked: { qsignalOnButtonClicked("register"); }
                }
                Button{
                    text: "Отмена"
                    width: column1.width/2 - 1
                    onClicked: { qsignalOnButtonClicked("cancelReg"); }
                }
            }
        }
    }
    MessageDialog{
        id: msgDialog
        objectName: "msgDialog"
        title: "Ошибка"
    }
}
