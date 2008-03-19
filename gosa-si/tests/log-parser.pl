#!/usr/bin/perl 
#===============================================================================
#
#         FILE:  log-parser.pl
#
#        USAGE:  ./log-parser.pl 
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
#      CREATED:  13.03.2008 14:51:03 CET
#     REVISION:  ---
#===============================================================================

use strict;
use warnings;
use Getopt::Long;

my $log_file = "/var/log/gosa-si-server.log"; 
my $within_log_session = 0;

my $session;
GetOptions("s|session=s" => \$session);

if( not defined $session) { die "\tplease define a session to parse\n\ti.e. 'perl log-parser.pl -s 4'\n" }; 

print "session: $session\n";

open(FILE, "<$log_file") or die "\t can not open log-file"; 
    
# Read lines
my $line;
while ($line = <FILE>){

    chomp($line);
    my @line_list = split(" ", $line);

	if (not $line_list[4]) {
		if ($within_log_session) {
			print "$line\n";
		}
		next;
	}

    if($line_list[4] eq $session) {
        print "$line\n"; 
		$within_log_session = 1; 
		
    } else {
		$within_log_session = 0;
	}
}
