import QtQuick
import QtPositioning
import QtLocation

Item {
    id: ls

    property bool loading: geocodeModel.status == GeocodeModel.Loading ? true : false

    signal searchResults(var locations)

    function search(query) {
        geocodeModel.reset();
        geocodeModel.query = query;
        geocodeModel.update();
    }

    Plugin {
        id: aPlugin

        name: "osm"

        parameters: PluginParameter {
            name: "osm.geocoding.host"
            value: "https://nominatim.openstreetmap.org"
        }
    }

    GeocodeModel {
        id: geocodeModel

        autoUpdate: false
        plugin: aPlugin

        onLocationsChanged: {
            ls.searchResults(this);
        }
    }

    //    PositionSource {

    //        id: src
    //        active: true

    //        onPositionChanged: {

    //            active = false
    //            var coord = src.position.coordinate;
    //            search(coord)
    //        }
    //    }

    //     function findMe() {
    //            geocodeModel.query = "california"//"77.100.28.69"// "birmingham"//  src.position.coordinate;//"53 Brandl St, Eight Mile Plains, Australia"
    //            geocodeModel.update()
    //    }
}
