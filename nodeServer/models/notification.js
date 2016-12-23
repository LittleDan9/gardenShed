var mysql = require('mysql');
var connInfo = require('../configs/mysqlConfig.js');
var user = require('./user.js');
var nodemailer = require('nodemailer');

var notification = function notification(notificationId, compareValue, created, isGreaterThan, isLessThan, isEqualTo, isActive, lastSent){
    this.notificationId = notificationId;
    this.compareValue = compareValue;
    this.created = created;
    this.isGreaterThan = isGreaterThan;
    this.isLessThan = isLessThan;
    this.isEqualTo = isEqualTo;
    this.isActive = isActive;
    this.lastSent = lastSent;
}

/*
Send a notification to all active user with the provided message
*/
notification.prototype.delete = function(callback){
    del(this.notificationId, callback);
};

var del = function(notificationId, callback){
    try{        
        sqlConn.connect(function(err){
            if(err)
                throw err;
            sqlConn.query("UPDATE tNotifications SET isActive = ? WHERE NotificationID = ?", [0, notificationId], function(err, results){                        
                if(err)
                    throw err;
                sqlConn.end();
                callback(success);
            });
        });
    }catch(err){
        console.error(err);
        sqlConn.destroy();
    }
}

notification.prototype.save = function(callback){
        try{        
        sqlConn.connect(function(err){
            if(err)
                throw err;
            sqlConn.query("INSERT INTO tNotifications SET ?", {CompareValue: this.compareValue, isGreater: this.isGreater, isLessThan: this.isLessThan, isEqualTo: this.isEqualTo }, function(err, results){                        
                if(err)
                    throw err;
                sqlConn.end();
                callback(success);
            });
        });
    }catch(err){
        console.error(err);
        sqlConn.destroy();
    }
}

notification.prototype.send = function(txtMessage, htmlMessage, forceSend, callback){
    var lastSentDate = new Date(this.lastSent);
    var now = new Date();
    var hourInMili = (60 * 60 * 1000);
    //  console.log(now);
    //  console.log(this.lastSent);
    //  console.log(now -  lastSentDate);
    //  console.log(hourInMili);

    if(now - lastSentDate <  hourInMili && !forceSend ){
        callback("Notification last sent less than an hour ago, skipping.");
        return;
    }

    if(forceSend)
        console.log("Force Send initiated.");
    
    var notificationId = this.notificationId
    //Get Users who get notifications.
    user.getUsers(function(users){
        var transporter = nodemailer.createTransport({
            host: 'smtp.gmail.com',
            port: 465,
            secure: true,
            auth: {
                user: 'gardenshed@littlerichele.com',
                pass: 'qwwvvifkqjgptckq'
            }        
        });

        var mailOptions = {
            from : "Garden Shed <gardenshed@littlerichele.com>",
            to : "",
            subject : "Garden Shed Alert",
            text : txtMessage,
            html : htmlMessage
        }

        var itemsSent = new Array();
        //for debug only
        var send = true;
        if(send){
            for(i = 0; i < users.length; i++){
                //console.log(users[i]);
                var sentItem = {to : users[i].eMail, htmlMessage: htmlMessage, txtMessage: txtMessage, eMailSuccess: false, smsSuccess: false};
                //Send notification to each
                if(users[i].eMail.length > 0){
                    mailOptions.to = users[i].name + ' <' + users[i].eMail + '>';

                    sentItem.success = transporter.sendMail(mailOptions, function(error, info){
                        if(error){
                            console.error(error);
                        }else{
                            console.log('Notification Messages Sent: ' + info.response);   
                        }
                    });
                }
                //console.log(mailOptions);

                if(users[i].cellPhone.length > 0 && users[i].smsAddress.length > 0){
                    mailOptions.to = users[i].name + "'s Cell <" + users[i].cellPhone + '@' + users[i].smsAddress + '>';
                    mailOptions.subject = "";
                    mailOptions.htmlMessage = "";
                    
                    eMailSuccess = sentItem.success = transporter.sendMail(mailOptions, function(error, info){
                        if(error){
                            console.error(error);
                        }
                        console.log('Notification Messages Sent: ' + info.response);   
                    });
                }
            }
        }
        var sqlConn = mysql.createConnection(connInfo.gardenShedConn);
        try{        
            sqlConn.connect(function(err){
                if(err)
                    throw err;
                sqlConn.query("UPDATE tNotifications SET LastSent = ? WHERE NotificationID = ?", [new Date().toMysqlFormat(), notificationId], function(err, results){                        
                    if(err)
                        throw err;
                    sqlConn.end();
                });
            });
        }catch(err){
            console.error(err);
            sqlConn.destroy();
        }    
        return true;      
    });    
}

