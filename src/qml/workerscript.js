WorkerScript.onMessage = function(msg) {
//    console.debug("Days in month: ", msg.days);
//    console.debug("Entries in JS: ", msg.entries);

    if(msg.entries === '') {
        for(var i = 0; i < msg.days; i++) {
            msg.model.append( { "fa":"", "sr":"", "da":"", "aa":"", "ma":"", "ia":"", "fi":"", "di":"", "ai":"", "mi":"", "ii":""} );
            //msg.model.sync();
        }
    } else {
        try {
            var entriesArr = JSON.parse(msg.entries)
            //console.debug("Entry in JS: ", msg.entries.get(i));
            for(var i = 0; i < msg.days; i++) {
                msg.model.append(entriesArr[i]);
                //msg.model.sync();
            }
        } catch(e) {
            console.error(e);
        }
    }
    msg.model.sync();
    WorkerScript.sendMessage({'reply': 'done'});
}
