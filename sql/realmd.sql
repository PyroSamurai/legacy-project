/*
SQLyog Community Edition- MySQL GUI v5.20
Host - 5.0.51a-community-nt : Database - realmd
*********************************************************************
Server version : 5.0.51a-community-nt
*/

SET NAMES utf8;

SET SQL_MODE='';

USE `realmd`;
SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO';

/*Table structure for table `accounts` */

DROP TABLE IF EXISTS `accounts`;

CREATE TABLE `accounts` (
  `accountid` int(11) NOT NULL default '0',
  `name` varchar(255) NOT NULL,
  `md5pass1` varchar(128) NOT NULL,
  `md5pass2` varchar(128) NOT NULL,
  `gmlevel` tinyint(1) NOT NULL default '0',
  `online` int(1) NOT NULL default '0',
  `email` varchar(30) default NULL,
  PRIMARY KEY  (`accountid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

SET SQL_MODE=@OLD_SQL_MODE;