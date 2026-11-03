// Generated from SVG file maghrib.svg
import QtQuick
import QtQuick.Shapes

Item {
    id: __qt_toplevel

    property AnimationsInfo animations: AnimationsInfo {}

    implicitHeight: 24
    implicitWidth: 24

    transform: [
        Scale {
            xScale: __qt_toplevel.width / 24
            yScale: __qt_toplevel.height / 24
        }
    ]

    Shape {
        id: _qt_node0

        asynchronous: true
        preferredRendererType: Shape.CurveRenderer
        transformOrigin: Item.TopLeft

        ShapePath {
            id: _qt_node1_stroke

            capStyle: ShapePath.RoundCap
            fillColor: "transparent"
            fillRule: ShapePath.WindingFill
            joinStyle: ShapePath.RoundJoin
            miterLimit: 4
            pathHints: ShapePath.PathQuadratic
            strokeColor: "#ff707070"
            strokeWidth: 2

            PathSvg {
                path: "M 12 10 L 12 2 "
            }
        }

        ShapePath {
            id: _qt_node2_stroke

            capStyle: ShapePath.RoundCap
            fillColor: "transparent"
            fillRule: ShapePath.WindingFill
            joinStyle: ShapePath.RoundJoin
            miterLimit: 4
            pathHints: ShapePath.PathQuadratic
            strokeColor: "#ff707070"
            strokeWidth: 2

            PathSvg {
                path: "M 4.93 10.93 L 6.34 12.34 "
            }
        }

        ShapePath {
            id: _qt_node3_stroke

            capStyle: ShapePath.RoundCap
            fillColor: "transparent"
            fillRule: ShapePath.WindingFill
            joinStyle: ShapePath.RoundJoin
            miterLimit: 4
            pathHints: ShapePath.PathQuadratic
            strokeColor: "#ff707070"
            strokeWidth: 2

            PathSvg {
                path: "M 2 18 L 4 18 "
            }
        }

        ShapePath {
            id: _qt_node4_stroke

            capStyle: ShapePath.RoundCap
            fillColor: "transparent"
            fillRule: ShapePath.WindingFill
            joinStyle: ShapePath.RoundJoin
            miterLimit: 4
            pathHints: ShapePath.PathQuadratic
            strokeColor: "#ff707070"
            strokeWidth: 2

            PathSvg {
                path: "M 20 18 L 22 18 "
            }
        }

        ShapePath {
            id: _qt_node5_stroke

            capStyle: ShapePath.RoundCap
            fillColor: "transparent"
            fillRule: ShapePath.WindingFill
            joinStyle: ShapePath.RoundJoin
            miterLimit: 4
            pathHints: ShapePath.PathQuadratic
            strokeColor: "#ff707070"
            strokeWidth: 2

            PathSvg {
                path: "M 19.07 10.93 L 17.66 12.34 "
            }
        }

        ShapePath {
            id: _qt_node6_stroke

            capStyle: ShapePath.RoundCap
            fillColor: "transparent"
            fillRule: ShapePath.WindingFill
            joinStyle: ShapePath.RoundJoin
            miterLimit: 4
            pathHints: ShapePath.PathQuadratic
            strokeColor: "#ff707070"
            strokeWidth: 2

            PathSvg {
                path: "M 22 22 L 2 22 "
            }
        }

        ShapePath {
            id: _qt_node7_stroke

            capStyle: ShapePath.RoundCap
            fillColor: "transparent"
            fillRule: ShapePath.WindingFill
            joinStyle: ShapePath.RoundJoin
            miterLimit: 4
            pathHints: ShapePath.PathQuadratic
            strokeColor: "#ff707070"
            strokeWidth: 2

            PathSvg {
                path: "M 16 6 L 12 10 L 8 6 "
            }
        }

        ShapePath {
            id: _qt_node8_stroke

            capStyle: ShapePath.RoundCap
            fillColor: "transparent"
            fillRule: ShapePath.WindingFill
            joinStyle: ShapePath.RoundJoin
            miterLimit: 4
            pathHints: ShapePath.PathQuadratic
            strokeColor: "#ff707070"
            strokeWidth: 2

            PathSvg {
                path: "M 16 18 Q 16 16.3431 14.8284 15.1716 Q 13.6569 14 12 14 Q 10.3431 14 9.17157 15.1716 Q 8 16.3431 8 18 "
            }
        }
    }

    component AnimationsInfo: QtObject {
        property int loops: 1
        property bool paused: false

        signal restart
    }
}
