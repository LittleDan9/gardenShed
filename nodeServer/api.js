//import { SIGBREAK } from 'constants';

/*************************************************************************************************************
 * Title: Garden Shed Sensor Site
 * Description: Express Site / API for interating with sensors in the Garden Shed.
 * Author: Daniel R. Little
 * Company: Little Squared, Inc.
 * Date: 12/11/2016
/*************************************************************************************************************
 * Setup Forking in the event of a kill from a non-handled error
 **********************************************************************/
var cluster = require('cluster');
if (cluster.isMaster) {
    // for(i = 0; i < 4; i++){
    // 	cluster.fork();
    // }
    cluster.fork();

    cluster.on('exit', function (worker) {
        console.log('Worker ' + worker.id + 'died');
        cluster.fork();
    });
} else {
    //Check to make sure the database config file was updated.
    var sqlConfig = require('./configs/dbConfig.js');
    if (sqlConfig.gardenShedConn.host == '[host]') {
        console.error('You must configure your database connection. ./configs/dbConfig.js');
        process.exit(1);
        //return;
        //break;
    }
    sqlConfig = null;

    //Check to make sure the Notification config file was updated.
    var notifyConfig = require('./configs/notificationConfig.js');
    if (notifyConfig.smtp.host == 'SMTP_HOST') {
        console.error('You must configure your notification connection. ./configs/notificationConfig.js');
        process.exit(1);
    }
    notifyConfig = null;

    /***********************************************************************
	 * Required Libraries
	 **********************************************************************/
    var express = require('express');
    var bodyParser = require('body-parser');
    //var http = require('http');
    //var path = require('path');
    var app = express();


    /***********************************************************************
	 * Local Scripts
	 **********************************************************************/
    var sensorBoards = require('./models/sensorBoard.js');
    var notifications = require('./models/notification.js');
    var conditions = require('./models/conditions.js');
    var user = require('./models/user.js');
    var dtHelper = require('./utils/dateHelper.js');
    var logger = require('./utils/logger.js');

    app.use(express.static(__dirname));
    app.use(express.static('public'));
    //app.use(bodyParser.urlencode({extended:false}));
    //app.use(bodyParser.json());
    var urlencodedParser = bodyParser.urlencoded({ extended: false });

    /***********************************************************************
	 * Default Listener
	 **********************************************************************/
    app.listen(3000, function () {
        try {
            console.log('Started listening on port 3000 - Garden Shed Conditions');
            //Start Logger
            try {
                //console.log('Started Logging');
                logger.logConditions(function (result) {
                    try {
                        //console.log(result);
                    } catch (err) {
                        console.error('Logger Failed.');
                        console.error(result);
                        console.error(err);
                    }
                });
            } catch (err) {
                console.error('Logger Failed');
                console.error(err);
            }
            /*
            setInterval(function () {
                try {
                    //console.log('Started Logging');
                    logger.logConditions(function (result) {
                        try {
                            //console.log(result);
                        } catch (err) {
                            console.error('Logger Failed.');
                            console.error(result);
                            console.error(err);
                        }
                    });
                } catch (err) {
                    console.error('Logger Failed');
                    console.error(err);
                }
            }, 60000);
            */
        } catch (err) {
            console.error(err);
        }
    });
    /***********************************************************************
	 * Date/Time Routes
	 **********************************************************************/
    app.get('/DateString', function (req, resp) {
        var dtSig = req.query.dt;
        if (!dtSig)
            resp.send('Invalid Date Time Signature');
        else {
            var d = new Date(dtSig);
            resp.send('{"dtString":"' + dtHelper.dateTimeString(d) + '"}');
        }
    });

    app.get('/DateTimeNow', function (req, resp) {
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
    /***********************************************************************
	 * Shed Condition Route
	 **********************************************************************/
    app.get('/Conditions/:boardId', function (req, resp) {
        try {
            var boardId = req.params.boardId;
            sensorBoards.getBoard(boardId, function (board) {
                //console.log(board);
                try{
                    var errMessage;
                    if(!board){
                        errMessage = 'Board ' + boardId + ' isn\'t registered!'; 
                    }else if(board.isActive){
                        board.getConditions(true, function (conditions) {
                            resp.send(conditions);
                        });
                    }else{
                        errMessage = 'Board ' + boardId + ' isn\'t active!';
                    }
                    if(errMessage && errMessage.length > 0){
                        console.error(errMessage);
                        resp.send(new conditions.conditions(NaN, NaN, -1, new Date(), false));
                    }
                }catch (err){
                    console.error(err);
                }
            });
        } catch (err) {
            console.error(err);
        }
    });

    /***********************************************************************
	 * Notification Routes
	 **********************************************************************/
    app.get('/notifications', function (req, resp) {
        notifications.getNotifications(function (result) {
            resp.send(result);
        });
    });

    app.get('/notifications/active', function (req, resp) {
        notifications.getNotifications(function (result) {
            var activeNotifications = result.filter(function (item) {
                return (item.isActive == true);
            });
            resp.send(activeNotifications);
        });
    });

    app.get('/notifications/inactive', function (req, resp) {
        notifications.getNotifications(function (result) {
            var activeNotifications = result.filter(function (item) {
                return (item.isActive == false);
            });
            resp.send(activeNotifications);
        });
    });

    app.get('/notifications/delete/:notificationId', function (req, resp) {
        notifications.delete(req.params.notificationId, function (response) {
            resp.send(response);
        });
    });

    app.post('/notifications/add', urlencodedParser, function (req, resp) {
        if (!req.body) return resp.sendStatus(400);
        var temp = req.body.txtTemp;
        var criteria = req.body.ddlCriteria;
        var boardId = req.body.ddlBoard;
        var isGreaterThan = false;
        var isLessThan = false;
        var isEqualTo = false;

        if (criteria.indexOf('>') >= 0) isGreaterThan = true;
        if (criteria.indexOf('<') >= 0) isLessThan = true;
        if (criteria.indexOf('=') >= 0) isEqualTo = true;

        notifications.create(temp, isGreaterThan, isLessThan, isEqualTo, boardId, function (n) {
            resp.send(n);
        });
    });

    /***********************************************************************
	 * User Routes
	 **********************************************************************/
    app.get('/users', function (req, resp) {
        user.getUsers(function (results) {
            resp.send(results);
        });
    });
}