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
        // The header comes after the content in the tab focus chain, which follows the order of the children.
        // Re-append the content, so that Tab goes from the search field to the tree.
        const content = tree.parent.parent;
        const contentParent = content.parent;
        content.parent = null;
        content.parent = contentParent;
        if (!Kirigami.Settings.isMobile) {
            kcm.columnWidth = Kirigami.Units.gridUnit * 15;
        }
        kcm.push("FileTypeDetails.qml");
    }

    Connections {
        target: kcm

        function onCurrentTypeChanged(): void {
            if (!kcm.currentType) {
                kcm.currentIndex = 0;
            }
        }
    }

    actions: [
        Kirigami.Action {
            icon.name: "list-add"
            text: i18nc("@action:button", "Add…")
            tooltip: i18nc("@info:tooltip", "Add a new file type")
            Accessible.name: tooltip
            onTriggered: {
                newTypeDialog.defaultGroup = root.selectedGroup();
                newTypeDialog.open();
            }
            shortcut: StandardKey.New
        }
    ]

    // The group of the current row if it is a group, otherwise the group of the selected file type
    function selectedGroup(): string {
        const item = tree.selectionModel.currentIndex.valid ? tree.itemAtIndex(tree.selectionModel.currentIndex) : null;
        if (item?.isGroup) {
            return item.name;
        }
        return kcm.currentType.split("/")[0];
    }

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
        text: kcm.filteredModel.filterText
        onAccepted: {
            kcm.filteredModel.filterText = text;
            if (text.length > 0) {
                Qt.callLater(tree.expandRecursively);
            }
        }

        QQC2.ToolTip.text: i18nc("@info:tooltip", "Enter a part of a filename pattern, and only file types with a matching file pattern will appear in the list. Alternatively, enter a part of a file type name as it appears in the list.")
        QQC2.ToolTip.visible: (hovered || visualFocus) && text.length === 0
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
        activeFocusOnTab: true
        selectionModel: ItemSelectionModel {}

        onActiveFocusChanged: {
            if (activeFocus && !selectionModel.currentIndex.valid && rows > 0) {
                selectionModel.setCurrentIndex(index(0, 0), ItemSelectionModel.NoUpdate);
            }
        }

        Keys.onReturnPressed: event => activateCurrent(event)
        Keys.onEnterPressed: event => activateCurrent(event)
        Keys.onSpacePressed: event => activateCurrent(event)
        Keys.onTabPressed: event => moveFocus(true, event)
        Keys.onBacktabPressed: event => moveFocus(false, event)

        function moveFocus(forward: bool, event = null): void {
            nextItemInFocusChain(forward)?.forceActiveFocus(forward ? Qt.TabFocusReason : Qt.BacktabFocusReason);
            if (event) {
                event.accepted = true;
            }
        }

        function activateCurrent(event: var): void {
            const index = selectionModel.currentIndex;
            if (!index.valid) {
                return;
            }
            const row = rowAtIndex(index);
            const item = itemAtIndex(index);
            if (!item) {
                return;
            }
            if (item.isGroup) {
                toggleExpanded(row);
            } else {
                root.showDetails(item.name);
                // The details page follows the tree in the focus chain
                Qt.callLater(() => tree.moveFocus(true));
            }
            event.accepted = true;
        }

        delegate: QQC2.TreeViewDelegate {
            id: delegate

            // The tree view itself is the single tab stop; rows are navigated with the arrow keys
            activeFocusOnTab: false

            required property string name
            required property bool isGroup
            required property var iconName

            highlighted: (!isGroup && name === kcm.currentType) || (tree.activeFocus && current)

            onClicked: {
                if (!isGroup) {
                    root.showDetails(name);
                } else {
                    tree.toggleExpanded(row);
                }
            }

            icon.name: delegate.isGroup ? "" : delegate.iconName
            icon.width: Kirigami.Units.iconSizes.small
            icon.height: Kirigami.Units.iconSizes.small
            implicitHeight: Math.max(implicitContentHeight, Kirigami.Units.iconSizes.small) + topPadding + bottomPadding
            text: delegate.model.display

            QQC2.ToolTip.text: delegate.model.comment
            QQC2.ToolTip.visible: (hovered || visualFocus) && QQC2.ToolTip.text.length > 0
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
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

        parent: QQC2.Overlay.overlay

        onTypeCreated: name => root.revealType(name)
    }
}
