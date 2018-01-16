 CREATE TABLE `tConditions` (
  `ConditionID` int(11) NOT NULL AUTO_INCREMENT,
  `BoardID` int(11) NOT NULL,
  `Updated` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `Temperature` decimal(5,2) DEFAULT NULL,
  `Humidity` decimal(5,2) DEFAULT NULL,
  `Status` int(11) NOT NULL,
  PRIMARY KEY (`ConditionID`),
  UNIQUE KEY `ConditionID_UNIQUE` (`ConditionID`)
  ) ENGINE=InnoDB AUTO_INCREMENT=5 DEFAULT CHARSET=latin1;