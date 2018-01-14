var notification = require('../models/notification.js');
//var sensorBoards = require('../models/sensorBoard.js');

var compareTemperature = function(conditions, boardId){
    // console.log("In Monitor: ");
    // console.log(conditions);
    // console.log(boardId);
    notification.getNotificationsForBoard(boardId, function(notifications){
        if(notifications == undefined)
        {   
            console.log('Invalid Conditions in Monitor.');
            return false;
        }

        for(var i = 0; i < notifications.length; i++){
            var send = false;
            var notification = notifications[i];
            //console.log("Temp: " + conditions.temp + " Compare To: " + notification.compareValue);
            if(!notification.isActive){
                continue;
            }else if(notification.isGreaterThan &&  conditions.temp > notification.compareValue){
                //console.log("isGreaterThan");
                send = true;
            }else if(notification.isLessThan && conditions.temp < notification.compareValue){
                //console.log("isLessThan");
                send = true;
            }else if(notification.isEqualTo && conditions.temp == notification.compareValue){
                //console.log("isEqual");
                send = true;
            }
            //console.log('Send: ' + send);

            //console.log(notification);

            if(send){
                //var icons = '⚠❗🚨🌱🐓';
                var txtMessage = '';
                txtMessage += '🚨' + notification.boardName + ' Alert🚨\n';
                txtMessage += '-------------------------\n';
                //txtMessage += '' + " Observation Time: " + conditions.observed + "\n";
                txtMessage += '🌡 Temperature: ' + conditions.temp + '°\n';
                txtMessage += '💧 Humidity: ' + conditions.humidity + '%\n';
                txtMessage += '🌱🐓🌱🐓🌱🐓🌱🐓🌱';
                //txtMessage += "🌱🌱🌱🌱🌱🌱🌱🌱🌱";

                var htmlMessage = '<body>';
                htmlMessage += '<div>';
                htmlMessage += '<h1><center>🚨' + notification.boardName + ' Alert🚨</center></h1>';
                htmlMessage += '<hr />';
                //htmlMessage += '<p><h2>Observed on: ' + conditions.observed + '</h2></p>';
                htmlMessage += '<p><h2 style="padding-left:20px;">🌡Temperature: ' + conditions.temp + '°</h2></p>';
                htmlMessage += '<p><h2 style="padding-left:20px;">💧 Humidity: ' + conditions.humidity + '%</h2></p>';
                htmlMessage += '<p style="text-align:center;"><center>🌱🐓🌱🐓🌱🐓🌱🐓🌱🐓🌱🐓🌱</center></p>';
                //htmlMessage += '<p style="text-align:center;"><center>🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱</center></p>';
                htmlMessage += '</div>';
                htmlMessage += '</body>';
                notification.send(txtMessage, htmlMessage, false, function(results){
                    if(!results){
                        console.error('notification.send failed');
                    }
                    //console.log(result);
                });
            }
        }
    });
};

module.exports = {compareTemperature};