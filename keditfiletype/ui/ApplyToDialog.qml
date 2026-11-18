// SPDX-FileCopyrightText: 2026 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

Kirigami.Dialog {
    id: dialog

    title: i18nc("@title:window", "Apply To…")

    standardButtons: Kirigami.Dialog.Ok | Kirigami.Dialog.Cancel
    preferredWidth: Kirigami.Units.gridUnit * 25

    property list<string> selected: []

    function setSelected(names: list<string>, value: bool): void {
        const rest = selected.filter(name => !names.includes(name));
        selected = value ? rest.concat(names) : rest;
    }

    onAboutToShow: selected = []

    onAccepted: kcm.applyApplicationsTo(selected)

    Item {
        implicitHeight: Kirigami.Units.gridUnit * 25

        TreeView {
            anchors.fill: parent
            clip: true
            model: kcm.model
            selectionBehavior: TableView.SelectionDisabled
            boundsBehavior: Flickable.StopAtBounds
            columnWidthProvider: () => width - (QQC2.ScrollBar.vertical.visible ? QQC2.ScrollBar.vertical.width : 0)

            QQC2.ScrollBar.vertical: QQC2.ScrollBar {}

            delegate: QQC2.TreeViewDelegate {
                id: delegate

                required property string name
                required property bool isGroup
                required property string iconName

                readonly property list<string> names: isGroup ? kcm.typesInGroup(name).filter(type => type !== kcm.currentType) : [name]
                readonly property bool isSource: !isGroup && name === kcm.currentType
                readonly property bool allSelected: names.length > 0 && names.every(type => dialog.selected.includes(type))

                onClicked: {
                    if (isSource) {
                        return;
                    }
                    dialog.setSelected(names, !allSelected);
                }

                contentItem: RowLayout {
                    spacing: Kirigami.Units.smallSpacing

                    QQC2.CheckBox {
                        enabled: !delegate.isSource
                        checked: delegate.allSelected
                        onToggled: dialog.setSelected(delegate.names, checked)
                    }

                    Kirigami.Icon {
                        visible: !delegate.isGroup
                        source: delegate.isGroup ? "" : delegate.iconName
                        implicitWidth: Kirigami.Units.iconSizes.small
                        implicitHeight: Kirigami.Units.iconSizes.small
                    }

                    QQC2.Label {
                        Layout.fillWidth: true
                        text: delegate.model.display
                        elide: Text.ElideRight
                    }
                }
            }
        }
    }
}
