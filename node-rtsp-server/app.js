var rtsp = require('rtsp-server')
 
var server = rtsp.createServer(function (req, res) {
  console.log(req.method, req)
 
//  switch (req.method) {
//    case 'OPTIONS':
//      res.setHeader('Public', 'OPTIONS');
//      break
//    default:
//      res.statusCode = 501 // Not implemented
//  }
 
  res.end() // will echo the CSeq header used in the request
})
 
server.listen(5000, function () {
  var port = server.address().port
  console.log('RTSP server is running on port:', port)
})
