/*************************************************************************************************************
 * Title: Garden Shed Sensor Site
 * Description: Express Site / API for interating with sensors in the Garden Shed.
 * Author: Daniel R. Little
 * Company: Little Squared, Inc.
 * Date: 12/11/2016
/*************************************************************************************************************/
var express = require('express');
var http = require('http');
var path = require('path');
var app = express();


//Local Scripts
var sensorBoards = require('./models/sensorBoards.js')
var dtHelper = require('./utils/dateHelper.js');
var logger = require('./utils/logger.js');

app.use(express.static(__dirname));
app.use(express.static('public'));

var server = app.listen(3000, function(){
	try{
		console.log('Started listening on port 3000 - Garden Shed Conditions');
		setInterval(function(){
			logger.logConditions(function(result){
				//console.log(result);
			})
		}, 60000)
	}catch(err){
		console.error(err);
	}
});

app.get('/DateString', function(req, resp){
	var dtSig = req.query.dt;
	if(!dtSig)
		resp.send('Invalid Date Time Signature');
	else{
		var d = new Date(dtSig);
		resp.send('{"dtString":"' + dtHelper.dateTimeString(d) + '"}');
	}
});

app.get('/Conditions/:boardId', function (req, resp) {
	try{	
		var boardId = req.params.boardId;
		sensorBoards.getBoard(boardId, function(board){
			board.getConditions(true, function(conditions){
				resp.send(conditions);
			});		
		});	
	}catch(err){
		console.error(err);
	}
}); 