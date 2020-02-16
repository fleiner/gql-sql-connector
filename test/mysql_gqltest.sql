-- MySQL dump 10.16  Distrib 10.1.26-MariaDB, for debian-linux-gnu (x86_64)
--
-- Host: localhost    Database: gqltest
-- ------------------------------------------------------
-- Server version	10.1.26-MariaDB-0+deb9u1

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `Counter`
--

DROP TABLE IF EXISTS `Counter`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `Counter` (
  `Counter` int(11) NOT NULL,
  PRIMARY KEY (`Counter`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_bin;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `Counter`
--

LOCK TABLES `Counter` WRITE;
/*!40000 ALTER TABLE `Counter` DISABLE KEYS */;
INSERT INTO `Counter` VALUES (101);
/*!40000 ALTER TABLE `Counter` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `GR`
--

DROP TABLE IF EXISTS `GR`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `GR` (
  `last` varchar(30) COLLATE utf8_bin NOT NULL,
  `first` varchar(30) COLLATE utf8_bin NOT NULL,
  `own` int(11) NOT NULL,
  `home` int(11) NOT NULL,
  PRIMARY KEY (`last`,`first`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_bin;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `GR`
--

LOCK TABLES `GR` WRITE;
/*!40000 ALTER TABLE `GR` DISABLE KEYS */;
INSERT INTO `GR` VALUES ('Bear','Hello',12,4),('Bear','do',10,10),('Bear','how',44,13),('Bear','there',15,3),('Bee','black',22,14),('Bee','yellow',112,31);
/*!40000 ALTER TABLE `GR` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `T B`
--

DROP TABLE IF EXISTS `T B`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `T B` (
  `Currency` float NOT NULL,
  `Date` datetime DEFAULT NULL,
  `String` varchar(40) COLLATE utf8_bin NOT NULL,
  `Floats` double NOT NULL,
  `Ints` int(11) NOT NULL,
  `Percents` float NOT NULL,
  `number` int(1) DEFAULT NULL,
  PRIMARY KEY (`Ints`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_bin;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `T B`
--

LOCK TABLES `T B` WRITE;
/*!40000 ALTER TABLE `T B` DISABLE KEYS */;
INSERT INTO `T B` VALUES (1,NULL,'',1e21,1,0,NULL),(2,'2017-02-28 23:59:13','space  space',0.00000000012,2,0.4,0),(2.33,'2017-01-12 08:00:00','Hütätä',2.34567,3,0.3,NULL),(4.55,'2017-02-10 00:00:00','hallo',1.234,4,0.2,0);
/*!40000 ALTER TABLE `T B` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `T1`
--

DROP TABLE IF EXISTS `T1`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `T1` (
  `EINS` int(11) NOT NULL,
  `ZWEI` int(11) NOT NULL,
  `DREI UND` int(11) NOT NULL,
  `VIER` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_bin;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `T1`
--

LOCK TABLES `T1` WRITE;
/*!40000 ALTER TABLE `T1` DISABLE KEYS */;
INSERT INTO `T1` VALUES (2,32,1,34),(3,11,1,12),(5,4,31,23),(7,24,23,3);
/*!40000 ALTER TABLE `T1` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `alltypes`
--

DROP TABLE IF EXISTS `alltypes`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `alltypes` (
  `varchar` varchar(20) COLLATE utf8_bin DEFAULT 'hello',
  `tinyint` tinyint(4) NOT NULL DEFAULT '120',
  `text` text COLLATE utf8_bin NOT NULL,
  `date` date DEFAULT '1980-01-01',
  `smallint` smallint(6) NOT NULL DEFAULT '30000',
  `mediumint` mediumint(9) unsigned NOT NULL DEFAULT '8000000',
  `int` int(11) unsigned NOT NULL DEFAULT '2000000000',
  `bigint` bigint(20) NOT NULL DEFAULT '9000000000000000000',
  `ubigint` bigint(20) unsigned NOT NULL,
  `float` float NOT NULL DEFAULT '1.125',
  `double` double NOT NULL DEFAULT '2.375',
  `decimal` decimal(9,4) NOT NULL DEFAULT '12345.6789',
  `datetime` datetime(3) DEFAULT '1980-01-01 13:14:15.456',
  `timestamp` timestamp NULL DEFAULT '1981-01-02 09:10:11',
  `time` time(3) NOT NULL DEFAULT '04:05:06.789',
  `year` year(4) NOT NULL DEFAULT '1983',
  `char` char(10) COLLATE utf8_bin NOT NULL DEFAULT 'blah',
  `tinyblob` tinyblob NOT NULL,
  `tinytext` tinytext COLLATE utf8_bin NOT NULL,
  `blob` blob NOT NULL,
  `mediumblob` mediumblob NOT NULL,
  `mediumtext` mediumtext COLLATE utf8_bin NOT NULL,
  `longblob` longblob NOT NULL,
  `longtext` longtext COLLATE utf8_bin NOT NULL,
  `enum` enum('1','2','3') COLLATE utf8_bin NOT NULL DEFAULT '2',
  `set` set('1','2','3') COLLATE utf8_bin NOT NULL DEFAULT '2,3',
  `bool` tinyint(1) NOT NULL DEFAULT '1',
  `binary` binary(20) NOT NULL DEFAULT '0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0',
  `varbinary` varbinary(20) NOT NULL DEFAULT '0',
  `t1key` int(11) NOT NULL,
  `datetimesshort` datetime NOT NULL,
  `timeshort` time NOT NULL,
  PRIMARY KEY (`int`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_bin;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `alltypes`
--

LOCK TABLES `alltypes` WRITE;
/*!40000 ALTER TABLE `alltypes` DISABLE KEYS */;
INSERT INTO `alltypes` VALUES ('',0,'','1234-01-01',0,0,0,0,0,12.25,123.125,0.0000,'0000-00-00 00:00:00.000','2022-12-31 00:00:00','00:00:00.000',2155,'','','','','','','','','2','2,3',0,'\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0','',2,'1967-11-30 11:01:02','01:02:03'),('hello',120,'<&small>','1980-12-31',30000,8000000,123123123,9000000000000000000,9000000000000000000,-1.17549e-38,-2.2250738585072014e-308,6789.0000,'2980-12-31 23:59:59.999','1981-01-02 09:10:11','04:05:06.789',0000,'blah','','','','','','','','2','2,3',1,'0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0','0',5,'0000-00-00 00:00:00','06:07:08'),('hallo äöü अतुल',127,'bla','0000-00-00',32767,16777215,2000000000,-9223372036854775808,18446744073709551615,3.40282e38,1.7976931348623157e308,12345.1235,'1940-01-01 13:14:15.456','1970-01-01 00:00:01','04:05:06.089',1983,'blah','','tttt','','','medium','','and very long','2','2,3',1,'0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0','0',3,'1900-01-31 00:00:00','02:04:05'),('01234567890123456789',-128,'something\n\"long\"\nand with\nUTF 8\näöü','2980-01-01',-32768,0,4294967295,9223372036854775807,9223372036854775807,1.17549e-38,2.2250738585072014e-308,-99999.9999,'1080-01-01 13:14:15.456','0000-00-00 00:00:00','23:59:59.999',1901,'abcdefghij','','asdadfasdfasdfasdfasdfasdf','','','asdadfasdfasdfasdfasdfasdfasdadfasdfasdfasdfasdfasdfasdadfasdfasdfasdfasdfasdfasdadfasdfasdfasdfasdfasdf','','asdadfasdfasdfasdfasdfasdfasdadfasdfasdfasdfasdfasdfasdadfasdfasdfasdfasdfasdfasdadfasdfasdfasdfasdfasdfasdadfasdfasdfasdfasdfasdfasdadfasdfasdfasdfasdfasdfasdadfasdfasdfasdfasdfasdfasdadfasdfasdfasdfasdfasdfasdadfasdfasdfasdfasdfasdfasdadfasdfasdfasdfasdfasdfasdadfasdfasdfasdfasdfasdfasdadfasdfasdfasdfasdfasdfasdadfasdfasdfasdfasdfasdfasdadfasdfasdfasdfasdfasdfasdadfasdfasdfasdfasdfasdfasdadfasdfasdfasdfasdfasdfasdadfasdfasdfasdfasdfasdfasdadfasdfasdfasdfasdfasdfasdadfasdfasdfasdfasdfasdfasdadfasdfasdfasdfasdfasdfasdadfasdfasdfasdfasdfasdfasdadfasdfasdfasdfasdfasdfasdadfasdfasdfasdfasdfasdfasdadfasdfasdfasdfasdfasdfasdadfasdfasdfasdfasdfasdfasdadfasdfasdfasdfasdfasdfasdadfasdfasdfasdfasdfasdf','2','2,3',1,'0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0','0',7,'0000-00-00 00:00:00','10:11:12');
/*!40000 ALTER TABLE `alltypes` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `pv`
--

DROP TABLE IF EXISTS `pv`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `pv` (
  `name` varchar(30) COLLATE utf8_bin NOT NULL,
  `dept` varchar(30) COLLATE utf8_bin NOT NULL,
  `lunchtime` time NOT NULL,
  `salary` decimal(5,0) NOT NULL,
  `hireDate` date NOT NULL,
  `age` int(11) NOT NULL,
  `isSenior` tinyint(1) NOT NULL,
  `seniorityStartTime` datetime DEFAULT NULL,
  PRIMARY KEY (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_bin;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `pv`
--

LOCK TABLES `pv` WRITE;
/*!40000 ALTER TABLE `pv` DISABLE KEYS */;
INSERT INTO `pv` VALUES ('Ben','Sales','12:00:00',400,'2002-10-10',32,1,'2005-03-09 12:30:00'),('Dana','Sales','12:00:00',350,'2004-09-08',25,0,'0000-00-00 00:00:00'),('Dave','Eng','12:00:00',500,'2006-04-19',27,0,'0000-00-00 00:00:00'),('John','Eng','12:00:00',1000,'2005-03-19',35,1,'2007-12-02 15:56:00'),('Mike','Marketing','13:00:00',800,'2005-01-10',24,1,'2007-12-30 14:40:00'),('Sally','Eng','13:00:00',600,'2005-10-10',30,0,'0000-00-00 00:00:00');
/*!40000 ALTER TABLE `pv` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2018-07-08 21:29:31
