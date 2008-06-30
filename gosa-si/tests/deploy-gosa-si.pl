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

my %copies = (
        "gosa-si-server"=> "/usr/sbin/gosa-si-server", 
        "modules/GosaPackages.pm" => "/usr/lib/gosa-si/modules/GosaPackages.pm",
        "modules/SIPackages.pm" => "/usr/lib/gosa-si/modules/SIPackages.pm",
        "modules/DBsqlite.pm" => "/usr/share/perl5/GOSA/DBsqlite.pm",
        "modules/GosaSupportDaemon.pm" => "/usr/share/perl5/GOSA/GosaSupportDaemon.pm",
        "server/events/clMessages.pm" => "/usr/lib/gosa-si/server/events/clMessages.pm",
        "server/events/databases.pm" => "/usr/lib/gosa-si/server/events/databases.pm",
        "server/events/gosaTriggered.pm" => "/usr/lib/gosa-si/server/events/gosaTriggered.pm",
        "server/events/siTriggered.pm" => "/usr/lib/gosa-si/server/events/siTriggered.pm",
);

while( my($file_name, $new_file) = each %copies ) {
    print STDERR "copy ../$file_name to $new_file\n"; 
    system("cp ../$file_name $new_file"); 
}


