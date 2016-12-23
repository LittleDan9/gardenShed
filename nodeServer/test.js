
var notification = require('./models/notification.js');
notification.getNotifications(function(data){
    if(data.length > 0)
    {
        data[0].send("Test", "Test", true, function(response){
            console.log(response);
        })
    }
});