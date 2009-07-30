/*
SQLyog Community Edition- MySQL GUI v5.20
Host - 5.0.51a-community-nt : Database - legacydb
*********************************************************************
Server version : 5.0.51a-community-nt
*/

SET NAMES utf8;

SET SQL_MODE='';

USE `legacydb`;
SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO';

/*Table structure for table `creature` */

DROP TABLE IF EXISTS `creature`;

CREATE TABLE `creature` (
  `guid` int(11) unsigned NOT NULL auto_increment COMMENT 'Global Unique Identifier',
  `entry` int(11) unsigned NOT NULL default '0' COMMENT 'Creature Identifier in Creature Template',
  `mapid` int(11) unsigned NOT NULL default '0' COMMENT 'Map Identifier',
  `map_npcid` int(11) NOT NULL default '0' COMMENT 'Map Npc Identifier',
  `pos_x` float NOT NULL default '0',
  `pos_y` float NOT NULL default '0',
  `spawntimesecs` int(11) unsigned NOT NULL default '120',
  `spawn_pos_x` float NOT NULL default '0',
  `spawn_pos_y` float NOT NULL default '0',
  `hp` int(11) unsigned NOT NULL default '1',
  `sp` int(11) unsigned NOT NULL default '0',
  `DeathState` tinyint(3) unsigned NOT NULL default '0',
  `MovementType` tinyint(3) unsigned NOT NULL default '0',
  `team_0_0` int(11) NOT NULL default '0',
  `team_0_1` int(11) NOT NULL default '0',
  `team_0_3` int(11) NOT NULL default '0',
  `team_0_4` int(11) NOT NULL default '0',
  `team_1_0` int(11) NOT NULL default '0',
  `team_1_1` int(11) NOT NULL default '0',
  `team_1_2` int(11) NOT NULL default '0',
  `team_1_3` int(11) NOT NULL default '0',
  `team_1_4` int(11) NOT NULL default '0',
  PRIMARY KEY  (`guid`),
  KEY `idx_map` (`mapid`),
  KEY `index_id` (`entry`)
) ENGINE=MyISAM AUTO_INCREMENT=12263004 DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Creature System';

/*Table structure for table `creature_template` */

DROP TABLE IF EXISTS `creature_template`;

