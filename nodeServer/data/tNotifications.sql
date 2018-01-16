CREATE TABLE `tNotifications` (
  `NotificationID` int(11) NOT NULL AUTO_INCREMENT,
  `CompareValue` int(11) NOT NULL,
  `Created` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `isGreaterThan` bit(1) NOT NULL DEFAULT b'0',
  `isLessThan` bit(1) NOT NULL DEFAULT b'0',
  `isActive` bit(1) DEFAULT b'1',
  `LastSent` datetime DEFAULT NULL,
  `isEqualTo` bit(1) DEFAULT b'0',
  `BoardID` int(11) NOT NULL,
  PRIMARY KEY (`NotificationID`),
  KEY `BoardID_idx` (`BoardID`),
  CONSTRAINT `BoardID` FOREIGN KEY (`BoardID`) REFERENCES `tBoards` (`BoardID`) ON DELETE NO ACTION ON UPDATE NO ACTION
) ENGINE=InnoDB AUTO_INCREMENT=29 DEFAULT CHARSET=latin1;