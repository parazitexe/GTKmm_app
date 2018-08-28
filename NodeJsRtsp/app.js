var express = require("express");
var app = express();
var cons = require("consolidate");
var bodyParser = require('body-parser');

app.engine("html", cons.nunjucks);
app.set("view engine", "html");
app.set("views", __dirname + "/views");
app.use(bodyParser.urlencoded({ extended: true }));

// Handler for internal server errors
function errorHandler(err, req, res, next) {
    console.log(err.message);
    console.log(err.stack);
    res.status(500);
    res.render("error_template", {error:err});
}
//url handler
app.get("/", function (req, res, next) {
    res.render("startPage", {'title':'startPage', "data": "FILIN"} )
});
app.use(express.static('public'));
app.use('./files', express.static('public'));


//rtsp Stream
var Stream = require('./node-rtsp-stream');
var stream = new Stream({
    name: 'myStream',
    streamUrl: 'rtsp://admin:admin1@192.168.100.222:554/cam/realmonitor?channel=1&subtype=1',
    //streamUrl: 'rtsp://wowzaec2demo.streamlock.net/vod/mp4:BigBuckBunny_115k.mov',
    wsPort: 9999,
});


//ui controls signals server
const ws = require('ws');
var ui_wss = new ws.Server({
    port: 9998
});
ui_wss.on('connection', function connection(socket) {
    socket.on('message', function incoming(data) {
        console.log("app get message", data);

        if(data == "restart"){
            stream.restart();
        }

    });
});


app.use(errorHandler);
var server = app.listen(3000, function() {
    var port = server.address().port;
    console.log('Express server listening on port %s.', port);
});




