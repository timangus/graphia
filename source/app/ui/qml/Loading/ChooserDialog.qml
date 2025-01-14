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

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import app.graphia
import app.graphia.Shared

import SortFilterProxyModel

Window
{
    id: root

    property var model: null
    property string displayRole: "display"
    property string valueRole: "display"
    property string explanationText: ""
    property string choiceLabelText: ""

    property var values: []

    modality: Qt.WindowModal
    flags: Qt.Dialog | (Qt.platform.os === "osx" ? Qt.Sheet : 0)

    width: 380
    minimumWidth: width
    maximumWidth: width

    height: 180
    minimumHeight: height
    maximumHeight: height

    onVisibleChanged:
    {
        if(visible)
            rememberThisChoiceCheckBox.checked = false;
    }

    ColumnLayout
    {
        id: layout

        spacing: Constants.spacing
        anchors.fill: parent
        anchors.margins: Constants.margin

        Text
        {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop

            text: explanationText
            wrapMode: Text.WordWrap
        }

        RowLayout
        {
            spacing: Constants.spacing

            Text { text: root.choiceLabelText }

            ComboBox
            {
                id: comboBox
                Layout.preferredWidth: 200

                model: SortFilterProxyModel
                {
                    id: proxyModel

                    sourceModel: root.model
                    filterRoleName: root.valueRole
                    filterPattern:
                    {
                        let s = "";

                        for(let i = 0; i < root.values.length; i++)
                        {
                            if(i !== 0) s += "|";
                            s += root.values[i];
                        }

                        return s;
                    }

                    onFilterPatternChanged:
                    {
                        // Reset to first item
                        comboBox.currentIndex = -1;
                        comboBox.currentIndex = 0;
                    }
                }

                property string selectedValue:
                {
                    if(root.model === null)
                        return "";

                    let row = proxyModel.mapToSource(currentIndex);
                    if(row < 0)
                        return "";

                    let role = QmlUtils.modelRoleForName(root.model, root.valueRole);
                    return root.model.data(root.model.index(row, 0), role);
                }

                textRole: root.displayRole
            }
        }

        CheckBox
        {
            id: rememberThisChoiceCheckBox
            Layout.alignment: Qt.AlignBottom
            text: qsTr("Remember This Choice")
        }

        RowLayout
        {
            Layout.alignment: Qt.AlignBottom

            Item { Layout.fillWidth: true }

            Button
            {
                text: qsTr("OK")
                onClicked: { root.close(); root.accepted(); }
            }

            Button
            {
                text: qsTr("Cancel")
                onClicked: { root.close(); root.rejected(); }
            }
        }
    }

    property var _onAcceptedFn: null

    onAccepted:
    {
        if(_onAcceptedFn !== null)
            _onAcceptedFn(comboBox.selectedValue, rememberThisChoiceCheckBox.checked);
    }

    function open(onAcceptedFn)
    {
        // Force comboBox.selectedValue to be updated
        comboBox.currentIndex = -1;
        comboBox.currentIndex = 0;

        root._onAcceptedFn = onAcceptedFn;
        Qt.callLater(function()
        {
            // Delay the opening in case an existing choice is still "in-flight",
            // e.g. when choosing a plugin immediately after choosing file type
            show();
        });
    }

    signal accepted();
    signal rejected();
}
