package mailqueue_com;
use Exporter;
@ISA = qw(Exporter);
my @events = (
    "get_events",
    "mailqueue_query",
    "mailqueue_header",
);
@EXPORT = @events;

use strict;
use warnings;
use GOSA::GosaSupportDaemon;
use Data::Dumper;
use Time::HiRes qw( usleep);
use MIME::Base64;


BEGIN {}

END {}

### Start ######################################################################

sub get_events {
    return \@events;
}

sub mailqueue_query {
    my ($msg, $msg_hash, $session_id) = @_ ;
    my $header = @{$msg_hash->{header}}[0];
    my $target = @{$msg_hash->{target}}[0];
    my $source = @{$msg_hash->{source}}[0];
    my $jobdb_id = @{$msg_hash->{'jobdb_id'}}[0];
    my $error = 0;
    my $error_string;
    my $answer_msg;
    my ($sql, $res);

    if( defined $jobdb_id) {
        my $sql_statement = "UPDATE $main::job_queue_tn SET status='processed' WHERE id=jobdb_id";
        &main::daemon_log("$session_id DEBUG: $sql_statement", 7); 
        my $res = $main::job_db->exec_statement($sql_statement);
    }

    # send message
    $sql = "SELECT * FROM $main::known_clients_tn WHERE ((hostname='$target') || (macaddress LIKE '$target'))"; 
    $res = $main::known_clients_db->exec_statement($sql);

    # sanity check of db result
    my ($host_name, $host_key);
    if ((defined $res) && (@$res > 0) && @{@$res[0]} > 0) {
        $host_name = @{@$res[0]}[0];
        $host_key = @{@$res[0]}[2];
    } else {
        &main::daemon_log("$session_id ERROR: cannot determine host_name and host_key from known_clients_db\n$msg", 1);
        $error_string = "Cannot determine host_name and host_key from known_clients_db";
        $error = 1;
    }

    if (not $error) {
        $msg =~ s/<source>GOSA<\/source>/<source>$main::server_address<\/source>/g; 
        $msg =~ s/<\/xml>/<session_id>$session_id<\/session_id><\/xml>/;
        &main::send_msg_to_target($msg, $host_name, $host_key, $header, $session_id);

        # waiting for answer
        my $message_id;
        my $i = 0;
        while (1) {
            $i++;
            $sql = "SELECT * FROM $main::incoming_tn WHERE headertag='answer_$session_id'";
            $res = $main::incoming_db->exec_statement($sql);
            if (ref @$res[0] eq "ARRAY") { 
                $message_id = @{@$res[0]}[0];
                last;
            }

            if ($i > 100) { last; } # do not run into a endless loop
            usleep(100000);
        }
        # if answer exists
         if (defined $message_id) {
            $answer_msg = decode_base64(@{@$res[0]}[4]);
            $answer_msg =~ s/<target>\S+<\/target>/<target>$source<\/target>/;
            $answer_msg =~ s/<header>\S+<\/header>/<header>$header<\/header>/;

            my $forward_to_gosa = @{$msg_hash->{'forward_to_gosa'}}[0];
            if (defined $forward_to_gosa){
                $answer_msg =~s/<\/xml>/<forward_to_gosa>$forward_to_gosa<\/forward_to_gosa><\/xml>/;
            }
            $sql = "DELETE FROM $main::incoming_tn WHERE id=$message_id"; 
            $res = $main::incoming_db->exec_statement($sql);
        }
    }

    return ( $answer_msg );
}

sub mailqueue_header {
    my ($msg, $msg_hash, $session_id) = @_ ;
    my $header = @{$msg_hash->{header}}[0];
    my $target = @{$msg_hash->{target}}[0];
    my $source = @{$msg_hash->{source}}[0];
    my $jobdb_id = @{$msg_hash->{'jobdb_id'}}[0];
    my $error = 0;
    my $error_string;
    my $answer_msg;
    my ($sql, $res);

    if( defined $jobdb_id) {
        my $sql_statement = "UPDATE $main::job_queue_tn SET status='processed' WHERE id=jobdb_id";
        &main::daemon_log("$session_id DEBUG: $sql_statement", 7); 
        my $res = $main::job_db->exec_statement($sql_statement);
    }

    # search for the correct target address
    $sql = "SELECT * FROM $main::known_clients_tn WHERE ((hostname='$target') || (macaddress LIKE '$target'))"; 
    $res = $main::known_clients_db->exec_statement($sql);
    my ($host_name, $host_key);
    if ((defined $res) && (@$res > 0) && @{@$res[0]} > 0) {   # sanity check of db result
        $host_name = @{@$res[0]}[0];
        $host_key = @{@$res[0]}[2];
    } else {
        &main::daemon_log("$session_id ERROR: cannot determine host_name and host_key from known_clients_db\n$msg", 1);
        $error_string = "Cannot determine host_name and host_key from known_clients_db";
        $error = 1;
    }

    # send message to target
    if (not $error) {
        $msg =~ s/<source>GOSA<\/source>/<source>$main::server_address<\/source>/g; 
        $msg =~ s/<\/xml>/<session_id>$session_id<\/session_id><\/xml>/;
        &main::send_msg_to_target($msg, $host_name, $host_key, $header, $session_id);
    }

    # waiting for answer
    if (not $error) {
        my $message_id;
        my $i = 0;
        while (1) {
            $i++;
            $sql = "SELECT * FROM $main::incoming_tn WHERE headertag='answer_$session_id'";
            $res = $main::incoming_db->exec_statement($sql);
            if (ref @$res[0] eq "ARRAY") { 
                $message_id = @{@$res[0]}[0];
                last;
            }

            if ($i > 100) { last; } # do not run into a endless loop
            usleep(100000);
        }
        # if answer exists
        if (defined $message_id) {
            $answer_msg = decode_base64(@{@$res[0]}[4]);
            $answer_msg =~ s/<target>\S+<\/target>/<target>$source<\/target>/;
            $answer_msg =~ s/<header>\S+<\/header>/<header>$header<\/header>/;

            my $forward_to_gosa = @{$msg_hash->{'forward_to_gosa'}}[0];
            if (defined $forward_to_gosa){
                $answer_msg =~s/<\/xml>/<forward_to_gosa>$forward_to_gosa<\/forward_to_gosa><\/xml>/;
            }
            $sql = "DELETE FROM $main::incoming_tn WHERE id=$message_id"; 
            $res = $main::incoming_db->exec_statement($sql);
        }
    }

    return ( $answer_msg );
}

# vim:ts=4:shiftwidth:expandtab
1;
