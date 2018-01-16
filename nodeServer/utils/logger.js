var dbConn = require('mariasql');
var connInfo = require('../configs/dbConfig.js');
var sensorBoards = require('../models/sensorBoard.js');
var monitor = require('../utils/monitor.js');

var logConditions = function (callback){
    try{
        sensorBoards.boards(function(boards){ 
            //console.log(boards);
            for(var i = 0, len = boards.length; i < len; i++ ){  
                logConditionsForBaord(boards[i], function(result){
                    callback(result);
                });
            }
        });
    }catch(err){
        console.error('Log Conditions Failure');
        console.error(err);
    }
};

var logConditionsForBaord = function (board, callback){
    var sqlConn = dbConn.createConnection(connInfo.gardenShedConn);
    board.getConditions(false, function(conditions){
        //console.log("Excuting Monitor");
        monitor.compareTemperature(conditions, board.boardId);
        sqlConn.query('INSERT INTO tConditions SET ?', { BoardID: board.boardId, Temperature: conditions.temp, Humidity: conditions.humidity, Status: conditions.status}, function(err, result){
            if(err)
                throw err;
                        
            callback(result.insertId);
            sqlConn.end();
        });
    });
};

module.exports = {
    logConditions,
    logConditionsForBaord,
};