var cluster = require('cluster');
var numCPUs = require('os').cpus().length;

if (cluster.isMaster) {
	console.log("numCPUS " + numCPUs);
	//OK ignore cpu count for now.
	numCPUs = 4;

	for(var i = 0; i < numCPUs; i++){
		cluster.fork();
	}
	cluster.on('eit', function(worker, code, singal) {
		console.log('worker ' + worker.process.pid + ' died');
	});
} else {
	console.log( "Worker" + process.pid);
	require("./bin/www");
}
