// Generated from SVG file dhuhr.svg
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
            id: _qt_node1_fill_stroke

            capStyle: ShapePath.RoundCap
            fillColor: "#00000000"
            fillRule: ShapePath.WindingFill
            joinStyle: ShapePath.RoundJoin
            miterLimit: 4
            pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
            strokeColor: "#ff707070"
            strokeWidth: 2

            PathSvg {
                path: "M 16 12 Q 16 13.6569 14.8284 14.8284 Q 13.6569 16 12 16 Q 10.3431 16 9.17157 14.8284 Q 8 13.6569 8 12 Q 8 10.3431 9.17157 9.17157 Q 10.3431 8 12 8 Q 13.6569 8 14.8284 9.17157 Q 16 10.3431 16 12 "
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
                path: "M 12 2 L 12 4 "
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
                path: "M 12 20 L 12 22 "
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
                path: "M 4.93 4.93 L 6.34 6.34 "
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
                path: "M 17.66 17.66 L 19.07 19.07 "
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
                path: "M 2 12 L 4 12 "
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
                path: "M 20 12 L 22 12 "
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
                path: "M 6.34 17.66 L 4.93 19.07 "
            }
        }

        ShapePath {
            id: _qt_node9_stroke

            capStyle: ShapePath.RoundCap
            fillColor: "transparent"
            fillRule: ShapePath.WindingFill
            joinStyle: ShapePath.RoundJoin
            miterLimit: 4
            pathHints: ShapePath.PathQuadratic
            strokeColor: "#ff707070"
            strokeWidth: 2

            PathSvg {
                path: "M 19.07 4.93 L 17.66 6.34 "
            }
        }
    }

    component AnimationsInfo: QtObject {
        property int loops: 1
        property bool paused: false

        signal restart
    }
}
