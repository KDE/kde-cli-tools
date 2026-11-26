/*
    SPDX-FileCopyrightText: 2026 Tobias Fella <fella@posteo.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

Kirigami.PromptDialog {
    id: dialog

    title: i18nc("@title:window", "Create New File Type")
    standardButtons: Kirigami.Dialog.Ok | Kirigami.Dialog.Cancel

    Component.onCompleted: dialog.standardButton(Kirigami.Dialog.Ok).enabled = Qt.binding(() => groupCombo.editText.trim().length > 0 && typeField.text.trim().length > 0)

    // The group to preselect, if it exists
    property string defaultGroup

    onAboutToShow: {
        groupCombo.currentIndex = Math.max(0, groupCombo.find(defaultGroup || "application"));
        typeField.clear();
        typeField.forceActiveFocus();
    }

    signal typeCreated(string name)

    onAccepted: {
        const name = kcm.addType(groupCombo.editText, typeField.text);
        if (name.length > 0) {
            typeCreated(name);
        }
    }

    Kirigami.Form {
        Layout.fillWidth: true

        Kirigami.FormGroup {
            Kirigami.FormEntry {
                title: i18nc("@label:textbox", "File type:")
                subtitle: i18nc("@info", "Select or type the category and the name of the new file type. For instance, 'image' and 'custom' will create the file type 'image/custom'.")
                contentItem: RowLayout {
                    spacing: Kirigami.Units.smallSpacing

                    QQC2.ComboBox {
                        id: groupCombo

                        Layout.fillWidth: true
                        Layout.preferredWidth: 1
                        editable: true
                        model: kcm.groups
                        Accessible.name: i18nc("@label:listbox", "Group")
                    }

                    QQC2.Label {
                        text: "/"
                    }

                    QQC2.TextField {
                        id: typeField

                        Layout.fillWidth: true
                        Layout.preferredWidth: 1
                        placeholderText: i18nc("@info:placeholder", "Type name")
                        Accessible.name: i18nc("@label:textbox", "Type name")
                        onAccepted: {
                            if (dialog.standardButton(Kirigami.Dialog.Ok).enabled) {
                                dialog.accept();
                            }
                        }
                    }
                }
            }
        }
    }
}
