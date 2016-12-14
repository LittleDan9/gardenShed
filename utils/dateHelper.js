var months = ["January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"];

var currentDateTimeString = function(){
    return dateTimeString(new Date());
}


var dateTimeString = function dateTimeStringFromDate(d){
    if(d instanceof Date){
        var hour = d.getHours();
        var dtString =  months[d.getMonth()] + ' ' + 
                        d.getDate().toString() + ', ' + 
                        d.getFullYear().toString() + ' at ' + 
                        (hour == 0 ? '12' : (hour > 12 ? (hour-12) : hour).toString()) + ':' + 
                        (d.getMinutes() < 10 ? '0' + d.getMinutes().toString() : d.getMinutes().toString()) + ' ' + 
                        (hour > 12 ? 'PM' : 'AM');
        return dtString;
    }else{
        return "Invalid date provided";
    }
}

module.exports = {
    months,
    currentDateTimeString,
    dateTimeString
}