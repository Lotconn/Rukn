import QtQuick
import QtQuick.Controls

SpinBox {
    id: spinBox

    property real storeVar

    signal edited(real newValue)

    editable: true
    from: 0
    implicitWidth: 120
    inputMethodHints: Qt.ImhDigitsOnly
    textFromValue: (value, locale) => Number(value).toLocaleString(locale, 'f', 0) + "%"
    to: 100
    value: Math.round(storeVar * 100)
    valueFromText: function (text, locale) {
        try {
            // strip "%" and the Arabic percent sign (U+066A) plus whitespace
            return Math.round(Number.fromLocaleString(locale, text.replace(/[%\u066A\s]/g, "")));
        } catch (e) {
            return value;   // keep the current value when input is unparsable
        }
    }
    wheelEnabled: true

    onValueModified: {
        edited(value / 100);
    }
}
