
=head1 NAME

mailqueue.pm

=head1 SYNOPSIS

use GOSA::GosaSupportDaemon;

=head1 DESCRIPTION

This module contains all GOsa-SI-client processing instructions concerning the mailqueue in GOsa.

=head1 VERSION

Version 1.0

=head1 AUTHOR

Andreas Rettenberger <rettenberger at gonicus dot de>

=head1 FUNCTIONS

=cut


package mailqueue;
use Exporter;
@ISA = qw(Exporter);
my @events = (
    "get_events",
    "mailqueue_query",
    "mailqueue_hold",
    "mailqueue_unhold",
    "mailqueue_requeue",
    "mailqueue_del",
    "mailqueue_header",
    );
@EXPORT = @events;

use strict;
use warnings;
use GOSA::GosaSupportDaemon;
use Data::Dumper;

BEGIN {}

END {}


###############################################################################
=over 

=item B<get_events ()>

=over

=item description 

    Reports all provided functions.

=item parameter

    None.

=item return 

    \@events - ARRAYREF - array containing all functions 

=back

=back

=cut
###############################################################################
sub get_events { return \@events; }


###############################################################################
=over 

=item B<mailqueue_query ($$)>

=over

=item description 

    Executes /usr/sbin/mailq, parse the informations and return them

=item parameter

    $msg - STRING - complete GOsa-si message
    $msg_hash - HASHREF - content of GOsa-si message in a hash

=item GOsa-si message xml content

    None.

=item return 

    $out_msg - STRING - GOsa-SI valid xml message containing msg_id, msg_hold, msg_size, arrival_time, sender and recipient.

=back

=back

=cut
###############################################################################
sub mailqueue_query {
    my ($msg, $msg_hash) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $session_id = @{$msg_hash->{'session_id'}}[0];
    my $error = 0;
    my $error_string;
    my $msg_id;
    my $msg_hold;
    my $msg_size;
    my $arrival_time;
    my $sender;
    my $recipient;
    my $out_hash;
    my $out_msg;

	&main::daemon_log("DEBUG: run /usr/bin/mailq\n", 7); 
    my $result = qx("/usr/bin/mailq");
    my @result_l = split(/([0-9A-Z]{10,12})/, $result);

    if (length($result) == 0) {
        $error = 1;
        $error_string = "/usr/bin/mailq has no result";
        &main::daemon_log("ERROR: $error_string : $msg", 1);
    }

    my $result_collection = {};
    if (not $error) {
        # parse information
        my $result_length = @result_l;
        my $j = 0;
        for (my $i = 1; $i < $result_length; $i+=2) {
            $j++;
            $result_collection->{$j} = {};

            $msg_id = $result_l[$i];
            $result_collection->{$j}->{'msg_id'} = $msg_id;
            $result_l[$i+1] =~ /^([\!| ])\s+(\d+) (\w{3} \w{3} \d+ \d+:\d+:\d+)\s+([\w.-]+@[\w.-]+)\s+/ ;
            $result_collection->{$j}->{'msg_hold'} =  $1 eq "!" ? 1 : 0 ;
            $result_collection->{$j}->{'msg_size'} = $2;
            $result_collection->{$j}->{'arrival_time'} = $3;
            $result_collection->{$j}->{'sender'} = $4;
            my @info_l = split(/\n/, $result_l[$i+1]);
            $result_collection->{$j}->{'recipient'} = $info_l[2] =~ /([\w.-]+@[\w.-]+)/ ? $1 : 'unknown' ;
        }
    }    
    
    # create outgoing msg
    $out_hash = &main::create_xml_hash("answer_$session_id", $target, $source);
    &add_content2xml_hash($out_hash, "session_id", $session_id);
    &add_content2xml_hash($out_hash, "error", $error);
    if (defined @{$msg_hash->{'forward_to_gosa'}}[0]){
        &add_content2xml_hash($out_hash, "forward_to_gosa", @{$msg_hash->{'forward_to_gosa'}}[0]);
    }

    # add error infos to outgoing msg
    if ($error) {
        &add_content2xml_hash($out_hash, "error_string", $error_string);
        $out_msg = &main::create_xml_string($out_hash);

    # add mail infos to outgoing msg
    } else {
        my $collection_string = &db_res2xml($result_collection);
        $out_msg = &main::create_xml_string($out_hash);
        $out_msg =~ s/<\/xml>/$collection_string<\/xml>/
    }
    
    return $out_msg;

}


