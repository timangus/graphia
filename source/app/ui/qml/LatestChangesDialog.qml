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

import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import app.graphia.Controls 1.0
import app.graphia.Shared 1.0
import app.graphia.Shared.Controls 1.0

Window
{
    id: root

    property string text
    property string version

    title: qsTr("Latest Changes for Version " + root.version)
    flags: Qt.Window|Qt.Dialog

    minimumWidth: 640
    minimumHeight: 480
    width: 640
    height: 480

    ColumnLayout
    {
        anchors.fill: parent
        anchors.margins: Constants.margin

        ScrollableTextArea
        {
            Layout.fillWidth: true
            Layout.fillHeight: true

            readOnly: true
            textFormat: TextEdit.MarkdownText
            text: root.text

            onLinkActivated: function(link) { Qt.openUrlExternally(link); }
        }

        Button
        {
            text: qsTr("Close")
            Layout.alignment: Qt.AlignRight
            onClicked: function(mouse) { root.close(); }
        }
    }
}