CREATE TABLE `creature_template` (
  `entry` int(11) NOT NULL default '0',
  `modelid` int(11) default '0',
  `name` char(100) default '0',
  `hp` int(11) default '0',
  `sp` int(11) default '0',
  `int` int(11) default '0',
  `atk` int(11) default '0',
  `def` int(11) default '0',
  `hpx` int(11) default '0',
  `spx` int(11) default '0',
  `agi` int(11) default '0',
  `level` int(11) default '0',
  `element` int(11) default '0',
  `reborn` int(11) default '0',
  `skill1` int(11) default '0',
  `skill2` int(11) default '0',
  `skill3` int(11) default '0',
  `skill4` int(11) default '0',
  `skill5` int(11) default '0',
  `drop1` int(11) default '0',
  `drop2` int(11) default '0',
  `drop3` int(11) default '0',
  `drop4` int(11) default '0',
  `drop5` int(11) default '0',
  `drop6` int(11) default '0',
  `drop7` int(11) default '0',
  `drop8` int(11) default '0',
  `drop9` int(11) default '0',
  `drop10` int(11) default '0',
  `npcflag` int(11) default '0',
  `AIName` char(128) NOT NULL,
  `MovementType` tinyint(3) default '0',
  `ScriptName` char(128) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

/*Table structure for table `item_template` */

DROP TABLE IF EXISTS `item_template`;

CREATE TABLE `item_template` (
  `entry` int(11) NOT NULL default '0',
  `modelid` int(11) default '0',
  `name` varchar(20) default NULL,
  `type` varchar(20) default NULL,
  `InventoryType` int(11) default '0',
  `level` smallint(4) default '1',
  `attribute` varchar(255) default NULL,
  `slot` varchar(20) default NULL,
  `EquipmentSlot` int(11) default '0',
  `contribute` tinyint(4) default '1',
  `buyprice` int(11) default '2',
  `sellprice` int(11) default '1',
  `maxcount` smallint(5) default '0',
  `stackable` smallint(5) default '0',
  `desc` varchar(255) default NULL,
  `ContainerSlots` int(11) default '0',
  `stat_type1` tinyint(4) default '0',
  `stat_value1` smallint(6) default '0',
  `stat_type2` tinyint(4) default '0',
  `stat_value2` smallint(6) default '0',
  `stat_type3` tinyint(4) default '0',
  `stat_value3` smallint(6) default '0',
  `stat_type4` tinyint(4) default '0',
  `stat_value4` smallint(6) default '0',
  `stat_type5` tinyint(4) default '0',
  `stat_value5` smallint(6) default '0',
  `stat_type6` tinyint(4) default '0',
  `stat_value6` smallint(6) default '0',
  `stat_type7` tinyint(4) default '0',
  `stat_value7` smallint(6) default '0',
  `stat_type8` tinyint(4) default '0',
  `stat_value8` smallint(6) default '0',
  `stat_type9` tinyint(4) default '0',
  `stat_value9` smallint(6) default '0',
  `stat_type10` tinyint(4) default '0',
  `stat_value10` smallint(6) default '0',
  `socketColor_1` int(11) default '0',
  `socketContent_1` int(11) default '0',
  `socketColor_2` int(11) default '0',
  `socketContent_2` int(11) default '0',
  `socketColor_3` int(11) default '0',
  `socketContent_3` int(11) default '0',
  `GemProperties` int(11) default '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

/*Table structure for table `map_matrix` */

DROP TABLE IF EXISTS `map_matrix`;

CREATE TABLE `map_matrix` (
  `id` int(5) NOT NULL auto_increment,
  `mapid_src` int(5) default '0',
  `doorid` int(5) default '0',
  `mapid_dest` int(5) default '0',
  `x` int(5) default '0',
  `y` int(5) default '0',
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=4419 DEFAULT CHARSET=utf8;

/*Table structure for table `npc_gossip` */

DROP TABLE IF EXISTS `npc_gossip`;

CREATE TABLE `npc_gossip` (
  `id` int(11) NOT NULL auto_increment,
  `npc_guid` int(11) unsigned NOT NULL default '0',
  `quest` int(11) default '0',
  `sequence` int(11) default '0',
  `action` int(11) default '0',
  `gossip_type` int(11) unsigned NOT NULL default '0',
  `textid` int(11) unsigned NOT NULL default '0',
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=25 DEFAULT CHARSET=utf8;

/*Table structure for table `npc_option` */

DROP TABLE IF EXISTS `npc_option`;

CREATE TABLE `npc_option` (
  `id` int(11) unsigned NOT NULL default '0',
  `zoneid` int(11) unsigned NOT NULL default '0',
  `npcflag` int(11) unsigned NOT NULL default '0',
  `sequence` int(11) NOT NULL default '0',
  `action` int(11) unsigned NOT NULL default '0',
  `option_type` int(11) unsigned NOT NULL default '0',
  `option_text` int(11) unsigned NOT NULL default '0',
  `note` varchar(255) default NULL,
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

/*Table structure for table `npc_vendor` */

DROP TABLE IF EXISTS `npc_vendor`;

CREATE TABLE `npc_vendor` (
  `guid` int(11) unsigned NOT NULL default '0',
  `slot` int(11) default '0',
  `item` int(11) unsigned NOT NULL default '0',
  PRIMARY KEY  (`guid`,`item`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=FIXED COMMENT='Npc System';

/*Table structure for table `quest_template` */

DROP TABLE IF EXISTS `quest_template`;

CREATE TABLE `quest_template` (
  `id` int(11) NOT NULL auto_increment,
  `name` varchar(100) default NULL,
  `req_level` int(11) default '0',
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8;

/*Table structure for table `spell_template` */

DROP TABLE IF EXISTS `spell_template`;

CREATE TABLE `spell_template` (
  `entry` int(11) NOT NULL auto_increment,
  `name` varchar(20) default NULL,
  `sp` int(11) default '0',
  `element` int(11) default '0',
  `hit` int(11) default '0',
  `LearnPoint` int(11) default '0',
  `LevelMax` int(11) default '0',
  `type` int(11) default '0',
  `DamageMod` int(11) default '0',
  `reborn` int(11) default '0',
  `Core` float default '0.5',
  PRIMARY KEY  (`entry`)
) ENGINE=InnoDB AUTO_INCREMENT=21019 DEFAULT CHARSET=utf8;

SET SQL_MODE=@OLD_SQL_MODE;