var getNotification = function(notificationId, callback){
        var sqlConn = mysql.createConnection(connInfo.gardenShedConn);
    //console.log(boardId);
    try{        
        sqlConn.connect(function(err){
            if(err)
                throw err;

            sqlConn.query('SELECT * FROM tNotifications WHERE NotificationID = ? WHERE isActive = 1', [NotificationId], function(err, rows, fields){
                if(err)
                    throw err;
                //console.log(rows);
                sqlConn.end();
                if(rows.length > 0){
                    callback(new notification(rows[0].NotificationId, 
                                              rows[0].CompareValue, 
                                              rows[0].Created, 
                                              rows[0].isGreaterThan,
                                              rows[0].isLessThan,
                                              rows[0].isEqualTo,
                                              rows[0].isActive,
                                              rows[0].LastSent));
                }else{
                    callback(null);
                }
            });
        });
        return;
    }catch(err){
        console.error(err);
        sqlConn.destroy();
        callback(null);
    }  
}

var getNotifications = function getNotificiations(callback){
    //console.log("In getNotifications");
    var sqlConn = mysql.createConnection(connInfo.gardenShedConn);
    try{        
        sqlConn.connect(function(err){
            if(err)
                throw err;

            sqlConn.query('SELECT * FROM tNotifications WHERE isActive = 1', function(err, rows, fields){
                if(err)
                    throw err;

                var notifications = new Array();
                for(i=0, len = rows.length; i < len; i++){
                    notifications.push(new notification(rows[0].NotificationID, 
                                              rows[0].CompareValue, 
                                              rows[0].Created, 
                                              rows[0].isGreaterThan[0],
                                              rows[0].isLessThan[0],
                                              rows[0].isEqualTo[0],
                                              rows[0].isActive[0],
                                              rows[0].LastSent));
                }
                //console.log(notifications);
                callback(notifications);
                sqlConn.end();
            });
        });
    }catch(err){
        console.error(err);
        sqlConn.destroy();
    }
}

var create = function update(compareValue, isGreaterThan, isLessThan, isEqualTo){            
    var sqlConn = mysql.createConnection(connInfo.gardenShedConn);
    try{
        sqlConn.connect(function(err){
            if(err)
                throw err; 

        //Update the dabase with the new information
        sqlConn.query('INSERT INTO tNotifications SET ?', {CompareValue: compareValue, isGreaterThan : isGreaterThan, isLessThan : isLessThan, isEqualTo: isEqalTo}, function(err, results){
            if(err)
                throw err;
            
            sqlConn.end();
            callback(new notification(result.insertId, compareValue, new Date, isGreaterThan, isLessThan, isEqalTo, true));
            
        });
    });
    }catch(err){
        console.error(err);
        sqlConn.destroy();
    }
    callback(null);
}

function twoDigits(d) {
    if(0 <= d && d < 10) return "0" + d.toString();
    if(-10 < d && d < 0) return "-0" + (-1*d).toString();
    return d.toString();
}

Date.prototype.toMysqlFormat = function() {
    var value = this.getFullYear() + '-' + twoDigits(this.getMonth()+1) + '-' + twoDigits(this.getDate()) + ' ' + twoDigits(this.getHours()) + ':' + twoDigits(this.getMinutes()) + ':' + twoDigits(this.getSeconds());
    return value;
};


module.exports = {
    notification,
    create,
    getNotification,
    getNotifications
};