// SPDX-FileCopyrightText: 2026 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.iconthemes as KIconThemes
import org.kde.kcmutils as KCMUtils

KCMUtils.SimpleKCM {
    id: root

    readonly property string currentType: kcm.currentType

    title: currentType.length > 0 ? currentType : i18nc("@title", "File Type Details")

    actions: [
        Kirigami.Action {
            icon.name: "list-remove"
            text: kcm.currentRemoveIsRevert ? i18nc("@action:button", "Revert") : i18nc("@action:button", "Remove")
            tooltip: kcm.currentRemoveIsRevert
                ? i18nc("@info:tooltip", "Revert this file type to its initial system-wide definition, which undoes any changes made to the file type. Note that system-wide file types cannot be deleted. You can however empty their pattern list, to minimize the chances of them being used (but the file type determination from file contents can still end up using them).")
                : i18nc("@info:tooltip", "Delete this file type definition completely. This is only possible for user-defined file types. System-wide file types cannot be deleted. You can however empty their pattern list, to minimize the chances of them being used (but the file type determination from file contents can still end up using them).")
            visible: root.currentType.length > 0
            enabled: kcm.currentCanRemove
            onTriggered: kcm.removeCurrentType()
            Accessible.name: tooltip
        }
    ]

    KIconThemes.IconDialog {
        id: iconDialog

        onIconNameChanged: iconName => kcm.setCurrentIcon(iconName)
    }

    ApplyToDialog {
        id: applyToDialog
    }

    Kirigami.PlaceholderMessage {
        parent: root
        anchors.centerIn: parent
        width: parent.width - Kirigami.Units.gridUnit * 2
        visible: root.currentType.length === 0
        icon.name: "preferences-desktop-filetype-association"
        text: i18nc("@info:placeholder", "Select a file type to edit it")
    }

    Kirigami.Form {
        visible: root.currentType.length > 0
        anchors.left: parent.left
        anchors.right: parent.right

        Kirigami.FormGroup {
            Kirigami.FormEntry {
                title: i18nc("@label:textbox", "Description:")
                contentItem: RowLayout {
                    spacing: Kirigami.Units.smallSpacing

                    QQC2.Button {
                        icon.name: kcm.currentIcon || "unknown"
                        icon.width: Kirigami.Units.iconSizes.large
                        icon.height: Kirigami.Units.iconSizes.large
                        onClicked: iconDialog.open()

                        QQC2.ToolTip.text: i18nc("@info:tooltip", "Change the icon of this file type")
                        QQC2.ToolTip.visible: hovered || visualFocus
                        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                        Accessible.name: QQC2.ToolTip.text
                    }

                    QQC2.TextField {
                        Layout.fillWidth: true
                        text: kcm.currentComment
                        onTextEdited: kcm.setCurrentComment(text)
                        placeholderText: i18nc("@info:placeholder", "The '%1' file type", root.currentType)
                        Accessible.name: i18nc("@label", "File type description")
                    }
                }
            }
        }

        Kirigami.FormGroup {
            title: i18nc("@title:group", "Filename Patterns")

            Kirigami.FormEntry {
                contentItem: QQC2.ScrollView {
                    Layout.fillWidth: true
                    Kirigami.StyleHints.showFramedBackground: true

                    implicitHeight: Math.min((patternsList.count > 0 ? patternsList.contentHeight + patternsList.topMargin + patternsList.bottomMargin : noPatternsMessage.implicitHeight + Kirigami.Units.gridUnit * 2), Kirigami.Units.gridUnit * 10) + topPadding + bottomPadding

                    contentItem: ListView {
                        id: patternsList

                        clip: true
                        boundsBehavior: Flickable.StopAtBounds
                        model: kcm.currentPatterns

                        Kirigami.PlaceholderMessage {
                            id: noPatternsMessage

                            parent: patternsList
                            anchors.centerIn: parent
                            width: parent.width - Kirigami.Units.gridUnit * 2
                            visible: patternsList.count === 0
                            text: i18nc("@info no filename patterns for this file type", "No patterns")
                        }

                        delegate: QQC2.ItemDelegate {
                            id: patternEntry

                            required property int index
                            required property string modelData

                            width: ListView.view.width - ListView.view.leftMargin - ListView.view.rightMargin

                            Kirigami.StyleHints.useAlternateBackgroundColor: true

                            Keys.onDeletePressed: kcm.removePattern(index)

                            contentItem: Kirigami.TitleSubtitleWithActions {
                                // Unicode chars that mark the entire string as LtR, to prevent "*.foo" turning into "foo.*" in RtL languages
                                title: "\u2066" + patternEntry.modelData + "\u2069"
                                actions: Kirigami.Action {
                                    icon.name: "list-remove"
                                    text: i18nc("@action:button", "Remove")
                                    onTriggered: kcm.removePattern(patternEntry.index)
                                    displayHint: Kirigami.DisplayHint.IconOnly
                                    tooltip: i18nc("@info:tooltip", "Remove this filename pattern")
                                    Accessible.name: tooltip
                                }
                            }
                        }
                    }
                }
            }

            Kirigami.FormEntry {
                contentItem: RowLayout {
                    QQC2.Button {
                        id: addPatternButton

                        icon.name: "list-add"
                        text: i18nc("@action:button", "Add…")
                        visible: !patternField.visible
                        onClicked: {
                            patternField.visible = true;
                            patternField.forceActiveFocus();
                        }

                        QQC2.ToolTip.text: i18nc("@info:tooltip", "Add a new pattern for the selected file type")
                        QQC2.ToolTip.visible: hovered || visualFocus
                        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                    }

                    Kirigami.ActionTextField {
                        id: patternField

                        function submit(): void {
                            if (text.length > 0) {
                                kcm.addPattern(text);
                            }
                            clear();
                            visible = false;
                        }

                        Layout.fillWidth: true
                        visible: false
                        placeholderText: i18nc("@info:placeholder", "*.txt")
                        onAccepted: submit()
                        Keys.onEscapePressed: {
                            clear();
                            visible = false;
                        }
                        rightActions: Kirigami.Action {
                            icon.name: "dialog-ok-apply-symbolic"
                            text: i18nc("@action:button", "Add Pattern")
                            enabled: patternField.text.length > 0
                            displayHint: Kirigami.DisplayHint.IconOnly
                            tooltip: i18nc("@info:tooltip", "Add this pattern to the selected file type")
                            onTriggered: patternField.submit()
                        }
                        onActiveFocusChanged: if (!activeFocus && text.length === 0) {
                            visible = false;
                        }
                    }
                }
            }
        }

        Kirigami.FormGroup {
            title: i18nc("@title:group", "Application Preference Order")

            Kirigami.FormEntry {
                contentItem: QQC2.ScrollView {
                    Layout.fillWidth: true
                    Kirigami.StyleHints.showFramedBackground: true

                    implicitHeight: Math.min((applicationsList.count > 0 ? applicationsList.contentHeight + applicationsList.topMargin + applicationsList.bottomMargin : noApplicationsMessage.implicitHeight + Kirigami.Units.gridUnit * 2), Kirigami.Units.gridUnit * 15) + topPadding + bottomPadding

                    contentItem: ListView {
                        id: applicationsList

                        // The margins add to the scrollable area, so account for them to not scroll sideways
                        contentWidth: width - leftMargin - rightMargin
                        clip: true
                        boundsBehavior: Flickable.StopAtBounds
                        // Local copy of the applications, so the list can be reordered live while dragging
                        // without recreating the delegates; the change is committed to the KCM once dropped
                        property int dragFrom: -1
                        property int dragTo: -1

                        function sync(): void {
                            const apps = kcm.currentApplications;
                            const same = apps.length === appsModel.count && apps.every((app, i) => app.storageId === appsModel.get(i).storageId);
                            if (same) {
                                return;
                            }
                            appsModel.clear();
                            apps.forEach(app => appsModel.append({storageId: app.storageId, name: app.name, icon: app.icon}));
                        }

                        model: ListModel {
                            id: appsModel
                        }

                        Component.onCompleted: sync()

                        Kirigami.PlaceholderMessage {
                            id: noApplicationsMessage

                            parent: applicationsList
                            anchors.centerIn: parent
                            width: parent.width - Kirigami.Units.gridUnit * 2
                            visible: applicationsList.count === 0
                            text: i18nc("No applications associated with this file type", "None")
                        }

                        Connections {
                            target: kcm

                            function onCurrentDataChanged(): void {
                                applicationsList.sync();
                            }
                        }

                        move: Transition {
                            NumberAnimation {
                                property: "y"
                                duration: Kirigami.Units.shortDuration
                            }
                        }
                        moveDisplaced: Transition {
                            NumberAnimation {
                                property: "y"
                                duration: Kirigami.Units.shortDuration
                            }
                        }

                        delegate: Item {
                            id: appEntry

                            required property int index
                            required property string name
                            required property string icon

                            width: applicationsList.width - applicationsList.leftMargin - applicationsList.rightMargin
                            implicitHeight: entryDelegate.implicitHeight
                            height: implicitHeight

                            QQC2.ItemDelegate {
                                id: entryDelegate

                                width: appEntry.width
                                x: parent === appEntry ? 0 : applicationsList.leftMargin
                                hoverEnabled: true
                                bottomPadding: topPadding

                                Kirigami.StyleHints.useAlternateBackgroundColor: true

                                Keys.onDeletePressed: kcm.removeApplication(appEntry.index)

                                contentItem: RowLayout {
                                    spacing: Kirigami.Units.smallSpacing

                                    Kirigami.ListItemDragHandle {
                                        id: dragHandle

                                        listItem: entryDelegate
                                        listView: applicationsList
                                        text: i18nc("@action:button", "Reorder")

                                        visible: applicationsList.count > 1

                                        QQC2.ToolTip.text: i18nc("@info:tooltip", "Drag to change the priority of this application. Applications further up take precedence. Note: This only affects the application if the file type is associated with more than one application.")
                                        QQC2.ToolTip.visible: hovered || visualFocus
                                        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay

                                        onMoveRequested: (oldIndex, newIndex) => {
                                            appsModel.move(oldIndex, newIndex, 1);
                                            if (dragActive) {
                                                if (applicationsList.dragFrom < 0) {
                                                    applicationsList.dragFrom = oldIndex;
                                                }
                                                applicationsList.dragTo = newIndex;
                                            } else {
                                                kcm.moveApplication(oldIndex, newIndex);
                                            }
                                        }
                                        onDropped: {
                                            if (applicationsList.dragFrom >= 0) {
                                                kcm.moveApplication(applicationsList.dragFrom, applicationsList.dragTo);
                                            }
                                            applicationsList.dragFrom = -1;
                                            applicationsList.dragTo = -1;
                                        }
                                    }

                                    Kirigami.IconTitleSubtitle {
                                        Layout.fillWidth: true
                                        Layout.minimumWidth: 0
                                        icon.name: appEntry.icon
                                        //TODO Qt6.12 use Qt.escapeHtml
                                        title: xi18nc("@info", "%1", appEntry.name)
                                        elide: Text.ElideRight
                                    }

                                    Kirigami.ActionToolBar {
                                        Layout.alignment: Qt.AlignVCenter
                                        Layout.minimumWidth: implicitWidth

                                        alignment: Qt.AlignRight

                                        flat: false

                                        actions: [
                                            Kirigami.Action {
                                                icon.name: "edit-rename"
                                                text: i18nc("@action:button", "Edit…")
                                                onTriggered: kcm.editApplication(appEntry.index)
                                                displayHint: Kirigami.DisplayHint.IconOnly
                                                tooltip: i18nc("@info:tooltip", "Edit this application")
                                                Accessible.name: tooltip
                                            },
                                            Kirigami.Action {
                                                icon.name: "list-remove"
                                                text: i18nc("@action:button", "Remove")
                                                tooltip: i18nc("@info:tooltip", "Remove this application from the preference order")
                                                onTriggered: kcm.removeApplication(appEntry.index)
                                                displayHint: Kirigami.DisplayHint.IconOnly
                                                Accessible.name: tooltip
                                            }
                                        ]
                                    }
                                }
                            }
                        }
                    }
                }
            }

            Kirigami.FormAction {
                action: Kirigami.Action {
                    icon.name: "list-add"
                    text: i18nc("@action:button", "Add Application…")
                    tooltip: i18nc("@info:tooltip", "Add a new application for this file type")
                    onTriggered: kcm.addApplication()
                    Accessible.name: tooltip
                }
            }

            Kirigami.FormAction {
                action: Kirigami.Action {
                    icon.name: "edit-copy"
                    text: i18nc("@action:button", "Apply To…")
                    tooltip: i18nc("@info:tooltip", "Apply the current preference order to other file types")
                    onTriggered: applyToDialog.open()
                    Accessible.name: tooltip
                }
            }
        }
    }
}
