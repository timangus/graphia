/* Copyright © 2013-2022 Graphia Technologies Ltd.
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.14
import QtQml 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.3
import QtQml.Models 2.13
import QtQuick.Shapes 1.13

import Qt.labs.platform 1.0 as Labs

import app.graphia 1.0
import app.graphia.Shared 1.0

Item
{
    id: root

    property var model
    property int defaultColumnWidth: 120
    property var selectedRows: []
    property alias rowCount: tableView.rows
    property alias sortIndicatorColumn: proxyModel.sortColumn
    property alias sortIndicatorOrder: proxyModel.sortOrder

    function initialise()
    {
        root.model.columnNames.forEach(function(columnName)
        {
            if(root.model.columnIsHiddenByDefault(columnName))
                setColumnVisibility(columnName, false);
        });

        tableView._updateColumnVisibility();
    }

    function resizeColumnsToContents()
    {
        // Resizing columns to contents is difficult with TableView2. The solution here
        // is to connect the tableview delegates to a signal so they report their implicitWidths
        // when the fetchColumnSizes signal is emitted. This will only fetch the sizes for the delegates
        // currently on screen. We store previous widths calculated in the currentColumnWidths array
        // and partially update them each time this function is called to best-guess offscreen columns
        tableView.userColumnWidths = [];
        tableView.fetchColumnSizes();
        if(tableView.columnWidths.length > 0)
        {
            // Update array
            for(let i = 0; i < tableView.columns; i++)
            {
                let newValue = tableView.columnWidths[i];
                if(newValue !== undefined)
                    tableView.currentColumnWidths[i] = newValue;
            }

            let tempArr = tableView.currentColumnWidths;
            tableView.currentColumnWidths = [];
            tableView.currentColumnWidths = tempArr;

            tableView.columnWidths = [];
        }
        tableView.currentTotalColumnWidth = 0;
        for(let i = 0; i < tableView.columns; i++)
        {
            let tempCalculatedWidth = tableView.calculateMinimumColumnWidth(i);
            tableView.currentTotalColumnWidth += tempCalculatedWidth;
        }

        tableView.forceLayoutSafe();
    }

    property var hiddenColumns: []
    function setHiddenColumns(hiddenColumns)
    {
        // Filter out any columns that don't exist in the table
        hiddenColumns = hiddenColumns.filter(v => root.model.columnNames.includes(v));

        root.hiddenColumns = hiddenColumns;
        tableView._updateColumnVisibility();
    }

    property alias columnOrder: proxyModel.columnOrder

    property bool columnSelectionMode: false
    onColumnSelectionModeChanged:
    {
        let indexArray = Array.from(new Array(root.model.columnNames.length).keys());

        if(columnSelectionMode)
            columnSelectionControls.show();
        else
            columnSelectionControls.hide();

        // Reset the scroll position in case the new visible columns are no longer in view
        horizontalTableViewScrollBar.position = 0;

        selectionModel.clear();
        root.selectedRows = [];

        tableView._updateColumnVisibility();
        tableView.forceLayoutSafe();
    }

    function setColumnVisibility(columnName, columnVisible)
    {
        if(columnVisible)
            hiddenColumns = Utils.setRemove(hiddenColumns, columnName);
        else
            hiddenColumns = Utils.setAdd(hiddenColumns, columnName);
    }

    function showAllColumns()
    {
        hiddenColumns = [];
    }

    function showAllCalculatedColumns()
    {
        let columns = hiddenColumns;
        hiddenColumns = [];
        root.model.columnNames.forEach(function(columnName)
        {
            if(root.model.columnIsCalculated(columnName))
                columns = Utils.setRemove(columns, columnName);
        });

        hiddenColumns = columns;
    }

    function hideAllColumns()
    {
        let columns = Array.from(root.model.columnNames);
        hiddenColumns = columns;
    }

    function hideAllCalculatedColumns()
    {
        let columns = hiddenColumns;
        hiddenColumns = [];
        root.model.columnNames.forEach(function(columnName)
        {
            if(root.model.columnIsCalculated(columnName))
                columns = Utils.setAdd(columns, columnName);
        });

        hiddenColumns = columns;
    }

    function populateTableMenu(menu)
    {
        if(menu === null)
            return;

        // Clear out any existing items
        while(menu.count > 0)
            menu.takeItem(0);

        menu.title = qsTr("&Table");

        Utils.addActionTo(menu, resizeColumnsToContentsAction);
        Utils.addActionTo(menu, selectColumnsAction);
        Utils.addActionTo(menu, exportTableAction);
        Utils.addSeparatorTo(menu);
        Utils.addActionTo(menu, selectAllTableAction);

        tableView._tableMenu = menu;
        Utils.cloneMenu(menu, contextMenu);

        Utils.addSeparatorTo(contextMenu);
        Utils.addActionTo(contextMenu, copyTableColumnToClipboardAction);
        Utils.addSeparatorTo(contextMenu);
        Utils.addActionTo(contextMenu, sortAscendingAction);
        Utils.addActionTo(contextMenu, sortDescendingAction);
        Utils.addSeparatorTo(contextMenu);

        let attributeIsEditable = function()
        {
            let attribute = document.attribute(root.lastClickedColumnName);
            return attribute.isValid && attribute.editable;
        };

        Utils.addActionTo(contextMenu, cloneSpecificAttributeAction);

        Utils.addActionTo(contextMenu, editSpecificAttributeAction);
        Utils.setMenuItemVisibleFunction(contextMenu.itemAt(contextMenu.count - 1), attributeIsEditable);

        Utils.addActionTo(contextMenu, removeSpecificAttributeAction);
        Utils.setMenuItemVisibleFunction(contextMenu.itemAt(contextMenu.count - 1), attributeIsEditable);
    }

    function selectAll()
    {
        selectRows(0, proxyModel.rowCount() - 1);
    }

    Preferences
    {
        id: misc
        section: "misc"

        property var fileSaveInitialFolder
    }

    Labs.FileDialog
    {
        id: exportTableDialog
        visible: false
        fileMode: Labs.FileDialog.SaveFile
        defaultSuffix: selectedNameFilter.extensions[0]
        title: qsTr("Export Table")
        nameFilters: ["CSV File (*.csv)", "TSV File (*.tsv)"]
        onAccepted:
        {
            misc.fileSaveInitialFolder = folder.toString();
            document.writeTableModelToFile(tableView.model, file,
                defaultSuffix, tableView.visibleColumnNames());
        }
    }

    property alias resizeColumnsAction: resizeColumnsToContentsAction

    Action
    {
        id: resizeColumnsToContentsAction
        text: qsTr("&Resize Columns To Contents")
        icon.name: "auto-column-resize"
        onTriggered:
        {
            root.resizeColumnsToContents();
        }
    }

    property alias selectAction: selectColumnsAction

    Action
    {
        id: selectColumnsAction
        text: qsTr("&Select Visible Columns")
        icon.name: "column-select"
        checkable: true
        checked: root.columnSelectionMode

        onTriggered: { root.columnSelectionMode = !root.columnSelectionMode; }
    }

    property alias exportAction: exportTableAction

    Action
    {
        id: exportTableAction
        enabled: tableView.rows > 0
        text: qsTr("Export…")
        icon.name: "document-save"
        onTriggered:
        {
            exportTableDialog.folder = misc.fileSaveInitialFolder !== undefined ?
                        misc.fileSaveInitialFolder : "";

            exportTableDialog.open();
        }
    }

    Action
    {
        id: selectAllTableAction
        text: qsTr("Select All")
        icon.name: "edit-select-all"
        enabled: tableView.rows > 0

        onTriggered: { root.selectAll(); }
    }

    property int lastClickedColumn: -1
    property string lastClickedColumnName:
    {
        if(lastClickedColumn < 0 || lastClickedColumn >= (root.model.columnNames.length - 1))
            return "";

        root.model.columnNames[lastClickedColumn];
    }

    Action
    {
        id: copyTableColumnToClipboardAction
        enabled: tableView.rows > 0
        text: qsTr("Copy Column To Clipboard")
        icon.name: "document-save"
        onTriggered:
        {
            document.copyTableViewColumnToClipboard(tableView, lastClickedColumn);
        }
    }

    function updateSortActionChecked()
    {
        sortAscendingAction.checked = proxyModel.sortColumn === root.lastClickedColumnName &&
            proxyModel.sortOrder === Qt.AscendingOrder;
        sortDescendingAction.checked = proxyModel.sortColumn === root.lastClickedColumnName &&
            proxyModel.sortOrder === Qt.DescendingOrder;
    }

    Action
    {
        id: sortAscendingAction
        text: qsTr("Sort Column Ascending")
        checkable: true
        onTriggered:
        {
            proxyModel.sortColumn = root.lastClickedColumnName;
            proxyModel.sortOrder = Qt.AscendingOrder;
            root.updateSortActionChecked();
        }
    }

    Action
    {
        id: sortDescendingAction
        text: qsTr("Sort Column Descending")
        checkable: true
        onTriggered:
        {
            proxyModel.sortColumn = root.lastClickedColumnName;
            proxyModel.sortOrder = Qt.DescendingOrder;
            root.updateSortActionChecked();
        }
    }

    Action
    {
        id: cloneSpecificAttributeAction
        text: qsTr("Clone…")
        onTriggered: { cloneAttribute(root.lastClickedColumnName); }
    }

    Action
    {
        id: editSpecificAttributeAction
        text: qsTr("Edit…")
        onTriggered: { editAttribute(root.lastClickedColumnName); }
    }

    Action
    {
        id: removeSpecificAttributeAction
        text: qsTr("Remove")
        onTriggered: { document.removeAttributes([root.lastClickedColumnName]); }
    }

    SystemPalette { id: sysPalette }

    function selectRows(inStartRow, inEndRow)
    {
        selectionModel.change(inStartRow, inEndRow, ItemSelectionModel.Select);
    }

    function deselectRows(inStartRow, inEndRow)
    {
        selectionModel.change(inStartRow, inEndRow, ItemSelectionModel.Deselect);
    }

    ItemSelectionModel
    {
        id: selectionModel
        model: proxyModel
        onSelectionChanged: { proxyModel.setSubSelection(selectionModel.selection, deselected); }

        function change(inStartRow, inEndRow, action)
        {
            let less = Math.min(inStartRow, inEndRow);
            let max = Math.max(inStartRow, inEndRow);

            let range = proxyModel.buildRowSelectionRange(less, max);
            selectionModel.select([range], ItemSelectionModel.Rows | action)

            root.selectedRows = selectionModel.selectedRows(0).map(index => proxyModel.mapToSourceRow(index.row));
        }
    }

    Item
    {
        clip: true

        anchors.fill: parent
        anchors.topMargin: headerView.height
        z: 10

        SlidingPanel
        {
            id: columnSelectionControls
            visible: tableView.visible

            alignment: Qt.AlignTop|Qt.AlignLeft

            anchors.left: parent.left
            anchors.top: parent.top
            anchors.leftMargin: -Constants.margin
            anchors.topMargin: -Constants.margin

            initiallyOpen: false
            disableItemWhenClosed: false

            item: Rectangle
            {
                width: row.width
                height: row.height

                border.color: "black"
                border.width: 1
                radius: 4
                color: "white"

                RowLayout
                {
                    id: row

                    // The RowLayout in a RowLayout is just a hack to get some padding
                    RowLayout
                    {
                        Layout.topMargin: Constants.padding + Constants.margin - 2
                        Layout.bottomMargin: Constants.padding
                        Layout.leftMargin: Constants.padding + Constants.margin - 2
                        Layout.rightMargin: Constants.padding

                        Button
                        {
                            text: qsTr("Show All")
                            onClicked: { root.showAllColumns(); }
                        }

                        Button
                        {
                            text: qsTr("Hide All")
                            onClicked: { root.hideAllColumns(); }
                        }

                        Button
                        {
                            text: qsTr("Show Calculated")
                            onClicked: { root.showAllCalculatedColumns(); }
                        }

                        Button
                        {
                            text: qsTr("Hide Calculated")
                            onClicked: { root.hideAllCalculatedColumns(); }
                        }

                        Button
                        {
                            text: qsTr("Done")
                            icon.name: "emblem-unreadable"
                            onClicked: { columnSelectionMode = false; }
                        }
                    }
                }
            }
        }
    }

    ColumnLayout
    {
        anchors.fill: parent
        spacing: 0

        Item
        {
            Layout.fillWidth: true
            Layout.preferredHeight: fontMetrics.height + (2 * headerView.delegatePadding)

            TableView
            {
                id: headerView
                model: proxyModel.headerModel
                width: parent.width
                height: parent.height
                interactive: false
                clip: true
                rowHeightProvider: function(row)
                {
                    return row > 0 ? 0 : -1;
                }
                columnWidthProvider: tableView.columnWidthProvider;
                visible: tableView.columns !== 0
                boundsBehavior: Flickable.StopAtBounds

                onOriginXChanged:
                {
                    // Weird things can happen when the origin shifts, for some reason when the origin
                    // returns to 0 contentX can be left in a -ve position offsetting the content.
                    // This corrects that behaviour.
                    // (Reproduce by scrolling to far right, deselecting, selecting. Then scroll to far left slowly)
                    // Then select another node...)
                    contentX = originX + (horizontalTableViewScrollBar.position * contentWidth);
                }

                readonly property int sortIndicatorWidth: 7
                readonly property int sortIndicatorHeight: 4
                readonly property int sortIndicatorMargin: 3
                readonly property int delegatePadding: 4

                Rectangle
                {
                    height: headerView.height
                    width: headerView.width
                    color: sysPalette.light
                }

                delegate: DropArea
                {
                    id: headerItem
                    TableView.onReused:
                    {
                        refreshState();
                    }

                    function refreshState()
                    {
                        sourceColumn = Qt.binding(function() { return proxyModel.mapOrderedToSourceColumn(model.column) } );
                        implicitWidth =  Qt.binding(function() { return tableView.columnWidthProvider(model.column); });
                    }

                    implicitWidth: tableView.columnWidthProvider(model.column);
                    implicitHeight: headerLabel.height
                    property var modelColumn: model.column
                    property int sourceColumn: proxyModel.mapOrderedToSourceColumn(model.column);

                    Binding { target: headerContent; property: "sourceColumn"; value: sourceColumn }
                    Binding { target: headerContent; property: "modelColumn"; value: modelColumn }

                    property string text:
                    {
                        let index = headerItem.sourceColumn;

                        if(index < 0 || index >= root.model.columnNames.length)
                            return "";

                        return root.model.columnNames[index];
                    }

                    Connections
                    {
                        target: proxyModel
                        function onColumnOrderChanged()
                        {
                            refreshState();
                        }
                    }

                    onEntered:
                    {
                        drag.source.target = proxyModel.mapOrderedToSourceColumn(model.column);
                        tableView.forceLayoutSafe();
                    }

                    Rectangle
                    {
                        anchors.fill: parent
                        visible: dragHandler.active
                        color: Qt.lighter(sysPalette.highlight, 1.99)
                    }

                    Item
                    {
                        id: headerContent
                        opacity: Drag.active ? 0.5 : 1
                        property int sourceColumn: 0
                        property int modelColumn: 0
                        property int target: -1
                        width: headerItem.implicitWidth
                        height: headerItem.implicitHeight
                        anchors.left: parent.left
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        clip: true

                        states:
                        [
                            State
                            {
                                when: headerContent.Drag.active
                                ParentChange
                                {
                                    target: headerContent
                                    parent: headerView
                                }
                                AnchorChanges
                                {
                                    target: headerContent
                                    anchors.left: undefined
                                    anchors.top: undefined
                                }
                            }
                        ]

                        Rectangle
                        {
                            anchors.fill: parent
                            color: headerMouseArea.containsMouse ?
                                       Qt.lighter(sysPalette.highlight, 2.0) : sysPalette.light
                        }

                        Item
                        {
                            anchors.fill: parent
                            anchors.rightMargin: 5
                            anchors.leftMargin: 5

                            clip: true
                            CheckBox
                            {
                                anchors.verticalCenter: parent.verticalCenter

                                visible: columnSelectionMode
                                text: headerLabel.text
                                height: headerLabel.height

                                function isChecked()
                                {
                                    return !Utils.setContains(root.hiddenColumns, headerItem.text);
                                }

                                checked: { return isChecked(); }
                                onCheckedChanged:
                                {
                                    // Unbind to prevent binding loop
                                    checked = checked;
                                    root.setColumnVisibility(headerItem.text, checked);

                                    // Rebind so that the delegate doesn't hold the state
                                    checked = Qt.binding(isChecked);
                                }
                            }
                        }

                        Label
                        {
                            id: headerLabel
                            visible: !columnSelectionMode
                            clip: true
                            elide: Text.ElideRight
                            maximumLineCount: 1
                            width: parent.width - (headerView.sortIndicatorMargin + headerView.sortIndicatorWidth)
                            text: headerItem.text
                            color: sysPalette.text
                            padding: headerView.delegatePadding
                            renderType: Text.NativeRendering
                        }

                        Shape
                        {
                            id: sortIndicator
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.right: parent.right
                            anchors.rightMargin: headerView.sortIndicatorMargin
                            antialiasing: false
                            width: headerView.sortIndicatorWidth
                            height: headerView.sortIndicatorHeight
                            visible: proxyModel.sortColumn === headerItem.text && !columnSelectionMode
                            transform: Rotation
                            {
                                origin.x: sortIndicator.width * 0.5
                                origin.y: sortIndicator.height * 0.5
                                angle: proxyModel.sortOrder === Qt.DescendingOrder ? 0 : 180
                            }

                            ShapePath
                            {
                                miterLimit: 0
                                strokeColor: sysPalette.mid
                                fillColor: "transparent"
                                strokeWidth: 2
                                startY: sortIndicator.height - 1
                                PathLine { x: Math.round((sortIndicator.width - 1) * 0.5); y: 0 }
                                PathLine { x: sortIndicator.width - 1; y: sortIndicator.height - 1 }
                            }
                        }

                        DragHandler
                        {
                            id: dragHandler
                            yAxis.enabled: false
                        }

                        Drag.active: dragHandler.active
                        Drag.source: headerContent
                        Drag.hotSpot.x: headerContent.width * 0.5
                        Drag.hotSpot.y: headerContent.height * 0.5
                        property bool dragActive: Drag.active
                        onDragActiveChanged:
                        {
                            if(!dragActive)
                            {
                                if(headerContent.target > -1)
                                {
                                    let newColumnOrder = Array.from(proxyModel.columnOrder);

                                    let currentIndex = newColumnOrder.indexOf(
                                        root.model.columnNameFor(sourceColumn));
                                    let targetIndex = newColumnOrder.indexOf(
                                        root.model.columnNameFor(headerContent.target));
                                    array_move(newColumnOrder, currentIndex, targetIndex);
                                    headerContent.target = -1;

                                    proxyModel.columnOrder = newColumnOrder;
                                }

                                tableView.forceLayoutSafe();
                            }
                        }

                        MouseArea
                        {
                            id: headerMouseArea
                            enabled: !columnSelectionMode
                            anchors.fill: headerContent
                            hoverEnabled: true
                            acceptedButtons: Qt.LeftButton|Qt.RightButton

                            onClicked:
                            {
                                root.lastClickedColumn = headerItem.sourceColumn;

                                if(mouse.button === Qt.LeftButton)
                                {
                                    if(proxyModel.sortColumn === headerItem.text)
                                    {
                                        proxyModel.sortOrder = proxyModel.sortOrder === Qt.DescendingOrder ?
                                            Qt.AscendingOrder : Qt.DescendingOrder;
                                    }
                                    else
                                        proxyModel.sortColumn = headerItem.text;
                                }
                                else if(mouse.button === Qt.RightButton)
                                    contextMenu.show();

                                selectionModel.clear();
                                root.selectedRows = [];
                            }
                        }

                        Rectangle
                        {
                            anchors.right: parent.right
                            height: parent.height
                            width: 1
                            color: sysPalette.midlight
                            MouseArea
                            {
                                id: resizeHandleMouseArea
                                cursorShape: Qt.SizeHorCursor
                                width: 5
                                height: parent.height
                                anchors.horizontalCenter: parent.horizontalCenter
                                drag.target: parent
                                drag.axis: Drag.XAxis

                                onMouseXChanged:
                                {
                                    if(drag.active)
                                    {
                                        let sourceColumn = proxyModel.mapOrderedToSourceColumn(model.column);
                                        let userWidth = Math.max(30, headerItem.implicitWidth + mouseX);
                                        tableView.userColumnWidths[sourceColumn] = userWidth;
                                        headerItem.refreshState();
                                        tableView.forceLayoutSafe();
                                    }
                                }

                                onDoubleClicked: { root.resizeColumnsToContents(); }
                            }
                        }
                    }
                }
            }

            Rectangle
            {
                anchors.bottom: parent.bottom
                width: parent.width
                height: 1
                color: sysPalette.midlight
            }
        }

        Item
        {
            Layout.fillHeight: true
            Layout.fillWidth: true

            Label
            {
                z: 3
                text: qsTr("No Visible Columns")
                visible: tableView.columns === 0

                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
            }

            TableView
            {
                id: tableView
                anchors.fill: parent

                syncDirection: Qt.Horizontal
                syncView: headerView

                property var userColumnWidths: []
                property var currentColumnWidths: []
                property int currentTotalColumnWidth: 0
                property var columnWidths: []
                property int rowHeight: fontMetrics.height + 1

                property var _loadedCells: new Set()
                property int _leftLoadedColumn: -1
                property int _rightLoadedColumn: -1
                property int _topLoadedRow: -1
                property int _bottomLoadedRow: -1

                function _updateCellExtents()
                {
                    if(_loadedCells.size === 0)
                        return;

                    let left = Number.MAX_SAFE_INTEGER;
                    let right = Number.MIN_SAFE_INTEGER;
                    let top = Number.MAX_SAFE_INTEGER;
                    let bottom = Number.MIN_SAFE_INTEGER;

                    _loadedCells.forEach((cell) =>
                    {
                        left = Math.min(left, cell.x);
                        right = Math.max(right, cell.x);
                        top = Math.min(top, cell.y);
                        bottom = Math.max(bottom, cell.y);
                    });

                    _leftLoadedColumn = left;
                    _rightLoadedColumn = right;
                    _topLoadedRow = top;
                    _bottomLoadedRow = bottom;
                }

                function visibleColumnNames()
                {
                    let columnNames = [];

                    for(let i = 0; i < tableView.columns; i++)
                    {
                        let sourceIndex = proxyModel.mapOrderedToSourceColumn(i);
                        columnNames.push(root.model.columnNames[sourceIndex])
                    }

                    return columnNames;
                }

                signal fetchColumnSizes;

                property string hoveredLink: ""

                clip: true
                visible: tableView.columns !== 0
                boundsBehavior: Flickable.StopAtBounds

                Canvas
                {
                    id: backgroundCanvas
                    width: tableView.width
                    height: tableView.height + (tableView.rowHeight * 2)
                    x: tableView.contentX
                    y: tableView.contentY - (tableView.contentY % (tableView.rowHeight * 2))
                    onPaint:
                    {
                        let ctx = getContext("2d");
                        for(let i = 0; i < Math.ceil(tableView.height / tableView.rowHeight) + 1; i++)
                        {
                            let yOffset = (i * tableView.rowHeight);
                            ctx.fillStyle = i % 2 ? sysPalette.window : sysPalette.alternateBase;
                            ctx.fillRect(0, yOffset, width, tableView.rowHeight);
                        }
                    }

                    Connections
                    {
                        target: tableView
                        function onContentYChanged()
                        {
                            backgroundCanvas.requestPaint();
                        }
                    }
                }

                ScrollBar.horizontal: ScrollBar
                {
                    id: horizontalTableViewScrollBar
                    policy: ScrollBar.AsNeeded
                    minimumSize: 0.1
                    visible: size < 1.0 // So that it's invisible to mouse clicks

                    onPositionChanged:
                    {
                        // Sometimes syncViews don't actually sync visibleAreas
                        // however contentX and contentWidth are synced.
                        // The only reliable way to position both views is using
                        // the scrollbar position directly
                        if(position + size > 1)
                            position = 1 - size;
                        if(position < 0)
                            position = 0;
                    }
                }

                ScrollBar.vertical: ScrollBar
                {
                    id: verticalTableViewScrollBar
                    policy: ScrollBar.AsNeeded
                    minimumSize: 0.1
                    visible: size < 1.0 // So that it's invisible to mouse clicks
                }

                model: TableProxyModel
                {
                    id: proxyModel
                    columnNames: root.model.columnNames
                    sourceModel: root.model
                }

                columnWidthProvider: function(col)
                {
                    let calculatedWidth = 0;
                    let userWidth = userColumnWidths[proxyModel.mapOrderedToSourceColumn(col)];

                    // Use the user specified column width if available
                    if(userWidth !== undefined)
                        calculatedWidth = userWidth;
                    else
                        calculatedWidth = calculateMinimumColumnWidth(col);

                    return calculatedWidth;
                }

                function forceLayoutSafe()
                {
                    if(tableView.rows > 0 && tableView.columns > 0)
                        tableView.forceLayout();
                    if(headerView.rows > 0 && headerView.columns > 0)
                        headerView.forceLayout();
                }

                function columnAt(mouseX)
                {
                    let tableViewContentContainsMouse = mouseX >= 0 && mouseX < tableView.width;
                    if(!tableViewContentContainsMouse)
                        return -1;

                    let item = headerView.childAt(mouseX, headerView.height * 0.5);
                    if(!item)
                        return -1;

                    let headerDropAreaItem = item.childAt(mouseX + headerView.contentX, headerView.contentY);
                    if(!headerDropAreaItem || headerDropAreaItem.modelColumn === undefined)
                        return -1;

                    return headerDropAreaItem.sourceColumn;
                }

                function rowAt(mouseY)
                {
                    let tableViewContentContainsMouse = mouseY >= 0 && mouseY < tableView.height;
                    if(!tableViewContentContainsMouse)
                        return -1;

                    let hoverItem = tableView.childAt(0, mouseY);
                    if(!hoverItem)
                        return -1;

                    let tableItem = hoverItem.childAt(tableView.contentX, mouseY + tableView.contentY);
                    if(!tableItem || tableItem.modelRow === undefined)
                        return -1;

                    return tableItem.modelRow;
                }

                function calculateMinimumColumnWidth(col)
                {
                    let delegateWidth = tableView.currentColumnWidths[col];
                    let headerActualWidth = headerFullWidth(col);
                    if(headerActualWidth === null)
                    {
                        console.log("Null CMCW", headerView.columns, col);
                        return defaultColumnWidth;
                    }

                    if(delegateWidth === undefined)
                        return Math.max(defaultColumnWidth, headerActualWidth);
                    else
                        return Math.max(delegateWidth, headerActualWidth);
                }

                function headerFullWidth(column)
                {
                    let sourceColumn = proxyModel.mapOrderedToSourceColumn(column);
                    let sortIndicatorSpacing = ((headerView.delegatePadding + headerView.sortIndicatorMargin) * 2.0) +
                        headerView.sortIndicatorWidth;

                    if(sourceColumn > -1)
                    {
                        let headerName = root.model.columnNameFor(sourceColumn);
                        let width = fontMetrics.advanceWidth(headerName);
                        width += sortIndicatorSpacing;
                        return width;
                    }

                    return sortIndicatorSpacing;
                }

                FontMetrics { id: fontMetrics }

                delegate: Item
                {
                    // Based on Qt source for BaseTableView delegate
                    implicitHeight: tableView.rowHeight
                    implicitWidth: label.implicitWidth + 16

                    clip: false

                    // For access from the outside
                    property int modelColumn: model.column
                    property int modelRow: model.row

                    TableView.onReused:
                    {
                        tableView.fetchColumnSizes.connect(updateColumnWidths);

                        tableView._loadedCells.add({x: model.column, y: model.row});
                        tableView._updateCellExtents();
                    }

                    TableView.onPooled:
                    {
                        tableView.fetchColumnSizes.disconnect(updateColumnWidths);

                        tableView._loadedCells.forEach((cell) =>
                        {
                            if(cell.x === model.column && cell.y === model.row)
                                tableView._loadedCells.delete(cell);
                        });

                        tableView._updateCellExtents();
                    }

                    Component.onCompleted:
                    {
                        tableView.fetchColumnSizes.connect(updateColumnWidths);

                        tableView._loadedCells.add({x: model.column, y: model.row});
                        tableView._updateCellExtents();
                    }

                    function updateColumnWidths()
                    {
                        if(typeof model === 'undefined')
                            return;
                        let storedWidth = tableView.columnWidths[model.column];
                        if(storedWidth !== undefined)
                            tableView.columnWidths[model.column] = Math.max(implicitWidth, storedWidth);
                        else
                            tableView.columnWidths[model.column] = implicitWidth;
                    }

                    // When the columns don't occupy the full width of the TableView, we display a "filler"
                    // selection marker in the empty space to the right of the last column
                    Rectangle
                    {
                        anchors.left: parent.right

                        // The remaining width to the right of this cell
                        width: tableView.width - (parent.x + parent.width)
                        height: parent.height

                        color: sysPalette.highlight;
                        visible: (model.column === (proxyModel.columnCount() - 1)) && model.subSelected
                    }

                    Rectangle
                    {
                        anchors.fill: parent

                        color:
                        {
                            if(model.subSelected)
                                return sysPalette.highlight;

                            return model.row % 2 ? sysPalette.window : sysPalette.alternateBase;
                        }

                        // Ripped more or less verbatim from qtquickcontrols/src/controls/Styles/Desktop/TableViewStyle.qml
                        // except for the text property
                        Text
                        {
                            id: label
                            objectName: "label"
                            elide: Text.ElideRight
                            wrapMode: Text.NoWrap
                            textFormat: Text.StyledText
                            renderType: Text.NativeRendering
                            width: parent.width
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.leftMargin: 10
                            color: QmlUtils.contrastingColor(parent.color)

                            text:
                            {
                                let sourceColumn = proxyModel.mapOrderedToSourceColumn(model.column);

                                // This can happen during column removal
                                if(sourceColumn === undefined || sourceColumn < 0)
                                {
                                    console.log("Model Column Unable to map", model.row, model.column);
                                    return "";
                                }

                                // AbstractItemModel required empty values to return empty variant
                                // but TableView2 delgates cast them to undefined js objects.
                                // It's difficult to tell if the model is corrupted or accessing
                                // invalid data now as they both return undefined.
                                if(model.display === undefined)
                                    return "";

                                let columnName = root.model.columnNameFor(sourceColumn);
                                if(root.model.columnIsNumerical(columnName))
                                    return QmlUtils.formatNumberScientific(model.display);

                                if(typeof(model.display) === "string")
                                {
                                    let linkifyRe = /(?![^<]*>|[^<>]*<\/)((https?:)\/\/[a-z0-9&#=.\/\-?_]+)/gi;
                                    let stripNewlinesRe = /[\r\n]+/g;

                                    return model.display
                                        .replace(linkifyRe, "<a href=\"$1\">$1</a>")
                                        .replace(stripNewlinesRe, " ");
                                }

                                return model.display;
                            }

                            onLinkHovered: { tableView.hoveredLink = link; }
                            onLinkActivated: Qt.openUrlExternally(link);
                        }
                    }
                }

                function _updateColumnVisibility()
                {
                    if(root.columnSelectionMode)
                        proxyModel.hiddenColumns = [];
                    else
                        proxyModel.hiddenColumns = hiddenColumns;
                }

                Connections
                {
                    target: root.model
                    function onSelectionChanged()
                    {
                        proxyModel.invalidateFilter();
                        selectRows(0, proxyModel.rowCount() - 1);
                        verticalTableViewScrollBar.position = 0;
                    }
                }

                Component.onCompleted:
                {
                    populateTableMenu(tableView._tableMenu);

                    root.resizeColumnsToContents();
                    tableView._updateColumnVisibility();
                }

                // This is just a reference to the menu, so we can repopulate it later as necessary
                property Menu _tableMenu
            }

            MouseArea
            {
                acceptedButtons: Qt.LeftButton | Qt.RightButton
                property int previousRow: -1
                property int startRow: -1
                property int endRow: -1
                property bool deselectDrag: false
                anchors.fill: parent

                // Bottom/right margin otherwise clicks don't reach the scrollbars
                anchors.bottomMargin: horizontalTableViewScrollBar.size < 1.0 ? horizontalTableViewScrollBar.height : 0
                anchors.rightMargin: verticalTableViewScrollBar.size < 1.0 ? verticalTableViewScrollBar.width : 0

                visible: !columnSelectionMode

                cursorShape: tableView.hoveredLink.length > 0 ?
                    Qt.PointingHandCursor : Qt.ArrowCursor;

                onDoubleClicked:
                {
                    if(mouse.button !== Qt.LeftButton)
                        return;

                    let clickedRow = tableView.rowAt(mouseY);
                    if(clickedRow < 0)
                        return;

                    let mappedRow = proxyModel.mapToSourceRow(clickedRow);
                    root.model.moveFocusToNodeForRowIndex(mappedRow);
                }

                onClicked:
                {
                    root.lastClickedColumn = tableView.columnAt(mouseX);

                    if(mouse.button === Qt.RightButton)
                        root.rightClick();
                }

                onPressed:
                {
                    if(mouse.button !== Qt.LeftButton)
                        return;

                    if(tableView.hoveredLink.length > 0)
                        mouse.accepted = false;

                    forceActiveFocus();
                    if(tableView.rows === 0)
                        return;

                    let clickedRow = tableView.rowAt(mouseY);
                    if(clickedRow < 0)
                        return;

                    let selectionChanged = false;
                    let rowIsSelected = selectionModel.isSelected(
                        proxyModel.index(clickedRow, 0));

                    if((mouse.modifiers & Qt.ShiftModifier) && endRow !== -1)
                    {
                        selectRows(endRow, clickedRow);
                    }
                    else if((mouse.modifiers & Qt.ControlModifier) && rowIsSelected)
                    {
                        deselectRows(clickedRow, clickedRow);
                        deselectDrag = true;
                    }
                    else
                    {
                        if(!(mouse.modifiers & Qt.ControlModifier))
                            selectionModel.clear();

                        selectRows(clickedRow, clickedRow);
                    }

                    previousRow = startRow = endRow = clickedRow;
                }

                onPositionChanged:
                {
                    if(mouse.buttons !== Qt.LeftButton)
                        return;

                    let rowUnderCursor = tableView.rowAt(mouseY);
                    if(rowUnderCursor < 0)
                        return;

                    if(rowUnderCursor !== previousRow)
                    {
                        if(deselectDrag)
                            deselectRows(startRow, rowUnderCursor);
                        else
                            selectRows(startRow, rowUnderCursor);

                        previousRow = endRow = rowUnderCursor;
                    }
                }

                onReleased:
                {
                    previousRow = -1;
                    deselectDrag = false;
                }

                Keys.onDownPressed:
                {
                    if(endRow !== -1 && (endRow + 1) < tableView.rows)
                    {
                        endRow++;
                        arrowPress(event.modifiers);
                    }
                }
                Keys.onUpPressed:
                {
                    if(endRow !== -1 && (endRow - 1) >= 0)
                    {
                        endRow--;
                        arrowPress(event.modifiers);
                    }
                }

                function arrowPress(modifier)
                {
                    // Horrible hack to scroll the view
                    let diff = 0;

                    diff = Math.max(endRow - (tableView._bottomLoadedRow - 1) , 0);
                    diff += Math.min(endRow - (tableView._topLoadedRow + 1), 0);
                    tableView.contentY += diff * tableView.rowHeight;

                    // Clamp scrollbar to prevent overscrolling
                    // (scrollbar seems to be the only safe way)
                    verticalTableViewScrollBar.position = Utils.clamp(
                        verticalTableViewScrollBar.position,
                        0, 1.0 - verticalTableViewScrollBar.size);

                    selectionModel.clear();
                    if(modifier & Qt.ShiftModifier)
                    {
                        if(startRow == -1)
                            startRow = endRow;

                        selectRows(startRow, endRow);
                    }
                    else
                    {
                        startRow = endRow;
                        selectRows(endRow, endRow);
                    }
                }
            }
        }
    }

    PlatformMenu
    {
        id: contextMenu

        function show()
        {
            root.updateSortActionChecked();
            contextMenu.popup();
        }
    }

    signal rightClick();
    onRightClick:
    {
        if(contextMenu.enabled)
            contextMenu.show();
    }

    function array_move(arr, old_index, new_index)
    {
        if (new_index >= arr.length)
        {
            let k = new_index - arr.length + 1;
            while(k--)
                arr.push(undefined);
        }
        arr.splice(new_index, 0, arr.splice(old_index, 1)[0]);
        return arr; // for testing
    }
}
