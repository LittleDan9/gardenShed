var dbConn = require('mariasql');
var connInfo = require('../configs/dbConfig.js');
var conditions = require('./conditions.js');

function board(boardId, chipId, desc, created, modified, hostname, port) {
    this.boardId = boardId;
    this.chipId = chipId;
    this.description = desc;
    this.created = created;
    this.modified = modified;
    this.hostname = hostname;
    this.port = port;
}

board.prototype.getConditions = function (fromDB, callback) {
    try {
        conditions.conditionsByBoard(this, fromDB, function (conditions) {
            callback(conditions);
        });
    } catch (err) {
        console.error('Unable to get conditions');
        console.error(err);
    }
};

board.prototype.update = function (callback) {
    var newBoard = this;
    if (newBoard.boardId > 0) {
        getBoard(newBoard.boardId, function (oldBoard) {
            if (!oldBoard)
                throw new Error('Board does not exist');

            var sqlConn = dbConn.createConnection(connInfo.gardenShedConn);
            try {
                sqlConn.connect(function (err) {
                    if (err)
                        throw err;

                    //Revent back to the old boards data if the new data is empty or undefined.                
                    newBoard.chipId = !newBoard.chipId ? oldBoard.chipId : newBoard.chipId;
                    newBoard.description = !newBoard.description ? oldBoard.description : newBoard.description;
                    newBoard.hostname = !newBoard.hostname ? oldBoard.hostname : newBoard.hostname;
                    newBoard.port = !newBoard.port ? oldBoard.port : newBoard.port;

                    //Update the dabase with the new information
                    sqlConn.query('UPDATE tBoards SET ChipID = ?, Description = ?, Hostname = ?, Port = ?  WHERE BoardID = ?', [newBoard.chipId, newBoard.description, newBoard.hostname, newBoard.port, newBoard.boardId], function (err, results) {
                        if (err)
                            console.log(err);
                        callback(results);
                        sqlConn.end();
                    });
                });
            } catch (err) {
                console.error(err);
                sqlConn.destroy();
            }
        });
    } else {
        throw new Error('Invalid Board ID.');
    }
    return;
};

var boards = function getBoards(callback) {
    var sqlConn = dbConn.createConnection(connInfo.gardenShedConn);
    try {
        sqlConn.connect(function (err) {
            if (err)
                throw err;

            sqlConn.query('SELECT * FROM tBoards WHERE isActive = 1', function (err, rows, fields) {
                //console.log(rows);
                if (err)
                    throw err;

                var boards = new Array();
                for (var i = 0, len = rows.length; i < len; i++) {
                    boards.push(new board(rows[i].BoardID, rows[i].ChipID, rows[i].Description, rows[i].Created, rows[i].Updated, rows[i].Hostname, rows[i].Port));
                }
                callback(boards);
                sqlConn.end();
            });
        });
    } catch (err) {
        console.error(err);
        sqlConn.destroy();
    }
    return;
};

var getBoard = function getBoard(boardId, callback) {
    var sqlConn = dbConn.createConnection(connInfo.gardenShedConn);
    //console.log(boardId + "test");
    try {
        sqlConn.connect(function (err) {
            if (err)
                throw err;

            sqlConn.query('SELECT * FROM tBoards WHERE BoardID = ?', [boardId], function (err, rows, fields) {
                if (err)
                    throw err;
                //console.log(rows);
                sqlConn.end();
                if (rows.length > 0) {
                    callback(new board(rows[0].BoardID, rows[0].ChipID, rows[0].Description, rows[0].Created, rows[0].Updated, rows[0].Hostname, rows[0].Port));
                } else {
                    callback(null);
                }
            });
        });
        return;
    } catch (err) {
        console.error(err);
        sqlConn.destroy();
        callback(null);
    }
};

module.exports = {
    boards,
    getBoard
};
