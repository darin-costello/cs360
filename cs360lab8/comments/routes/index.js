var express = require('express');
var router = express.Router();

var mongoose = require('mongoose');
var request = require('request');

mongoose.connect('mongodb://localhost/commentDB');

var commentSchema = mongoose.Schema({
	Name: String,
	Comment: String
});

var Comment = mongoose.model('Comment', commentSchema);

var personSchema = mongoose.Schema({
			Name:  String,
			URL: String
});
var Pics = mongoose.model('Pics', personSchema);

var db = mongoose.connection;
db.on('error', console.error.bind(console, 'connection Error:'));
db.once('open', function() {
	console.log('connected');
});

var cage  ='http://theworstthingsforsale.com/wp-content/uploads/2014/02/nic-cage-personal.jpg'
/* GET home page. */
router.get('/', function(req, res, next) {
  res.render('index', { title: 'Express' });
});

router.post('/comment', function(req, res, next) {
	console.log("POST comment route");
	var newcomment = new Comment(req.body);
	console.log(newcomment);
	newcomment.save(function (err, post) {
		if(err) return console.err(err);
		console.log(post);
		res.sendStatus(200);
	});
})

router.get('/comment', function(req, res, next) {
	console.log("In the Get route");
	Comment.find(function(err, commentList) {
		if(err) return console.error(err);
		else {
			res.json(commentList)
		}
	});
});


router.get(/^\/ppic\/(\w+)/, function (req, res) {
	console.log("get Pic");
	Pics.findOne({'Name': req.params[0]}, function (err, person){
		if(err) return console.error(err);
		else {
			console.log(person);
			var answer;
			if(person == null) answer = cage;
			else answer = person.URL;
			
			request(answer).pipe(res);
		}
	})	
})

router.post(/^\/ppic\/(\w+)/, function(req, res, next) {
	console.log("Post pic", req.params[0])
	var pics = new Pics(req.body);
	console.log(pics);
	pics.save(function (err, post) {
		if(err) return console.err(err);
		console.log(post);
		res.sendStatus(200);
		});
});

module.exports = router;
