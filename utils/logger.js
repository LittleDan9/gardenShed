var mysql = require('mysql');
var connInfo = require('../configs/mysqlConfig.js')
var sensorBoards = require('../models/sensorBoards.js')

var logConditions = function (callback){
    sensorBoards.boards(function(boards){ 
        //console.log(boards);
        for(i = 0, len = boards.length; i < len; i++ ){  
            logConditionsForBaord(boards[i], function(result){
                callback(result);
            });
        }
    });
}

var logConditionsForBaord = function (board, callback){
    var sqlConn = mysql.createConnection(connInfo.gardenShedConn);
    board.getConditions(false, function(conditions){
        sqlConn.query('INSERT INTO tConditions SET ?', { BoardID: board.boardId, Temperature: conditions.temp, Humidity: conditions.humidity, Status: conditions.status}, function(err, result){
            if(err)
                throw err;
                        
            callback(result.insertId);
            sqlConn.end();
        });
    });
}

module.exports = {
    logConditions,
    logConditionsForBaord,
}