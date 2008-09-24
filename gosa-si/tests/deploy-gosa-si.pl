#!/usr/bin/perl 
#===============================================================================
#
#         FILE:  deploy-gosa-si.pl
#
#        USAGE:  ./deploy-gosa-si.pl 
#
#  DESCRIPTION:  
#
#      OPTIONS:  ---
# REQUIREMENTS:  ---
#         BUGS:  ---
#        NOTES:  ---
#       AUTHOR:   (), <>
#      COMPANY:  
#      VERSION:  1.0
#      CREATED:  22.04.2008 11:28:43 CEST
#     REVISION:  ---
#===============================================================================

use strict;
use warnings;

my $gosa_path = "/home/rettenbe/gonicus/Projekte/gosa/trunk/gosa-si/"; 

my %copies = (
        "gosa-si-server"=> "/usr/sbin/gosa-si-server", 
        "gosa-si-client"=> "/usr/sbin/gosa-si-client",

        "modules/GosaPackages.pm" => "/usr/lib/gosa-si/modules/GosaPackages.pm",
        "modules/ClientPackages.pm" => "/usr/lib/gosa-si/modules/ClientPackages.pm",
        "modules/ServerPackages.pm" => "/usr/lib/gosa-si/modules/ServerPackages.pm",
        "modules/ArpHandler.pm" => "/usr/lib/gosa-si/modules/ArpHandler.pm",

        "modules/DBsqlite.pm" => "/usr/share/perl5/GOSA/DBsqlite.pm",
        "modules/DBmysql.pm" => "/usr/share/perl5/GOSA/DBmysql.pm",
        "modules/GosaSupportDaemon.pm" => "/usr/share/perl5/GOSA/GosaSupportDaemon.pm",

        "server/events/clMessages.pm" => "/usr/lib/gosa-si/server/ClientPackages/clMessages.pm",
        "server/events/siTriggered" => "/usr/lib/gosa-si/server/ClientPackages/siTriggered.pm",

        "server/events/databases.pm" => "/usr/lib/gosa-si/server/GosaPackages/databases.pm",
        "server/events/gosaTriggered.pm" => "/usr/lib/gosa-si/server/GosaPackages/gosaTriggered.pm",
        "server/events/logHandling.pm" => "/usr/lib/gosa-si/server/GosaPackages/logHandling.pm",
        "server/events/mailqueue_com.pm" => "/usr/lib/gosa-si/server/GosaPackages/mailqueue_com.pm",
        "server/events/opsi_com.pm" => "/usr/lib/gosa-si/server/GosaPackages/opsi_com.pm",

        "server/events/opsi_com.pm" => "/usr/lib/gosa-si/server/ServerPackages/opsi_com.pm",
        "server/events/databases.pm" => "/usr/lib/gosa-si/server/ServerPackages/databases.pm",
        "server/events/gosaTriggered.pm" => "/usr/lib/gosa-si/server/ServerPackages/gosaTriggered.pm",
        "server/events/logHandling.pm" => "/usr/lib/gosa-si/server/ServerPackages/logHandling.pm",
        "server/events/mailqueue_com.pm" => "/usr/lib/gosa-si/server/ServerPackages/mailqueue_com.pm",
        "server/events/server_server_com.pm" => "/usr/lib/gosa-si/server/ServerPackages/server_server_com.pm",

        "client/events/corefunctions.pm" => "/usr/lib/gosa-si/client/events/corefunctions.pm",
        "client/events/dak.pm" => "/usr/lib/gosa-si/client/events/dak.pm",
        "client/events/gosaTriggered.pm" => "/usr/lib/gosa-si/client/events/gosaTriggered.pm",
        "client/events/installation.pm" => "/usr/lib/gosa-si/client/events/installation.pm",
        "client/events/mailqueue.pm" => "/usr/lib/gosa-si/client/events/mailqueue.pm",
);

while( my($file_name, $new_file) = each %copies ) {
    #print STDERR "copy ../$file_name to $new_file\n"; 
    #system("cp ../$file_name $new_file"); 
    
    my $del_cmd = "rm -rf $new_file"; 
    print STDERR "$del_cmd\n";
    system($del_cmd);
    
    my $ln_cmd = "ln -s ".$gosa_path.$file_name." $new_file"; 
    print STDERR "$ln_cmd\n"; 
    system($ln_cmd);
    
    print STDERR "\n"; 
}


