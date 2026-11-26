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

    parent: QQC2.Overlay.overlay

    property list<string> selected: []

    function setSelected(names: list<string>, value: bool): void {
        const rest = selected.filter(name => !names.includes(name));
        selected = value ? rest.concat(names) : rest;
    }

    onAboutToShow: selected = []

    onAccepted: kcm.applyApplicationsTo(selected)

    contentItem: TreeView {
        id: tree

        implicitHeight: Kirigami.Units.gridUnit * 20
        implicitWidth: Kirigami.Units.gridUnit * 25

        clip: true
        model: kcm.model
        selectionBehavior: TableView.SelectionDisabled
        boundsBehavior: Flickable.StopAtBounds
        columnWidthProvider: () => width - (verticalScrollBar.visible ? verticalScrollBar.width : 0)
        onWidthChanged: forceLayout()

        QQC2.ScrollBar.vertical: QQC2.ScrollBar {
            id: verticalScrollBar

            onVisibleChanged: tree.forceLayout()
            onWidthChanged: tree.forceLayout()
        }

        delegate: QQC2.TreeViewDelegate {
            id: delegate

            required property string name
            required property bool isGroup
            required property string iconName

            readonly property list<string> names: isGroup ? kcm.typesInGroup(name).filter(type => type !== kcm.currentType) : [name]
            readonly property bool isSource: !isGroup && name === kcm.currentType
            readonly property bool allSelected: names.length > 0 && names.every(type => dialog.selected.includes(type))
            readonly property bool someSelected: names.length > 0 && names.some(type => dialog.selected.includes(type))

            onClicked: {
                if (isSource) {
                    return;
                }
                dialog.setSelected(names, !allSelected);
            }

            QQC2.ToolTip.text: delegate.model.comment
            QQC2.ToolTip.visible: (hovered || visualFocus) && QQC2.ToolTip.text.length > 0
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay

            contentItem: QQC2.CheckDelegate {
                background: null
                horizontalPadding: 0
                topPadding: 0
                bottomPadding: 0
                enabled: !delegate.isSource
                text: delegate.model.display
                icon.name: delegate.isGroup ? "" : delegate.iconName
                icon.width: Kirigami.Units.iconSizes.small
                icon.height: Kirigami.Units.iconSizes.small
                tristate: true
                checkState: delegate.allSelected ? Qt.Checked : (delegate.someSelected ? Qt.PartiallyChecked : Qt.Unchecked)
                nextCheckState: () => checkState === Qt.Checked ? Qt.Unchecked : Qt.Checked
                onClicked: dialog.setSelected(delegate.names, checkState === Qt.Checked)
            }
        }
    }
}
