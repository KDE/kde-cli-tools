// SPDX-FileCopyrightText: 2026 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kcmutils as KCMUtils

KCMUtils.ScrollViewKCM {
    id: root

    sidebarMode: !Kirigami.Settings.isMobile

    Component.onCompleted: {
        if (!Kirigami.Settings.isMobile) {
            kcm.columnWidth = Kirigami.Units.gridUnit * 15;
        }
        kcm.push("FileTypeDetails.qml");
        Qt.callLater(() => kcm.currentIndex = 0);
    }

    Connections {
        target: kcm

        function onCurrentTypeChanged(): void {
            if (kcm.currentType.length === 0) {
                kcm.currentIndex = 0;
            }
        }
    }

    actions: [
        Kirigami.Action {
            icon.name: "list-add"
            text: i18nc("@action:button", "Add…")
            tooltip: i18nc("@info:tooltip", "Add a new file type")
            onTriggered: newTypeDialog.open()
        }
    ]

    function showDetails(name: string): void {
        kcm.currentType = name;
        kcm.currentIndex = 1;
    }

    function revealType(name: string): void {
        showDetails(name);
        Qt.callLater(() => {
            const index = kcm.filteredModel.indexForName(name);
            if (!index.valid) {
                return;
            }
            tree.expandToIndex(index);
            tree.positionViewAtRow(tree.rowAtIndex(index), TableView.Contain);
        });
    }

    header: Kirigami.SearchField {
        id: searchField

        Layout.fillWidth: true
        placeholderText: i18nc("@info:placeholder", "Search for file type or filename pattern…")
        text: kcm.filteredModel.filterText
        onTextChanged: {
            kcm.filteredModel.filterText = text;
            if (text.length > 0) {
                Qt.callLater(tree.expandRecursively);
            }
        }

        QQC2.ToolTip.text: i18nc("@info:tooltip", "Enter a part of a filename pattern, and only file types with a matching file pattern will appear in the list. Alternatively, enter a part of a file type name as it appears in the list.")
        QQC2.ToolTip.visible: hovered && text.length === 0
        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
    }

    view: TreeView {
        id: tree

        clip: true
        model: kcm.filteredModel
        Layout.fillWidth: true
        Layout.fillHeight: true
        boundsBehavior: Flickable.StopAtBounds
        columnWidthProvider: () => width

        onWidthChanged: forceLayout()

        delegate: QQC2.TreeViewDelegate {
            id: delegate

            required property string name
            required property bool isGroup
            required property var iconName

            highlighted: !isGroup && name === kcm.currentType

            onClicked: {
                if (!isGroup) {
                    root.showDetails(name);
                } else {
                    tree.toggleExpanded(row);
                }
            }

            contentItem: RowLayout {
                spacing: Kirigami.Units.smallSpacing

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
                    color: delegate.highlighted ? Kirigami.Theme.highlightedTextColor : Kirigami.Theme.textColor
                }
            }
        }

        Kirigami.PlaceholderMessage {
            parent: tree
            anchors.centerIn: parent
            width: parent.width - Kirigami.Units.gridUnit * 2
            visible: tree.rows === 0
            text: i18nc("@info:placeholder", "No matching file types")
        }
    }

    NewTypeDialog {
        id: newTypeDialog

        onTypeCreated: name => root.revealType(name)
    }
}
