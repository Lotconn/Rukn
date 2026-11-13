// Generated from SVG file fajr.svg
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
                path: "M 12 2 L 12 4 "
            }
        }

        ShapePath {
            id: _qt_node2_fill_stroke

            capStyle: ShapePath.RoundCap
            fillColor: "#00000000"
            fillRule: ShapePath.WindingFill
            joinStyle: ShapePath.RoundJoin
            miterLimit: 4
            pathHints: ShapePath.PathQuadratic | ShapePath.PathNonIntersecting | ShapePath.PathNonOverlappingControlPointTriangles
            strokeColor: "#ff707070"
            strokeWidth: 2

            PathSvg {
                path: "M 14.837 16.385 Q 14.3257 18.5419 12.5349 19.8483 Q 10.744 21.1547 8.53402 20.983 Q 6.324 20.8113 4.75645 19.2439 Q 3.18891 17.6766 3.01689 15.4666 Q 2.84488 13.2566 4.15102 11.4656 Q 5.45716 9.67461 7.614 9.163 Q 8.06625 9.05646 8.30112 9.51138 Q 8.52831 9.95141 8.329 10.411 Q 7.84048 11.5404 8.0712 12.7491 Q 8.30192 13.9577 9.1721 14.8277 Q 10.0423 15.6978 11.251 15.9282 Q 12.4597 16.1587 13.589 15.67 Q 14.0495 15.4706 14.4894 15.6975 Q 14.9442 15.9321 14.837 16.385 "
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
                path: "M 16 12 Q 16 10.3431 14.8284 9.17157 Q 13.6569 8 12 8 "
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
                path: "M 19 5 L 17.744 6.256 "
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
                path: "M 20 12 L 22 12 "
            }
        }
    }

    component AnimationsInfo: QtObject {
        property int loops: 1
        property bool paused: false

        signal restart
    }
}
