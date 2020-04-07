/*****************************************************************************
 * Copyright (C) 2019 VLC authors and VideoLAN
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * ( at your option ) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/
import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3
import QtQml.Models 2.2

import org.videolan.medialib 0.1

import "qrc:///util/" as Util
import "qrc:///widgets/" as Widgets
import "qrc:///style/"

Widgets.NavigableFocusScope {
    id: root
    readonly property var currentIndex: view.currentItem.currentIndex
    //the index to "go to" when the view is loaded
    property var initialIndex: 0

    property alias contentModel: videosDelegate.model;

    navigationCancel: function() {
        if (view.currentItem.currentIndex <= 0) {
            defaultNavigationCancel()
        } else {
            view.currentItem.currentIndex = 0;
            view.currentItem.positionViewAtIndex(0, ItemView.Contain)
        }
    }

    onCurrentIndexChanged: {
        history.update([ "mc", "video", {"initialIndex": currentIndex}])
    }

    onInitialIndexChanged: resetFocus()
    onContentModelChanged: resetFocus()

    function resetFocus() {
        if (videosDelegate.items.count === 0) {
            return
        }
        var initialIndex = root.initialIndex
        if (initialIndex >= videosDelegate.items.count)
            initialIndex = 0
        videosDelegate.selectNone()
        videosDelegate.items.get(initialIndex).inSelected = true
        view.currentItem.currentIndex = initialIndex
        view.currentItem.positionViewAtIndex(initialIndex, ItemView.Contain)
    }

    Widgets.MenuExt {
        id: contextMenu
        property var model: ({})
        closePolicy: Popup.CloseOnReleaseOutside | Popup.CloseOnEscape

        Widgets.MenuItemExt {
            id: playMenuItem
            text: "Play from start"
            onTriggered: {
                medialib.addAndPlay( contextMenu.model.id )
                history.push(["player"])
            }
        }
        Widgets.MenuItemExt {
            text: "Play all"
            onTriggered: console.log("not implemented")
        }
        Widgets.MenuItemExt {
            text: "Play as audio"
            onTriggered: console.log("not implemented")
        }
        Widgets.MenuItemExt {
            text: "Enqueue"
            onTriggered: medialib.addToPlaylist( contextMenu.model.id )
        }
        Widgets.MenuItemExt {
            enabled: medialib.gridView
            text: "Information"
            onTriggered: {
                view.currentItem.switchExpandItem(contextMenu.model.index)
            }
        }
        Widgets.MenuItemExt {
            text: "Download subtitles"
            onTriggered: console.log("not implemented")
        }
        Widgets.MenuItemExt {
            text: "Add to playlist"
            onTriggered: console.log("not implemented")
        }
        Widgets.MenuItemExt {
            text: "Delete"
            onTriggered: g_dialogs.ask(i18n.qtr("Are you sure you want to delete?"), function() {
                console.log("unimplemented")
            })
        }

        onClosed: contextMenu.parent.forceActiveFocus()

    }
    Util.SelectableDelegateModel {
        id: videosDelegate

        model: MLVideoModel {
            id: videoModel
            ml: medialib
        }
        delegate: Package{
            Item { Package.name: "grid" }
        }

        onCountChanged: {
            if (videosDelegate.items.count > 0 && videosDelegate.selectedGroup.count === 0) {
                root.resetFocus()
            }
        }

        function actionAtIndex(index) {
            var list = []
            for (var i = 0; i < videosDelegate.selectedGroup.count; i++)
                list.push(videosDelegate.selectedGroup.get(i).model.id)
            medialib.addAndPlay( list )
            history.push(["player"])
        }
    }

    Component {
        id: gridComponent

        Widgets.ExpandGridView {
            id: videosGV
            property Item currentItem: Item{}

            activeFocusOnTab:true
            delegateModel: videosDelegate
            model: videoModel

            headerDelegate: Widgets.LabelSeparator {
                id: videosSeparator
                width: videosGV.width
                text: i18n.qtr("Videos")
            }


            delegate: VideoGridItem {
                id: videoGridItem

                onContextMenuButtonClicked: {
                    contextMenu.model = videoGridItem.model
                    contextMenu.popup()
                }

                onItemClicked : {
                    videosDelegate.updateSelection( modifier , videosGV.currentIndex, index)
                    videosGV.currentIndex = index
                    videosGV.forceActiveFocus()
                }
            }

            expandDelegate: VideoInfoExpandPanel {
                visible: !videosGV.isAnimating

                implicitHeight: videosGV.height - videosGV.cellHeight
                width: videosGV.width

                onRetract: videosGV.retract()
                notchPosition: videosGV.getItemPos(videosGV._expandIndex)[0] + (videosGV.cellWidth / 2)

                navigationParent: videosGV
                navigationCancel:  function() {  videosGV.retract() }
                navigationUp: function() {  videosGV.retract() }
                navigationDown: function() { videosGV.retract() }
            }

            navigationParent: root

            /*
             *define the intial position/selection
             * This is done on activeFocus rather than Component.onCompleted because videosDelegate.
             * selectedGroup update itself after this event
             */
            onActiveFocusChanged: {
                if (activeFocus && videosDelegate.items.count > 0 && videosDelegate.selectedGroup.count === 0) {
                    videosDelegate.items.get(0).inSelected = true
                }
            }

            cellWidth: VLCStyle.gridItem_video_width
            cellHeight: VLCStyle.gridItem_video_height

            onSelectAll:videosDelegate.selectAll()
            onSelectionUpdated: videosDelegate.updateSelection( keyModifiers, oldIndex, newIndex )
            onActionAtIndex: videosDelegate.actionAtIndex( index )
        }

    }


    Component {
        id: listComponent
        VideoListDisplay {
            height: view.height
            width: view.width
            onContextMenuButtonClicked:{
                contextMenu.model = menuModel
                contextMenu.popup(menuParent)
            }

            onRightClick:{
                contextMenu.model = menuModel
                contextMenu.popup()
            }

            navigationParent: root
        }
    }

    Widgets.StackViewExt {
        id: view
        anchors.fill:parent
        clip: true
        focus: videosDelegate.items.count !== 0
        initialItem: medialib.gridView ? gridComponent : listComponent
        Connections {
            target: medialib
            onGridViewChanged: {
                if (medialib.gridView)
                    view.replace(gridComponent)
                else
                    view.replace(listComponent)
            }
        }

    }

    EmptyLabel {
        anchors.fill: parent
        visible: videosDelegate.items.count === 0
        focus: visible
        text: i18n.qtr("No video found\nPlease try adding sources, by going to the Network tab")
        navigationParent: root
    }
}
