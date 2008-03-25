package logHandling;
use Exporter;
@ISA = qw(Exporter);
my @events = (
    "get_events",
#    "show_log_by_mac",
#    "show_log_by_date",
#    "show_log_by_date_and_mac",
#    "get_log_by_date",
#    "get_log_by_mac",
#    "get_log_by_date_and_mac",
#    "get_recent_log_by_mac",
#    "delete_log_by_date_and_mac",
    );
@EXPORT = @events;

use strict;
use warnings;
use GOSA::GosaSupportDaemon;
use Data::Dumper;
use File::Spec;

BEGIN {}

END {}

### Start ######################################################################

#&main::read_configfile($main::cfg_file, %cfg_defaults);

sub get_events {
    return \@events
}

sub show_log_by_date {
    my ($msg, $msg_hash, $session_id) = @_;
    my $header = @{$msg_hash->{header}}[0];
    $header =~ s/gosa_//;
    my $target = @{$msg_hash->{target}}[0];
    my $source = @{$msg_hash->{source}}[0];

    my $date_l =  $msg_hash->{date};

    if (not -d $main::client_fai_log_dir) {
        &main::daemon_log("$session_id ERROR: client fai log directory '$main::client_fai_log_dir' do not exist", 1); 
        return;
    }

    # build out_msg
    my $out_hash = &create_xml_hash($header, $target, $source);
    
    # fetch mac directory
    opendir(DIR, $main::client_fai_log_dir); 
    my @avail_macs = readdir(DIR);
    closedir(DIR);   
   
}

sub show_log_by_mac {
    my ($msg, $msg_hash, $session_id) = @_;
    my $header = @{$msg_hash->{header}}[0];
    $header =~ s/gosa_//;
    my $target = @{$msg_hash->{target}}[0];
    my $source = @{$msg_hash->{source}}[0];
    my $mac_l = $msg_hash->{mac};

    if (not -d $main::client_fai_log_dir) {
        &main::daemon_log("$session_id ERROR: client fai log directory '$main::client_fai_log_dir' do not exist", 1); 
        return;
    }

    # build out_msg
    my $out_hash = &create_xml_hash($header, $target, $source);
    foreach my $mac (@{$mac_l}) {
        my $act_log_dir = File::Spec->catdir($main::client_fai_log_dir, $mac);
        if (not -d $act_log_dir) { next; }
        $mac =~ s/:/_/g;

        # fetch mac directory
        opendir(DIR, $act_log_dir); 
        my @install_dates = readdir(DIR);
        closedir(DIR);   
        foreach my $date (@install_dates) {
            if ($date eq ".." || $date eq ".") { next; }
            &add_content2xml_hash($out_hash, "mac_$mac", $date);
        }
    }

    my $out_msg = &create_xml_string($out_hash);
    return ($out_msg);
}


sub show_log_by_date_and_mac {
    my ($msg, $msg_hash, $session_id) = @_ ;
    my $date_l = $msg_hash->{date};
    my $mac_l = $msg_hash->{mac};

    print STDERR "###########################################################\n"; 
    print STDERR "date:".Dumper($date_l); 
    print STDERR "mac: ".Dumper($mac_l); 
    print STDERR "client_fai_log_dir: $main::client_fai_log_dir\n"; 

    if (not -d $main::client_fai_log_dir) {
        &main::daemon_log("$session_id ERROR: client fai log directory '$main::client_fai_log_dir' do not exist", 1); 
        return;
    }

    # fetch mac directory
    opendir(DIR, $main::client_fai_log_dir); 
    my @avail_macs = readdir(DIR);
    closedir(DIR);   
    
    foreach my $avail_mac (@avail_macs) {
        if (not $avail_mac =~ /^([0-9a-f]{2}:[0-9a-f]{2}:[0-9a-f]{2}:[0-9a-f]{2}:[0-9a-f]{2}:[0-9a-f]{2})$/i ) { 
            next; 
        }

        print STDERR "mac: $avail_mac\n"; 
        # fetch date directory
        my $mac_log_dir = File::Spec->catdir($main::client_fai_log_dir, $avail_mac);
        opendir(DIR, $mac_log_dir); 
        my @avail_dates = readdir(DIR);
        closedir(DIR);
        #print STDERR "\@avail_dates:".Dumper(@avail_dates); 
    }

    return;
}


sub get_log_by_date {}
sub get_log_by_mac {}
sub get_log_by_date_and_mac {}
sub fetch_log {}

sub get_recent_log_by_mac {

}


sub delete_log_by_date_and_mac {

}

1;
