var notification = require("../models/notification.js");

var compareTemperature = function(conditions){
    //console.log("In Monitor: ");
    //console.log(conditions);
    notification.getNotifications(function(notifications){
        if(notifications == undefined)
        {   
            console.log("Invalid Conditions in Monitor.");
            return false
        }
        
        for(i = 0; i < notifications.length; i++){
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

            if(send){
                var icons = '⚠❗🚨🌱';
                var txtMessage = "";
                txtMessage += '🚨Conditions Alert🚨\n';
                txtMessage += '-------------------------\n';
                //txtMessage += '' + " Observation Time: " + conditions.observed + "\n";
                txtMessage += "🌡 Temperature: " + conditions.temp + "°\n";
                txtMessage += "💧 Humidity: " + conditions.humidity + "%\n";
                txtMessage += "🌱🌱🌱🌱🌱🌱🌱🌱🌱";

                var htmlMessage = '<body>';
                htmlMessage += '<div>';
                htmlMessage += '<h1><center>🚨Conditions Alert🚨</center></h1>';
                htmlMessage += '<hr />';
                //htmlMessage += '<p><h2>Observed on: ' + conditions.observed + '</h2></p>';
                htmlMessage += '<p><h2 style="padding-left:20px;">🌡Temperature: ' + conditions.temp + '°</h2></p>';
                htmlMessage += '<p><h2 style="padding-left:20px;">💧 Humidity: ' + conditions.humidity + '%</h2></p>';
                htmlMessage += '<p style="text-align:center;"><center>🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱</center></p>';
                htmlMessage += '</div>';
                htmlMessage += '</body>';
                notification.send(txtMessage, htmlMessage, false, function(result){
                //console.log(result);
                });
            }
        }
    });
}

module.exports = {compareTemperature};