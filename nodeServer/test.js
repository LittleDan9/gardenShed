var sensorBoards = require('./sensorBoards.js')
sensorBoards.boards(function(boards){
    for(i = 0, len = boards.length; i < len; i++ ){
        boards[i].getConditions(function(conditions){
            console.log(conditions);
        });
    }
});