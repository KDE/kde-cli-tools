// SPDX-FileCopyrightText: 2026 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

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
                title: i18nc("@label", "Icon:")
                contentItem: QQC2.Button {
                    icon.name: kcm.currentIcon
                    icon.width: Kirigami.Units.iconSizes.large
                    icon.height: Kirigami.Units.iconSizes.large
                    onClicked: iconDialog.open()
                }
            }

            Kirigami.FormEntry {
                title: i18nc("@label:textbox", "Description:")
                contentItem: QQC2.TextField {
                    text: kcm.currentComment
                    onTextEdited: kcm.setCurrentComment(text)
                }
            }
        }

        Kirigami.FormGroup {
            title: i18nc("@title:group", "Filename Patterns")

            Kirigami.FormEntry {
                visible: kcm.currentPatterns.length === 0
                contentItem: QQC2.Label {
                    text: i18nc("@info no filename patterns for this file type", "No patterns")
                    opacity: 0.7
                }
            }

            Kirigami.FormEntry {
                visible: kcm.currentPatterns.length > 0
                contentItem: QQC2.ScrollView {
                    Layout.fillWidth: true
                    Kirigami.StyleHints.showFramedBackground: true

                    implicitHeight: Math.min(patternsList.contentHeight + patternsList.topMargin + patternsList.bottomMargin, Kirigami.Units.gridUnit * 10) + topPadding + bottomPadding

                    contentItem: ListView {
                        id: patternsList

                        contentWidth: width - leftMargin - rightMargin
                        clip: true
                        topMargin: Kirigami.Units.smallSpacing
                        bottomMargin: Kirigami.Units.smallSpacing
                        leftMargin: Kirigami.Units.smallSpacing
                        rightMargin: Kirigami.Units.smallSpacing
                        boundsBehavior: Flickable.StopAtBounds
                        model: kcm.currentPatterns

                        delegate: RowLayout {
                            id: patternEntry

                            required property int index
                            required property string modelData

                            width: ListView.view.width - ListView.view.leftMargin - ListView.view.rightMargin

                            QQC2.Label {
                                Layout.fillWidth: true
                                // Unicode chars that mark the entire string as LtR, to prevent "*.foo" turning into "foo.*" in RtL languages
                                text: "\u2066" + patternEntry.modelData + "\u2069"
                                elide: Text.ElideRight
                            }

                            QQC2.ToolButton {
                                icon.name: "list-remove"
                                text: i18nc("@action:button", "Remove")
                                display: QQC2.AbstractButton.IconOnly
                                onClicked: kcm.removePattern(patternEntry.index)

                                QQC2.ToolTip.text: i18nc("@info:tooltip", "Remove this filename pattern")
                                QQC2.ToolTip.visible: hovered
                                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                            }
                        }
                    }
                }
            }

            Kirigami.FormSeparator {}

            Kirigami.FormEntry {
                title: i18nc("@label:textbox", "New pattern:")
                contentItem: RowLayout {
                    QQC2.TextField {
                        id: patternField

                        Layout.fillWidth: true
                        placeholderText: i18nc("@info:placeholder", "*.txt")
                        onAccepted: addPatternButton.clicked()
                    }

                    QQC2.Button {
                        id: addPatternButton

                        icon.name: "list-add"
                        text: i18nc("@action:button", "Add")
                        enabled: patternField.text.length > 0
                        onClicked: {
                            kcm.addPattern(patternField.text);
                            patternField.clear();
                        }

                        QQC2.ToolTip.text: i18nc("@info:tooltip", "Add a new pattern for the selected file type")
                        QQC2.ToolTip.visible: hovered
                        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                    }
                }
            }
        }

        Kirigami.FormGroup {
            title: i18nc("@title:group", "Application Preference Order")

            Kirigami.FormEntry {
                visible: kcm.currentApplications.length === 0
                contentItem: QQC2.Label {
                    text: i18nc("No applications associated with this file type", "None")
                    opacity: 0.7
                }
            }

            Kirigami.FormEntry {
                visible: kcm.currentApplications.length > 0
                contentItem: QQC2.ScrollView {
                    Layout.fillWidth: true
                    Kirigami.StyleHints.showFramedBackground: true

                    implicitHeight: Math.min(applicationsList.contentHeight + applicationsList.topMargin + applicationsList.bottomMargin, Kirigami.Units.gridUnit * 15) + topPadding + bottomPadding

                    contentItem: ListView {
                        id: applicationsList

                        // The margins add to the scrollable area, so account for them to not scroll sideways
                        contentWidth: width - leftMargin - rightMargin
                        clip: true
                        topMargin: Kirigami.Units.smallSpacing
                        bottomMargin: Kirigami.Units.smallSpacing
                        leftMargin: Kirigami.Units.smallSpacing
                        rightMargin: Kirigami.Units.smallSpacing
                        boundsBehavior: Flickable.StopAtBounds
                        model: kcm.currentApplications

                        delegate: RowLayout {
                            id: appEntry

                            required property int index
                            required property var modelData

                            width: ListView.view.width - ListView.view.leftMargin - ListView.view.rightMargin
                            spacing: Kirigami.Units.smallSpacing

                            Kirigami.IconTitleSubtitle {
                                Layout.fillWidth: true
                                icon.name: appEntry.modelData.icon
                                //TODO Qt6.12 use Qt.escapeHtml
                                title: xi18nc("@info", "%1", appEntry.modelData.name)
                            }

                            QQC2.ToolButton {
                                icon.name: "arrow-up"
                                text: i18nc("@action:button", "Move Up")
                                display: QQC2.AbstractButton.IconOnly
                                enabled: appEntry.index > 0
                                onClicked: kcm.moveApplication(appEntry.index, appEntry.index - 1)

                                QQC2.ToolTip.text: i18nc("@info:tooltip", "Assigns a higher priority to this application, moving it up in the list. Note: This only affects the application if the file type is associated with more than one application.")
                                QQC2.ToolTip.visible: hovered
                                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                            }

                            QQC2.ToolButton {
                                icon.name: "arrow-down"
                                text: i18nc("@action:button", "Move Down")
                                display: QQC2.AbstractButton.IconOnly
                                enabled: appEntry.index < applicationsList.count - 1
                                onClicked: kcm.moveApplication(appEntry.index, appEntry.index + 1)

                                QQC2.ToolTip.text: i18nc("@info:tooltip", "Assigns a lower priority to this application, moving it down in the list. Note: This only affects the application if the file type is associated with more than one application.")
                                QQC2.ToolTip.visible: hovered
                                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                            }

                            QQC2.ToolButton {
                                icon.name: "edit-rename"
                                text: i18nc("@action:button", "Edit…")
                                display: QQC2.AbstractButton.IconOnly
                                onClicked: kcm.editApplication(appEntry.index)

                                QQC2.ToolTip.text: i18nc("@info:tooltip", "Edit command line of this application")
                                QQC2.ToolTip.visible: hovered
                                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                            }

                            QQC2.ToolButton {
                                icon.name: "list-remove"
                                text: i18nc("@action:button", "Remove")
                                display: QQC2.AbstractButton.IconOnly
                                onClicked: kcm.removeApplication(appEntry.index)

                                QQC2.ToolTip.text: i18nc("@info:tooltip", "Remove this application from the list")
                                QQC2.ToolTip.visible: hovered
                                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                            }
                        }
                    }
                }
            }

            Kirigami.FormSeparator {}

            Kirigami.FormAction {
                action: Kirigami.Action {
                    icon.name: "list-add"
                    text: i18nc("@action:button", "Add Application…")
                    tooltip: i18nc("@info:tooltip", "Add a new application for this file type")
                    onTriggered: kcm.addApplication()
                }
            }

            Kirigami.FormAction {
                action: Kirigami.Action {
                    icon.name: "edit-copy"
                    text: i18nc("@action:button", "Apply To…")
                    tooltip: i18nc("@info:tooltip", "Apply the current preference order to other file types")
                    onTriggered: applyToDialog.open()
                }
            }
        }
    }
}