###############################################################################
=over 

=item B<mailqueue_hold ($$)>

=over

=item description 

    Executes '/usr/sbin/postsuper -h' and set mail to hold. 

=item parameter

    $msg - STRING - complete GOsa-si message
    $msg_hash - HASHREF - content of GOsa-si message in a hash

=item GOsa-si message xml content

    <msg_id> - STRING - postfix mail id

=item return 

    Nothing.

=back

=back

=cut
###############################################################################
sub mailqueue_hold {
    my ($msg, $msg_hash) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $session_id = @{$msg_hash->{'session_id'}}[0];
    my $error = 0;
    my $error_string;

    # sanity check of input
    if (not exists $msg_hash->{'msg_id'}) {
        $error_string = "Message doesn't contain a XML tag 'msg_id"; 
        &main::daemon_log("ERROR: $error_string : $msg", 1);
        $error = 1;
    } elsif (ref @{$msg_hash->{'msg_id'}}[0] eq "HASH") { 
        $error_string = "XML tag 'msg_id' is empty";
        &main::daemon_log("ERROR: $error_string : $msg", 1);
        $error = 1;
    }

    if (not $error) {
        my @msg_ids = @{$msg_hash->{'msg_id'}};
        foreach my $msg_id (@msg_ids) {
            my $error = 0;   # clear error status

            # sanity check of each msg_id
            if (not $msg_id =~ /^[0-9A-Z]{10}$/) {
                $error = 1;
                $error_string = "message ID is not valid ([0-9A-Z]{10}) : $msg_id";
                &main::daemon_log("ERROR: $error_string : $msg", 1);
            }

            if (not $error) {
                my $cmd = "/usr/sbin/postsuper -h $msg_id 2>&1";
                &main::daemon_log("DEBUG: run $cmd", 7); 
                my $result = qx($cmd);
                if ($result =~ /^postsuper: ([0-9A-Z]{10}): placed on hold/ ) {
                    &main::daemon_log("INFO: Mail $msg_id placed on hold", 5);
                } elsif ($result eq "") {
                    &main::daemon_log("INFO: Mail $msg_id is alread placed on hold", 5);
                
                } else {
                    &main::daemon_log("ERROR: '$cmd' failed : $result", 1); 
                }
            }
        }
    }

    return;
}

###############################################################################
=over 

=item B<mailqueue_unhold ($$)>

=over

=item description 

    Executes '/usr/sbin/postsuper -H' and set mail to unhold. 

=item parameter

    $msg - STRING - complete GOsa-si message
    $msg_hash - HASHREF - content of GOsa-si message in a hash

=item GOsa-si message xml content

    <msg_id> - STRING - postfix mail id

=item return 

Nothing.

=back

=back

=cut
###############################################################################
sub mailqueue_unhold {
    my ($msg, $msg_hash) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $session_id = @{$msg_hash->{'session_id'}}[0];
    my $error = 0;
    my $error_string;
    
    # sanity check of input
    if (not exists $msg_hash->{'msg_id'}) {
        $error_string = "Message doesn't contain a XML tag 'msg_id'"; 
        &main::daemon_log("ERROR: $error_string : $msg", 1);
        $error = 1;
    } elsif (ref @{$msg_hash->{'msg_id'}}[0] eq "HASH") { 
        $error_string = "XML tag 'msg_id' is empty";
        &main::daemon_log("ERROR: $error_string : $msg", 1);
        $error = 1;
    }
        
    if (not $error) {
        my @msg_ids = @{$msg_hash->{'msg_id'}};
        foreach my $msg_id (@msg_ids) {
            my $error = 0;   # clear error status

            # sanity check of each msg_id
            if (not $msg_id =~ /^[0-9A-Z]{10}$/) {
                $error = 1;
                $error_string = "message ID is not valid ([0-9A-Z]{10}) : $msg_id";
                &main::daemon_log("ERROR: $error_string : $msg", 1);
            }

            if (not $error) {
                my $cmd = "/usr/sbin/postsuper -H $msg_id 2>&1";
                &main::daemon_log("DEBUG: run $cmd\n", 7); 
                my $result = qx($cmd);
                if ($result =~ /^postsuper: ([0-9A-Z]{10}): released from hold/ ) {
                    &main::daemon_log("INFO: Mail $msg_id released from on hold", 5);
                } elsif ($result eq "") {
                    &main::daemon_log("INFO: Mail $msg_id is alread released from hold", 5);

                } else {
                    &main::daemon_log("ERROR: '$cmd' failed : $result", 1); 
                }

            }
        }
    }

    return;
}

