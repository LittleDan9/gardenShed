var mysql = require('mysql');
var connInfo = require('../configs/mysqlConfig.js');

var user = function user (userId, name, eMail, cellPhone, smsAddress, sendAlerts, isActive){
    this.userId = userId;
    this.name = name;
    this.eMail = eMail;
    this.cellPhone = cellPhone;
    this.smsAddress = smsAddress;
    this.sendAlerts = sendAlerts;
    this.isActive = isActive;
};

var getUsers = function(callback){
    var sqlConn = mysql.createConnection(connInfo.gardenShedConn);
    try{        
        sqlConn.connect(function(err){
            if(err)
                throw err;

            sqlConn.query('SELECT * FROM tUsers WHERE isActive = 1', function(err, rows, fields){
                if(err)
                    throw err;

                if(rows.length <= 0){
                    console.log("No Users");
                    callback(null);
                    return;
                }

                var users = new Array();
                for(i=0, len = rows.length; i < len; i++){
                    users.push(new user(rows[i].UserID, rows[i].Name, rows[i].EMail, rows[i].CellPhone, rows[i].SMSAddress, rows[i].SendAlerts[0], rows[i].isActive[0]));
                }
                callback(users);
                sqlConn.end();
            });
        });
    }catch(err){
        console.error(err);
        sqlConn.destroy();
    }
    //callback(null);
}

module.exports = {
    user,
    getUsers,
};