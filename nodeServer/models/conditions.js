
var net = require('net');
var dbConn = require('mariasql');

//Local Scripts
//var dateHelp = require('../utils/dateHelper.js');
var connInfo = require('../configs/dbConfig.js');

//Global variables
var command = 'TH';

var conditions = function (temp, humidity, status, observed, isLive){
    this.temp = temp;
    this.humidity = humidity;
    this.status = status;
    this.observed = !(observed instanceof Date) ? new Date() : observed;
    this.isLive = isLive;
};

var conditionsByBoard = function(board, fromDB, callback){
    var options = {
        port:board.port,
        host:board.hostname,
    };
    try{
        var client = new net.Socket();
        client.connect(options, function(data) {
            try{
                client.write(command);
            }catch(err){
                console.error('Unable to connect to:' + options);
                console.error(data);
                console.error(err);
                client.destroy();
            }
        });    
        client.on('data', function(data) { 
            if(data == '')  
                return;
            try{
                //Parse the response a JSON object
                var response = JSON.parse(data);
                var temp = -1;
                var humid = -1;
                var status = response.data.status;
                var observed = '';
                var isLive = false;
                //console.log(response);
                if(response.data.status >= 0){
                    temp = response.data.conditions.temp;
                    humid = response.data.conditions.humid;
                    isLive = true;    
                    callback(new conditions(temp, humid, status, observed, isLive));  
                    client.end();        
                }else if(fromDB){
                    //Atempt to get from DB
                    var sqlConn = new dbConn (connInfo.gardenShedConn);
                    try{
                        sqlConn.connect();
                        var query = 'SELECT Temperature, Humidity, Updated FROM tConditions WHERE BoardID = ? ORDER BY ConditionID DESC LIMIT 1';
                        sqlConn.query(query, [board.boardId], function(err, rows/*, fields*/){
                            //console.log(response.data.status + "test");
                            if(err)
                                throw err;
                            callback(new conditions(rows[0].Temperature, rows[0].Humidity, 0, rows[0].Updated, false));
                            client.end();
                        });
                    }catch(err){
                        console.error('Failed reading database:');
                        console.error(err);
                        client.destroy();
                    }
                }else{
                    client.end();
                }
                client.destroy();
            }catch(err){
                console.error('Data Received Error:');
                console.log(data);
                console.error(err);
                client.destroy();
            }   
        });

        client.on('close', function() {
            //console.log('Client Closed');
            //Right now do nothing.
        });   
    }catch(err){
        console.error('Issue connecting to remote server. ');
        console.error(options);
        console.error(err);
    }
};

module.exports = {
    conditions,
    conditionsByBoard
};