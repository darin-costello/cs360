var express = require('express');
var router = express.Router();
var fs = require('fs')
/*
 * Returns the result of having each alphabetic letter of the given text string shifted forward
 * by the given amount, with wraparound. Case is preserved, and non-letters are unchanged.
 * Examples:
 *   caesarShift("abz",  0) = "abz"
 *   caesarShift("abz",  1) = "bca"
 *   caesarShift("abz", 25) = "zay"
 *   caesarShift("THe 123 !@#$", 13) = "GUr 123 !@#$"
 */
function caesarShift(text, shift) {
	var result = "";
	var a = "a".charCodeAt(0);
	text = text.toLowerCase()
	for (var i = 0; i < text.length; i++) {
		var c = text.charCodeAt(i);
		var lettNu  = c - a;
		var test =(lettNu + shift) % 26  ;
		if(test < 0) test += 26;
		if (lettNu >= 0 && lettNu < 26) result += String.fromCharCode( test + a);
		else    result += text.charAt(i);  // Copy
	}
	return result;
}

/* GET home page. */
router.get('/', function(req, res, next) {
  res.sendFile('lab6.html', { root: 'public' });
});

router.get('/getcity', function(req, res, nex) {
	console.log("In getcity route");
	var myRe = new RegExp("^" + req.query.q);
	fs.readFile(__dirname + '/cities.dat.txt', function(err, data){
	if(err) throw err;
	var cities = data.toString().split("\n");
	var jsonresult = [];
	for(var i = 0; i< cities.length; i++) {
		var result = cities[i].search(myRe);
		if(result != -1) {
			console.log(cities[i]);
			jsonresult.push({city:cities[i]});
		}
	}
	res.status(200).json(jsonresult);
	});
});

router.post('/code', function(req, res, nex){
	var tex =  req.body.text
	var shift = parseInt(req.body.shift)
	res.send(caesarShift(tex,shift));
	});
router.post('/decode', function(req, res, nex){
	var tex = req.body.text
	var shift = parseInt(req.body.shift)
	res.send(caesarShift(tex,-1 * shift));
})


module.exports = router;
