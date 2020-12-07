import QtQuick 2.0
import QtQuick.Window 2.10
import QtQuick.Controls 2.10
import QtQuick.Controls.Universal 2.12
import QtQuick.Dialogs 1.2
import Qt.labs.platform 1.0

Window {
    id: wndOptions
    width: 400
    height: 365
    visible: false
    flags: Qt.MSWindowsFixedSizeDialogHint

    signal qsignalOnButtonClicked(string msg)

    Pane{
        Universal.theme: Universal.Dark
        anchors.fill: parent
        Column{
            id: column1
            anchors.fill: parent
            anchors.topMargin: 10
            anchors.leftMargin: 30
            anchors.rightMargin: 30
            spacing: 4
            Label{
                anchors.horizontalCenter: parent.horizontalCenter
                text: "IP-адрес сервера"
            }
            TextField{
                id: tfIP
                objectName: "tfIP"
                anchors.horizontalCenter: parent.horizontalCenter
                width: 250
                height: 30
            }
            Label{
                id:lFilesFolder
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Папка для сохранения файлов"
            }
            TextField{
                id: tfFilesDirectory
                objectName: "tfFilesDirectory"
                anchors.horizontalCenter: parent.horizontalCenter
                width: 250
                height: 30
            }
            Button{
                id: bExplore
                anchors.horizontalCenter: parent.horizontalCenter
                width: 250
                objectName: "bExplore"
                text: "Обзор"
                onClicked: optionsFolderDialog.open()
            }
            Label{
                id: lNickname
                objectName: "lNickname"
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Псевдоним"
            }
            TextField{
                id: tfNickname
                objectName: "tfNickname"
                anchors.horizontalCenter: parent.horizontalCenter
                width: 250
                height: 30
            }
            Button{
                id: bSaveNickname
                objectName: "bSaveNickname"
                text: "Обновить псевдоним"
                width: parent.width
                onClicked: qsignalOnButtonClicked("optChangeNickname")
            }
            Button{
                id: bChangePassword
                objectName: "bChangePassword"
                text: "Изменить пароль"
                width: parent.width
                onClicked: qsignalOnButtonClicked("bChangePassword")
            }

            Row{
                spacing: 2
                Button{
                    text: "Сохранить"
                    width: column1.width/2 - 1
                    onClicked: {
                        if(lNickname.visible){
                            qsignalOnButtonClicked("saveFromMainOpt");
                        }
                        else{
                            qsignalOnButtonClicked("saveFromLoginOpt");
                        }
                    }
                }
                Button{
                    text: "Закрыть"
                    width: column1.width/2 - 1
                    onClicked: { (lNickname.visible) ? qsignalOnButtonClicked("cancelMainOpt") : qsignalOnButtonClicked("cancelOpt"); }
                }
            }
        }
    }

    MessageDialog{
        id: dialogOptionsMiss
        objectName: "dialogOptionsMiss"
        title: "Педупреждение"
        text: "Произошла ошибка при чтении файла настроек."
        informativeText: "Введите параметры настроек."
    }
    MessageDialog{
        id: optionsDialog
        objectName: "optionsDialog"
        title: "Ошибка"
        text: "Произошла ошибка при сохранении параметров";
    }
    FolderDialog{
        id:optionsFolderDialog
        onAccepted: {
            var path = optionsFolderDialog.currentFolder.toString();
            // remove prefixed "file:///"
            path = path.replace(/^(file:\/{3})/,"");
            // unescape html codes like '%23' for '#'
            var clean_path = decodeURIComponent(path);
            tfFilesDirectory.text = clean_path
        }
    }
}
