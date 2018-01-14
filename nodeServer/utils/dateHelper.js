var months = ['January', 'February', 'March', 'April', 'May', 'June', 'July', 'August', 'September', 'October', 'November', 'December'];

var currentDateTimeString = function(){
    return dateTimeString(new Date());
};


var dateTimeString = function dateTimeStringFromDate(d){
    if(d instanceof Date){
        var dtString =  dateString(d) + ' at ' + timeString(d);
        return dtString;
    }else{
        return 'Invalid date provided';
    }
};

var dateString = function dateStringFromDate(d){
    var dString = months[d.getMonth()] + ' ' + d.getDate().toString();
    return dString;
};

var timeString = function timeStringFromDate(d){
    var hour = d.getHours();
    var mins = d.getMinutes();
    var tString = (hour == 0 ? '12' : (hour > 12 ? (hour-12) : hour).toString()) + ':' + 
                  (mins < 10 ? '0' + mins.toString() : mins.toString()) + ' ' + 
                  (hour >= 12 ? 'PM' : 'AM');
    return tString;
};

module.exports = {
    months,
    currentDateTimeString,
    dateTimeString,
    dateString,
    timeString
};
