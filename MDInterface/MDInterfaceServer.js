var express = require('express');
var app = express();
var server = app.listen(8090);
var io = require('socket.io').listen(server);
var fs = require('fs');
//var mysql = require('mysql');

/**
 * Default algorithm and interface parameters.
 */
var enableStream = true;
var enableMorph = true;
var enableContours = false;
var enableMask = true;
var varianceParam = 2;

/**
 * Handling database update.
 */
var databaseUpdate = false;
setInterval(databaseCallback, 500);



function databaseCallback() {
	if (state == 3) {
		databaseUpdate = true;
	} else {
		databaseUpdate = false;
	}
}

/**
 * State 0 - Processor not connected
 * State 1 - Setup process state
 * State 2 - No motion state
 * State 3 - Motion detected state
 */
var state = 0;


var ipc = require('node-ipc');
ipc.config.id = 'MDInterface';
ipc.config.rawBuffer = true;
ipc.config.encoding = 'ascii'
ipc.config.retry = 1000;
ipc.config.appspace = '';
ipc.config.socketRoot = '';

ipc.connectTo('node_stream', '/tmp/node_stream', function() {

});

ipc.connectTo('node_detection', '/tmp/node_detection', function() {

});

ipc.connectTo('node_algorithm', '/tmp/node_algorithm', function() {
	//refreshMorph();
	//refreshMask();
	//refreshVariance();
	//refreshContours();
});


function refreshMorph() {
	var morphMsg = (enableMorph) ? '11' : '10';
	ipc.of.node_algorithm.emit(morphMsg);
}

function refreshMask() {
	var maskMsg = (enableMask) ? '21' : '20';
	ipc.of.node_algorithm.emit(maskMsg);
}

function refreshVariance() {
	var varMsg = '3' + varianceParam;
	ipc.of.node_algorithm.emit(varMsg);
}

function refreshContours() {
	var contMsg = (enableContours) ? '41' : '40';
	ipc.of.node_algorithm.emit(contMsg);
}

// var connection = mysql.createConnection({
// 	host : 'localhost',
// 	user : 'root',
// 	port : 3306,
// 	password : 'toor',
// 	database : 'detection'
// });

app.use(express.static( __dirname + '/client' ));
app.set('view engine', 'ejs');
app.get('/', renderIndex);
app.get('/history', renderLog);
app.get('/about', renderAbout);

function renderLog(req, res) {
	// connection.query('SELECT date,image FROM motions', function (error, results, fields) {
	// 	res.render('pages/log', { motion: results });
	// });
}

function renderIndex(req, res) {
	res.render('pages/index', {enableStream: enableStream,
								enableMorph: enableMorph,
								enableContours: enableContours,
								enableMask: enableMask,
								varianceParam: varianceParam});
}

function renderAbout(req, res) {
	res.render('pages/about');
}

ipc.of.node_stream.on('data',
    function (data) {
    	if(enableStream) {
			var encoded = data.toString('ascii');
        	io.emit('data', encoded);
        	if (databaseUpdate) {
        		console.log("spremam u bazu");
        		saveToDatabase(encoded);
        		databaseUpdate = false;
        	}
    	}
});

ipc.of.node_detection.on('data',
	function (data) {
		if (state == 1 || state == 0) {
			return;
		}
		if (data == '1') {
			if (state == 2) {
				alarmUp();
			}
			state = 3;
		} else if (state == 3) {
			state = 2;
		}
});

ipc.of.node_algorithm.on('data',
	function (data) {
		if (state == 0) {
			state = 1;
			setTimeout(function () {
				state = 2;
				console.log("Utitro se!");
			}, 20 * 1000);
		}
});

function alarmUp() {
	console.log('alarmUp');
	io.emit('alarm', true);
}

function alarmDown() {
	console.log('alarmDown');
	io.emit('alarm', false);
}

function saveToDatabase(encoded) {
	// connection.beginTransaction(function(err) {
	// if (err) {
	// 	throw err;
	// }
	// connection.query('INSERT INTO motions SET ?', {image:encoded}, function (err, result) {
	// 	if (err) {
	// 		return connection.rollback(function() {
	// 			throw err;
	// 		});
	// 	}
	// });
	// connection.commit(function(err) {
    //     if (err) {
	// 		return connection.rollback(function() {
    //         	throw err;
    //       	});
    //     }
    //     console.log('Transaction completed successfully!');
	// 	});
	// });
}

io.sockets.on('connection', function (socket) {
	socket.on('enableStream', function(data) {
		enableStream = data;
		socket.broadcast.emit('enableStream', data);
	});
	socket.on('enableMorph', function(data) {
		enableMorph = data;
		socket.broadcast.emit('enableMorph', data);
		refreshMorph();
	});
	socket.on('enableContours', function(data) {
		enableContours = data;
		socket.broadcast.emit('enableContours', data);
		refreshContours();
	});
	socket.on('enableMask', function(data) {
		enableMask = data;
		socket.broadcast.emit('enableMask', data);
		refreshMask();
	});
	socket.on('varianceParam', function(data) {
		varianceParam = data;
		socket.broadcast.emit('varianceParam', data);
		refreshVariance();
	});
});

console.log("Server started...");
