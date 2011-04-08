--
-- Table structure for table `$$PREFIX$$mailinglists`
--

DROP TABLE IF EXISTS `$$PREFIX$$mailinglists`;
CREATE TABLE IF NOT EXISTS `$$PREFIX$$mailinglists` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(64) NOT NULL,
  `desc` text NOT NULL,
  `address` text NOT NULL,
  `admin` text NOT NULL,
  `force_from` text,
  `footer` text,
  `created` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `public` enum('no','yes') NOT NULL DEFAULT 'no',
  `subs` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'subscriber count',
  PRIMARY KEY (`id`),
  KEY `name` (`name`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 AUTO_INCREMENT=0 ;

--
-- Table structure for table `$$PREFIX$$mailinglists_allow`
--

DROP TABLE IF EXISTS `$$PREFIX$$mailinglists_allow`;
CREATE TABLE IF NOT EXISTS `$$PREFIX$$mailinglists_allow` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `list_id` bigint(20) unsigned NOT NULL,
  `address` varchar(255) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `list_id` (`list_id`,`address`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 AUTO_INCREMENT=0 ;

--
-- Table structure for table `$$PREFIX$$mailinglists_subscribers`
--

DROP TABLE IF EXISTS `$$PREFIX$$mailinglists_subscribers`;
CREATE TABLE IF NOT EXISTS `$$PREFIX$$mailinglists_subscribers` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `list_id` bigint(20) unsigned NOT NULL,
  `address` varchar(255) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `list_id` (`list_id`,`address`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 AUTO_INCREMENT=0 ;

-- 
-- Table structure for table `$$PREFIX$$mail_queue`
-- 

DROP TABLE IF EXISTS `$$PREFIX$$mail_queue`;
CREATE TABLE IF NOT EXISTS `$$PREFIX$$mail_queue` (
  `id` tinyint(3) unsigned NOT NULL auto_increment,
  `type` enum('normal','mlist') NOT NULL default 'normal',
  `from` text NOT NULL,
  `subject` varchar(255) NOT NULL,
  `body` text,
  `footer` text,
  `list_name` varchar(255) default NULL,
  `list_address` text,
  `created` timestamp NOT NULL default CURRENT_TIMESTAMP,
  `status` enum('on_hold','waiting','processing', 'failed') NOT NULL default 'on_hold',
  PRIMARY KEY  (`id`),
  KEY `type` (`type`),
  KEY `status` (`status`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 AUTO_INCREMENT=0 ;


--
-- Table structure for table `$$PREFIX$$mail_queue_data`
--

DROP TABLE IF EXISTS `$$PREFIX$$mail_queue_data`;
CREATE TABLE IF NOT EXISTS `$$PREFIX$$mail_queue_data` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `queue_id` bigint(20) unsigned NOT NULL COMMENT 'points to queue row',
  `header_type` enum('to','cc','bcc') NOT NULL DEFAULT 'to',
  `email` varchar(255) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 AUTO_INCREMENT=0 ;

-- --------------------------------------------
-- MAILINGLISTS after delete queue
-- ----------------------------------------------

create trigger $$PREFIX$$after_delete_mail_queue
after delete on `$$PREFIX$$mail_queue`
for each row begin
    delete from `$$PREFIX$$mail_queue_data` where `queue_id`=`old`.`id`;
end$$

