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
my $within_session = 0;
my $within_incoming = 0;
my $session;
my $incoming;

sub check_incoming {
	my ($line) = @_ ;
	my @line_list = split(" ", $line);

	# new incoming msg, set all values back to default
	if ($line =~ /INFO: Incoming msg with session ID \d+ from/ ) {
		$within_incoming = 0;
	} 

	if ($line =~ /INFO: Incoming msg with session ID \d+ from '$incoming'/) {
		$within_incoming = 1;
		return $line;
	} else {
		if ($within_incoming) { 
			return $line;
		} else {
			return;
		}
	}
}

sub check_session {
	my ($line) = @_ ;
	my @line_list = split(" ", $line);
	
	if (not $line_list[4]) {
		if ($within_session) {
			return $line;
		}
		return;
	}

	if($line_list[4] eq $session) {
		$within_session = 1;
		return $line;
	} else {
		$within_session = 0;
	}
	return;
}

### MAIN ######################################################################

GetOptions(
		"s|session=s" => \$session,
		"i|incoming=s" => \$incoming,
		);

# check script pram
my $check_script_pram = 0;
if (defined $session) { 
	print "session: $session\n";
	$check_script_pram++;
}
if (defined $incoming) { 
	print "incoming msg for mac: $incoming\n";
	$check_script_pram++;
} 

if ($check_script_pram == 0) {
	# print usage and die
	print "exiting script\n"; 
	exit(0);
}

open(FILE, "<$log_file") or die "\t can not open log-file"; 
# Read lines
my $line;
while ($line = <FILE>){
    chomp($line);
	my $line2print;
	if (defined $session && (not defined $incoming)) {
		$line2print = &check_session($line);
		
	} elsif (defined $incoming && (not defined $session)) {
		$line2print = &check_incoming($line);
		
	} elsif ((defined $incoming) && (defined $session)) {
		my $line1 = &check_session($line);
		my $line2 = &check_incoming($line);
		if ((defined $line1) && (defined $line2)) {
			$line2print = $line;
		}
	}

	# printing
	if (defined $line2print) {
		print "$line\n";
	}
	
}



