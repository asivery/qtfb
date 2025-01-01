import QtQuick 2.5
import QtQuick.Window 2.2
import net.asivery.Framebuffer;

Window {
    visible: true
    title: qsTr("QTFB - PC emulator")
    width: 1620 / 4
    height: 2160 / 4
    id: window

    Rectangle {
        anchors.fill: parent
        color: "#f0f0f0"

        Loader {
            id: loader
            sourceComponent: Component {
                Rectangle {
                    FBController {
                        anchors.fill: parent
                        id: canvasController
                        allowScaling: true
                        framebufferID: 245209899
                    }
                }
            }

            width: 1620
            height: 2160

            transform: Scale {
                origin.x: loader.width / 2
                origin.y: loader.height / 2
                xScale: Math.min(window.width / loader.width, window.height / loader.height)
                yScale: Math.min(window.width / loader.width, window.height / loader.height)
            }

            anchors.centerIn: parent
        }
    }
}

