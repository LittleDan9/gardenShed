
var net = require('net');
var mysql = require('mysql');

//Local Scripts
var dateHelp = require('../utils/dateHelper.js')
var connInfo = require('../configs/mysqlConfig.js');

//Global variables
var command = 'TH';

var conditions = function (temp, humidity, status, observed, isLive){
    this.temp = temp;
    this.humidity = humidity;
    this.status = status;
    this.observed = !(observed instanceof Date) ? new Date() : observed
    this.isLive = isLive;
}

var conditionsByBoard = function(board, fromDB, callback){
    var client = new net.Socket();
    var options = {
        port:board.port,
        host:board.hostname,
    }    
    client.connect(options, function(data) {
        try{
        client.write(command);
        }catch(err){
            console.error(err);
            client.destroy();
        }
    });    
    client.on('data', function(data) {      
        try{
            //Parse the response a JSON object
            var response = JSON.parse(data);
            var temp = -1;
            var humid = -1;
            var status = response.data.status;
            var observed = '';
            var isLive = false;
            if(response.data.status >= 0){
                temp = response.data.conditions.temp;
                humid = response.data.conditions.humid;
                isLive = true;    
                callback(new conditions(temp, humid, status, observed, isLive));  
                client.end();        
            }else if(fromDB){
                //Atempt to get from DB
                var sqlConn = mysql.createConnection(connInfo.gardenShedConn);
                try{
                    sqlConn.connect();
                    var query = "SELECT Temperature, Humidity, Updated FROM tConditions WHERE BoardID = ? ORDER BY ConditionID DESC LIMIT 1"
                    sqlConn.query(query, [board.boardId], function(err, rows, fields){
                        console.log(response.data.status);
                        if(err)
                            throw err;
                        callback(new conditions(rows[0].Temperature, rows[0].Humidity, 0, rows[0].Updated, false));
                        client.end();
                    });
                }catch(err){
                    client.destroy();
                }
            }            
        }catch(err){
            console.error(err);
            client.destroy();
        }   
    });

    client.on('close', function() {
        //Right now do nothing.
    });   
}

module.exports = {
    conditions,
    conditionsByBoard
}