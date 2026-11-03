// Generated from SVG file isha.svg
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
                path: "M 20.985 12.486 Q 20.7891 16.1141 18.1293 18.5894 Q 15.4694 21.0646 11.8366 20.9994 Q 8.2038 20.9342 5.63446 18.3652 Q 3.06512 15.7961 2.99955 12.1633 Q 2.93398 8.53047 5.40894 5.87035 Q 7.8839 3.21024 11.512 3.014 Q 11.8113 2.99774 11.9455 3.27863 Q 12.0778 3.55561 11.914 3.817 Q 10.8034 5.59394 11.0405 7.67595 Q 11.2776 9.75796 12.7593 11.2397 Q 14.241 12.7214 16.323 12.9585 Q 18.4051 13.1956 20.182 12.085 Q 20.4441 11.9212 20.7208 12.053 Q 21.0013 12.1867 20.985 12.486 "
            }
        }
    }

    component AnimationsInfo: QtObject {
        property int loops: 1
        property bool paused: false

        signal restart
    }
}
