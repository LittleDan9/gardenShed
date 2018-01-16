CREATE TABLE `tBoards` (
  `BoardID` int(11) NOT NULL AUTO_INCREMENT,
  `ChipID` varchar(10) NOT NULL,
  `Name` varchar(45) DEFAULT NULL,
  `Description` varchar(45) DEFAULT NULL,
  `Created` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `Updated` timestamp NULL DEFAULT NULL,
  `Hostname` varchar(45) DEFAULT NULL,
  `Port` int(11) DEFAULT NULL,
  `isActive` bit(1) NOT NULL DEFAULT b'1',
  PRIMARY KEY (`BoardID`),
  UNIQUE KEY `BoardID_UNIQUE` (`BoardID`)
) ENGINE=InnoDB AUTO_INCREMENT=5 DEFAULT CHARSET=latin1;