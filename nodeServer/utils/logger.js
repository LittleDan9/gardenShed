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
                    //console.log('test');
                    //console.log(result);
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
    //console.log('Log Conditions For Board');
    var sqlConn = new dbConn(connInfo.gardenShedConn);
    board.getConditions(false, function(conditions){
        monitor.compareTemperature(conditions, board.boardId);
        sqlConn.query('INSERT INTO tConditions SET BoardID = :BoardID, Temperature = :Temperature, Humidity = :Humidity, Status = :Status', { BoardID: board.boardId, Temperature: conditions.temp, Humidity: conditions.humidity, Status: conditions.status}, function(err, result){
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