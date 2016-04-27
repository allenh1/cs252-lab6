var http = require('http'),
    path = require('path'),
    fs = require('fs'),
    extTranslator = require('extTranslator'),//module, that tranlates file extension to proper Content-type
    config = JSON.parse(fs.readFileSync('./config.json', {encoding: 'utf-8'}));//loads config file

function handleHttpRequest(req, res) {
    try {
	console.log('HTTP_REQUEST: ' + req.connection.remoteAddress + ' to URL ' + req.url);

	//redirect access to dir to default file
	if (req.url.charAt(req.url.length - 1) == '/') {
	    req.url += config.directoryIndex;
	}

	var targetPath = path.normalize(config.webRoot + req.url),
	    extension = path.extname(targetPath).substr(1);

	fs.exists(targetPath, function (exists) {
	    if (exists) {
		res.statusCode = 200;
		res.setHeader('Content-type', extTranslator(extension));
		//stream file content to client
		fs.createReadStream(targetPath).pipe(res);
	    } else {
		res.statusCode = 404;
		res.end('404 Not Found');
	    }
	});
    } catch (e) {
	console.log('ERROR: ' + e.message);
	res.statusCode = 500;
	res.end('500 Server error occurred');
    }
}

http.createServer(handleHttpRequest).listen(2015);
