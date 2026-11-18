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

    onAboutToShow: {
        // "application" is certainly a better default than "all"
        groupCombo.currentIndex = Math.max(0, groupCombo.find("application"));
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
                title: i18nc("@label:listbox", "Group:")
                subtitle: i18nc("@info", "Select the category under which the new file type should be added.")
                contentItem: QQC2.ComboBox {
                    id: groupCombo

                    editable: true
                    model: kcm.groups
                }
            }

            Kirigami.FormEntry {
                title: i18nc("@label:textbox", "Type name:")
                subtitle: i18nc("@info", "Type the name of the file type. For instance, if you selected 'image' as category and you type 'custom' here, the file type 'image/custom' will be created.")
                contentItem: QQC2.TextField {
                    id: typeField

                    onAccepted: dialog.accept()
                }
            }
        }
    }
}
