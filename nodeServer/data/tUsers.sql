CREATE TABLE `tUsers` (
  `UserID` int(11) NOT NULL AUTO_INCREMENT,
  `Name` varchar(200) NOT NULL,
  `EMail` varchar(255) NOT NULL,
  `CellPhone` varchar(255) DEFAULT NULL,
  `SMSAddress` varchar(200) DEFAULT NULL,
  `SendAlerts` bit(1) DEFAULT b'0',
  `isActive` bit(1) DEFAULT b'1',
  PRIMARY KEY (`UserID`)
) ENGINE=InnoDB AUTO_INCREMENT=3 DEFAULT CHARSET=latin1;