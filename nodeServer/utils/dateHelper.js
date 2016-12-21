var months = ["January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"];

var currentDateTimeString = function(){
    return dateTimeString(new Date());
}


var dateTimeString = function dateTimeStringFromDate(d){
    if(d instanceof Date){
        var hour = d.getHours();
        var dtString =  dateString(d) + ' at ' + timeString(d);
        return dtString;
    }else{
        return "Invalid date provided";
    }
}

var dateString = function dateStringFromDate(d){
    var dString = monthd[d.getMonth()] + ' ' + d.getDate().toString() + ', '  + d.getFullYear().toString();
    return dString;
}

var timeString = function timeStringFromDate(d){
    var tString = (hour == 0 ? '12' : (hour > 12 ? (hour-12) : hour).toString()) + ':' + 
                  (d.getMinutes() < 10 ? '0' + d.getMinutes().toString() : d.getMinutes().toString()) + ' ' + 
                  (hour > 12 ? 'PM' : 'AM');
}

module.exports = {
    months,
    currentDateTimeString,
    dateTimeString,
    dateString,
    timeString
}