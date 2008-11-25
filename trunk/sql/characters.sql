/*
SQLyog Community Edition- MySQL GUI v5.20
Host - 5.0.51a-community-nt : Database - characters
*********************************************************************
Server version : 5.0.51a-community-nt
*/

SET NAMES utf8;

SET SQL_MODE='';

USE `characters`;
SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO';

/*Table structure for table `character_inventory` */

DROP TABLE IF EXISTS `character_inventory`;

CREATE TABLE `character_inventory` (
  `guid` int(11) unsigned NOT NULL default '0' COMMENT 'Global Unique Identifier',
  `pet` int(11) default '0',
  `slot` tinyint(3) unsigned NOT NULL default '0',
  `item` int(11) unsigned NOT NULL default '0' COMMENT 'Item Global Unique Identifier',
  `entry` int(11) unsigned NOT NULL default '0' COMMENT 'Item Identifier',
  PRIMARY KEY  (`item`),
  KEY `idx_guid` (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Player System';

/*Table structure for table `character_pet` */

DROP TABLE IF EXISTS `character_pet`;

CREATE TABLE `character_pet` (
  `guid` int(11) NOT NULL auto_increment,
  `owner_guid` int(11) default '0',
  `name` char(100) default '0',
  `data` longtext NOT NULL,
  `loyalty` int(11) default '0',
  `spell1_level` int(11) default '0',
  `spell2_level` int(11) default '0',
  `spell3_level` int(11) default '0',
  `spell4_level` int(11) default '0',
  `spell5_level` int(11) default '0',
  `npcflag` int(11) default '0',
  `petslot` int(11) default '0',
  `mode_battle` tinyint(1) default '0',
  PRIMARY KEY  (`guid`)
) ENGINE=InnoDB AUTO_INCREMENT=16 DEFAULT CHARSET=utf8;

/*Table structure for table `character_spell` */

DROP TABLE IF EXISTS `character_spell`;

CREATE TABLE `character_spell` (
  `guid` int(11) NOT NULL auto_increment,
  `owner_guid` int(1) default '0',
  `entry` int(11) default '0',
  `level` int(11) default '0',
  PRIMARY KEY  (`guid`)
) ENGINE=InnoDB AUTO_INCREMENT=163 DEFAULT CHARSET=utf8;

/*Table structure for table `characters` */

DROP TABLE IF EXISTS `characters`;

CREATE TABLE `characters` (
  `guid` int(10) NOT NULL auto_increment,
  `accountid` int(6) default '0',
  `data` longtext,
  `name` varchar(20) default 'charname',
  `mapid` int(5) default '12001',
  `pos_x` int(5) default '800',
  `pos_y` int(5) default '1175',
  `online` smallint(6) default '0',
  PRIMARY KEY  (`guid`)
) ENGINE=InnoDB AUTO_INCREMENT=11 DEFAULT CHARSET=utf8;

/*Table structure for table `item_instance` */

DROP TABLE IF EXISTS `item_instance`;

CREATE TABLE `item_instance` (
  `guid` int(11) NOT NULL auto_increment,
  `owner_guid` int(11) default '0',
  `data` longtext,
  PRIMARY KEY  (`guid`)
) ENGINE=InnoDB AUTO_INCREMENT=279 DEFAULT CHARSET=utf8;

SET SQL_MODE=@OLD_SQL_MODE;