###############################################################################
=over 

=item B<mailqueue_requeue ($$)>

=over

=item description 

    Executes '/usr/sbin/postsuper -r' and requeue the mail.

=item parameter

    $msg - STRING - complete GOsa-si message
    $msg_hash - HASHREF - content of GOsa-si message in a hash

=item GOsa-si message xml content

    <msg_id> - STRING - postfix mail id

=item return 

Nothing.

=back

=back

=cut
###############################################################################
sub mailqueue_requeue {
    my ($msg, $msg_hash) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $session_id = @{$msg_hash->{'session_id'}}[0];
    my @msg_ids = @{$msg_hash->{'msg_id'}};
    my $error = 0;
    my $error_string;  

    # sanity check of input
    if (not exists $msg_hash->{'msg_id'}) {
        $error_string = "Message doesn't contain a XML tag 'msg_id'"; 
        &main::daemon_log("ERROR: $error_string : $msg", 1);
        $error = 1;
    } elsif (ref @{$msg_hash->{'msg_id'}}[0] eq "HASH") { 
        $error_string = "XML tag 'msg_id' is empty";
        &main::daemon_log("ERROR: $error_string : $msg", 1);
        $error = 1;
    }
        
    if (not $error) {
        my @msg_ids = @{$msg_hash->{'msg_id'}};
        foreach my $msg_id (@msg_ids) {
            my $error = 0;   # clear error status

            # sanity check of each msg_id
            if (not $msg_id =~ /^[0-9A-Z]{10}$/) {
                $error = 1;
                $error_string = "message ID is not valid ([0-9A-Z]{10}) : $msg_id";
                &main::daemon_log("ERROR: $error_string : $msg", 1);
            }

            if (not $error) {
                my $cmd = "/usr/sbin/postsuper -r $msg_id 2>&1";
                &main::daemon_log("DEBUG: run '$cmd'", 7); 
                my $result = qx($cmd);
                if ($result =~ /^postsuper: ([0-9A-Z]{10}): requeued/ ) {
                    &main::daemon_log("INFO: Mail $msg_id requeued", 5);
                } elsif ($result eq "") {
                    &main::daemon_log("WARNING: Cannot requeue mail '$msg_id', mail not found!", 3);

                } else {
                    &main::daemon_log("ERROR: '$cmd' failed : $result", 1); 
                }

            }
        }
    }

    return;
}


###############################################################################
=over 

=item B<mailqueue_del ($$)>

=over

=item description 

    Executes '/usr/sbin/postsuper -d' and deletes mail from queue.

=item parameter

    $msg - STRING - complete GOsa-si message
    $msg_hash - HASHREF - content of GOsa-si message in a hash

=item GOsa-si message xml content

    <msg_id> - STRING - postfix mail id

=item return 

Nothing.

=back

=back

