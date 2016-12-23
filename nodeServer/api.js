/*************************************************************************************************************
 * Title: Garden Shed Sensor Site
 * Description: Express Site / API for interating with sensors in the Garden Shed.
 * Author: Daniel R. Little
 * Company: Little Squared, Inc.
 * Date: 12/11/2016
/*************************************************************************************************************/
var cluster = require('cluster');
if(cluster.isMaster){
	// for(i = 0; i < 4; i++){
	// 	cluster.fork();
	// }
	cluster.fork();

	cluster.on('exit', function(worker){
		console.log('Worker ' + worker.id + 'died');
		cluster.fork();
	});
}else{
	//Check to make sure the mySQL config files was updated.
	var sqlConfig = require('./configs/mysqlConfig.js');
	if(sqlConfig.gardenShedConn.host == "[host]"){
		console.error("You must configure your mysql connection. ./configs/mysqlConfig.js");
		return;
	}
	sqlConfig = null;

	var express = require('express');
	var http = require('http');
	var path = require('path');
	var app = express();

	//Local Scripts
	var sensorBoards = require('./models/sensorBoard.js')
	var notifications = require('./models/notification.js')
	var dtHelper = require('./utils/dateHelper.js');
	var logger = require('./utils/logger.js');

	app.use(express.static(__dirname));
	app.use(express.static('public'));
	//app.use(bodyParser.json);

	var server = app.listen(3000, function(){
		try{
			console.log('Started listening on port 3000 - Garden Shed Conditions');
			//Start Logger
			try{
				//console.log('Started Logging');
				logger.logConditions(function(result){
					try{
						//console.log(result);
					}catch(err){
						console.error("Logger Failed.");
						console.error(err);
					}
				});
			}catch(err){
				console.error("Logger Failed");
				console.error(err);
			}
			setInterval(function(){
				try{
					console.log('Started Logging');
					logger.logConditions(function(result){
						try{
							//console.log(result);
						}catch(err){
							console.error("Logger Failed.");
							console.error(err);
						}
					});
				}catch(err){
					console.error("Logger Failed");
					console.error(err);
				}
			}, 60000);			
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

	app.get('/DateTimeNow', function(req, resp){
		var Now = new Date();
		resp.send('{"date":"' + dtHelper.dateString(Now) + '", "time":"' + dtHelper.timeString(Now) + '"' + 
		', "hr":"' + Now.getHours() + '"' +
		', "min":"' + Now.getMinutes() + '"' +
		', "sec":"' + Now.getSeconds() + '"' +
		', "day":"' + Now.getDate() + '"' +
		', "month":"' + Now.getMonth() + '"' +
		', "year":"' + Now.getFullYear() + '"' +
		'}');
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

	app.get('/notification/get', function (resp, resp){
		
		notifications.getNotifications(function(result){
			resp.send(result);
		});
	});

	app.get('/notification/delete/:notificationId', function (req, resp){
		
		notifications.del(req.params.notificationId);
	})

	app.post('/notfication/add/', function(req,resp){
		console.log(req.body.compareValue);
		console.log(req.body.isGreaterThan);
		console.log(req.body.isLessThan);
		console.log(req.body.isEqualTo);
	});
}