=cut
###############################################################################
sub mailqueue_del {
    my ($msg, $msg_hash) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $session_id = @{$msg_hash->{'session_id'}}[0];
    my @msg_ids = @{$msg_hash->{'msg_id'}};
    my $error = 0;
    my $error_string;

    # sanity check of input
    if (not exists $msg_hash->{'msg_id'}) {
        $error_string = "Message doesn't contain a XML tag 'msg_id'"; 
        &main::daemon_log("ERROR: $error_string : $msg", 1);
        $error = 1;
    } elsif (ref @{$msg_hash->{'msg_id'}}[0] eq "HASH") { 
        $error_string = "XML tag 'msg_id' is empty";
        &main::daemon_log("ERROR: $error_string : $msg", 1);
        $error = 1;
    }
        
    if (not $error) {
        my @msg_ids = @{$msg_hash->{'msg_id'}};
        foreach my $msg_id (@msg_ids) {
            my $error = 0;   # clear error status

            # sanity check of each msg_id
            if (not $msg_id =~ /^[0-9A-Z]{10}$/) {
                $error = 1;
                $error_string = "message ID is not valid ([0-9A-Z]{10}) : $msg_id";
                &main::daemon_log("ERROR: $error_string : $msg", 1);
            }

            if (not $error) {
                my $cmd = "/usr/sbin/postsuper -d $msg_id 2>&1";
                &main::daemon_log("DEBUG: run '$cmd'", 7); 
                my $result = qx($cmd);
                if ($result =~ /^postsuper: ([0-9A-Z]{10}): removed/ ) {
                    &main::daemon_log("INFO: Mail $msg_id deleted", 5);
                } elsif ($result eq "") {
                    &main::daemon_log("WARNING: Cannot remove mail '$msg_id', mail not found!", 3);

                } else {
                    &main::daemon_log("ERROR: '$cmd' failed : $result", 1); 
                }

            }
        }
    }

    return;
}

###############################################################################
=over 

=item B<mailqueue_header ($$)>

=over

=item description 

    Executes 'postcat -q', parse the informations and return them. 

=item parameter

    $msg - STRING - complete GOsa-si message
    $msg_hash - HASHREF - content of GOsa-si message in a hash

=item GOsa-si message xml content

    <msg_id> - STRING - postfix mail id

=item return 

    $out_msg - STRING - GOsa-si valid xml message containing recipient, sender and subject.

=back

=back

=cut
###############################################################################
sub mailqueue_header {
    my ($msg, $msg_hash) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $session_id = @{$msg_hash->{'session_id'}}[0];
    my $error = 0;
    my $error_string;
    my $sender;
    my $recipient;
    my $subject;
    my $out_hash;
    my $out_msg;

    # sanity check of input
    if (not exists $msg_hash->{'msg_id'}) {
        $error_string = "Message doesn't contain a XML tag 'msg_id'"; 
        &main::daemon_log("ERROR: $error_string : $msg", 1);
        $error = 1;
    } elsif (ref @{$msg_hash->{'msg_id'}}[0] eq "HASH") { 
        $error_string = "XML tag 'msg_id' is empty";
        &main::daemon_log("ERROR: $error_string : $msg", 1);
        $error = 1;
    }

    # sanity check of each msg_id
    my $msg_id;
    if (not $error) {
        $msg_id = @{$msg_hash->{'msg_id'}}[0];
        if (not $msg_id =~ /^[0-9A-Z]{10}$/) {
            $error = 1;
            $error_string = "message ID is not valid ([0-9A-Z]{10}) : $msg_id";
            &main::daemon_log("ERROR: $error_string : $msg", 1);
        }
    }

    # parsing information
    if (not $error) {
        my $cmd = "postcat -q $msg_id";
        &main::daemon_log("DEBUG: run '$cmd'", 7); 
        my $result = qx($cmd);
        foreach (split(/\n/, $result)) {
            if ($_ =~ /^To: ([\S]+)/) { $recipient = $1; }
            if ($_ =~ /^From: ([\S]+)/) { $sender = $1; }
            if ($_ =~ /^Subject: ([\s\S]+)/) { $subject = $1; }
        }
    }       

    # create outgoing msg
    $out_hash = &main::create_xml_hash("answer_$session_id", $target, $source);
    &add_content2xml_hash($out_hash, "session_id", $session_id);
    &add_content2xml_hash($out_hash, "error", $error);
    if (defined @{$msg_hash->{'forward_to_gosa'}}[0]){
        &add_content2xml_hash($out_hash, "forward_to_gosa", @{$msg_hash->{'forward_to_gosa'}}[0]);
    }

    # add error infos to outgoing msg
    if ($error) {
        &add_content2xml_hash($out_hash, "error_string", $error_string);
        $out_msg = &main::create_xml_string($out_hash);

    # add mail infos to outgoing msg
    } else {
        &add_content2xml_hash($out_hash, "recipient", $recipient);        
        &add_content2xml_hash($out_hash, "sender", $sender);        
        &add_content2xml_hash($out_hash, "subject", $subject);        
        $out_msg = &main::create_xml_string($out_hash);
    }
 
    return $out_msg;
}


# vim:ts=4:shiftwidth:expandtab

1;
