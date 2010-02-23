
=head1 NAME

gosaTriggered.pm

=head1 SYNOPSIS

use GOSA::GosaSupportDaemon;

use Data::Dumper;

use MIME::Base64

=head1 DESCRIPTION

This module contains all GOsa-SI-client processing instructions concerning actions controllable from GOsa.

=head1 VERSION

Version 1.0

=head1 AUTHOR

Andreas Rettenberger <rettenberger at gonicus dot de>

=head1 FUNCTIONS

=cut


package gosaTriggered;
use Exporter;
@ISA = qw(Exporter);
my @events = (
    "get_events",
    "usr_msg",
    "trigger_action_localboot",
    "trigger_action_halt",
    "trigger_action_faireboot",
    "trigger_action_reboot",
    "trigger_action_reinstall",
    "trigger_action_update",
    "trigger_action_instant_update",
    "trigger_goto_settings_reload",
    );
@EXPORT = @events;

use strict;
use warnings;
use GOSA::GosaSupportDaemon;
use Data::Dumper;
use MIME::Base64;
use File::Temp qw/ tempfile/;

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

=item B<usr_msg ($$)>

=over

=item description 

    Executes '/usr/bin/goto-notify' wich displays the message, subject und receiver at screen

=item parameter

    $msg - STRING - complete GOsa-si message
    $msg_hash - HASHREF - content of GOsa-si message in a hash

=item GOsa-si message xml content
    
    <to> - STRING - username message should be deliverd to
    <subject> - STRING - subject of the message, base64 encoded
    <message> - STRING - message itself, base64 encoded

=item return 

    $out_msg - STRING - GOsa-si valid xml message, feedback that message was deliverd

=back

=back

=cut
###############################################################################
sub usr_msg {
    my ($msg, $msg_hash) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];

    my $to = @{$msg_hash->{'usr'}}[0];
    my $subject = &decode_base64(@{$msg_hash->{'subject'}}[0]);
    my $message = &decode_base64(@{$msg_hash->{'message'}}[0]);

    my ($rand_fh, $rand_file) = tempfile( SUFFIX => '.goto_notify');
    print $rand_fh "source:$source\ntarget:$target\nusr:$to\nsubject:".@{$msg_hash->{'subject'}}[0]."\nmessage:".@{$msg_hash->{'message'}}[0]."\n";
    close $rand_fh;

    my $feedback = system("/usr/bin/goto-notify user-message '$to' '$subject' '$message' '$rand_file' &" );

    return
}


###############################################################################
=over 

=item B<trigger_action_localboot ($$)>

=over

=item description 

    Executes '/sbin/shutdown -r' if  no user is logged in otherwise write 
    'trigger_action_localboot' to '/etc/gosa-si/event'

=item parameter

    $msg - STRING - complete GOsa-si message
    $msg_hash - HASHREF - content of GOsa-si message in a hash

=item GOsa-si message xml content

    <timeout> - INTEGER - timeout to wait befor restart, default 0

=item return 
    
    Nothing.

=back

=back

=cut
###############################################################################
sub trigger_action_localboot {
    my ($msg, $msg_hash) = @_;
    my $timeout;

    if((not exists $msg_hash->{timeout} ) || (1 != @{$msg_hash->{timeout}} ) ) {
        $timeout = -1;
    } 
    else {
        $timeout = @{$msg_hash->{timeout}}[0];
    }

    # check logged in user
    my $logged_in_user = 1;
    if( $logged_in_user ) {
        # TODO do something
    }
    else {
        $timeout = 0;
    }
        
    # execute function
    if( $timeout == 0 ) {
        print STDERR ("shutdown -r +$timeout\n");
    }
    elsif( $timeout > 0 ) {
        print STDERR ("shutdown -r +$timeout\n");
    }
    elsif( $timeout < 0 ) {
        print STDERR "The administrator has sent a signal to reboot this workstation. It will reboot after you've logged out.\n";
        open(FILE, "> /etc/gosa-si/event");
        print FILE "trigger_action_localboot\n";
        close(FILE);
    }
    else {
        # TODO do something, error handling, logging
    }

    return;
}


###############################################################################
=over 

=item B<trigger_action_faireboot ($$)>

=over

=item description 

    Executes '/usr/sbin/faireboot'.

=item parameter

    $msg - STRING - complete GOsa-si message
    $msg_hash - HASHREF - content of GOsa-si message in a hash

=item GOsa-si message xml content

    None.

=item return 
    
    Nothing.

=back

=back

=cut
###############################################################################
sub trigger_action_faireboot {
    my ($msg, $msg_hash) = @_;
	&main::daemon_log("DEBUG: run /usr/sbin/faireboot\n", 7); 
    system("/usr/sbin/faireboot");
    return;
}


###############################################################################
=over 

=item B<trigger_action_reboot ($$)>

=over

=item description 

    Executes '/usr/bin/goto-notify reboot' and '/sbin/shutdown -r'  if  no 
    user is logged in otherwise write 'reboot' to '/etc/gosa-si/event'

=item parameter

    $msg - STRING - complete GOsa-si message
    $msg_hash - HASHREF - content of GOsa-si message in a hash

=item GOsa-si message xml content

    <timeout> - INTEGER - timeout to wait befor reboot, default 0

=item return 
    
    Nothing.

=back

=back

=cut
###############################################################################
sub trigger_action_reboot {
    my ($msg, $msg_hash) = @_;
    my $timeout;

    # check logged in user
    my @user_list = &get_logged_in_users;
    if( @user_list >= 1 ) {
    	system( "/usr/bin/goto-notify reboot" );
    }

    open(FILE, "> /etc/gosa-si/event");
    print FILE "reboot\n";
    close(FILE);

    system( "/usr/sbin/goto-action &" );

    return;
}


###############################################################################
=over 

=item B<trigger_action_halt ($$)>

=over

=item description 

    Executes '/usr/bin/goto-notify halt' and '/sbin/shutdown -h' if  no 
    user is logged in otherwise write 'halt' to '/etc/gosa-si/event'

=item parameter

    $msg - STRING - complete GOsa-si message
    $msg_hash - HASHREF - content of GOsa-si message in a hash

=item GOsa-si message xml content

    <timeout> - INTEGER - timeout to wait befor halt, default 0

=item return 
    
    Nothing.    

=back

=back

=cut
###############################################################################
sub trigger_action_halt {
    my ($msg, $msg_hash) = @_;
    my $timeout;

    # check logged in user
    my @user_list = &get_logged_in_users;
    if( @user_list >= 1 ) {
    	system( "/usr/bin/goto-notify halt" );
    }

    open(FILE, "> /etc/gosa-si/event");
    print FILE "halt\n";
    close(FILE);

    system( "/usr/sbin/goto-action &" );

    return;
}


###############################################################################
=over 

=item B<trigger_action_reinstall>

=over

=item description 

    Executes '/usr/bin/goto-notify install' and '/sbin/shutdown -r now' if no 
    user is logged in otherwise write 'install' to '/etc/gosa-si/event'

=item parameter

    $msg - STRING - complete GOsa-si message
    $msg_hash - HASHREF - content of GOsa-si message in a hash

=item GOsa-si message xml content

    None.

=item return 
    
    Nothing.

=back

=back

=cut
###############################################################################
sub trigger_action_reinstall {
    my ($msg, $msg_hash) = @_;

    # check logged in user
    my @user_list = &get_logged_in_users;
    if( @user_list >= 1 ) {
    	system( "/usr/bin/goto-notify install" );
        open(FILE, "> /etc/gosa-si/event");
        print FILE "install\n";
        close(FILE);
    }

    system( "/usr/sbin/goto-action &" );

    return;
}


###############################################################################
=over 

=item B<trigger_action_updae>

=over

=item description 

    Executes 'DEBIAN_FRONTEND=noninteractive /usr/sbin/fai-softupdate &'

=item parameter

    $msg - STRING - complete GOsa-si message
    $msg_hash - HASHREF - content of GOsa-si message in a hash

=item GOsa-si message xml content

    None.

=item return 
    
    Nothing

=back

=back

=cut
###############################################################################
# Backward compatibility
sub trigger_action_update {
    my ($msg, $msg_hash) = @_;

    # check logged in user
    my @user_list = &get_logged_in_users;
    if( @user_list >= 1 ) {
    	system( "/usr/bin/goto-notify softupdate" );
        open(FILE, "> /etc/gosa-si/event");
        print FILE "softupdate\n";
        close(FILE);
    }

    system( "/usr/sbin/goto-action &" );

    return;
}


###############################################################################
=over 

=item B<trigger_action_instant_update ($$)>

=over

=item description 

    Executes 'DEBIAN_FRONTEND=noninteractive /usr/sbin/fai-softupdate &'

=item parameter

    $msg - STRING - complete GOsa-si message
    $msg_hash - HASHREF - content of GOsa-si message in a hash

=item GOsa-si message xml content

    None.

=item return 
    
    Nothing.

=back

=back

=cut
###############################################################################
# Backward compatibility
sub trigger_action_instant_update {
    my ($msg, $msg_hash) = @_;

    # check logged in user
    my @user_list = &get_logged_in_users;
    if( @user_list >= 1 ) {
    	system( "/usr/bin/goto-notify softupdate" );
        open(FILE, "> /etc/gosa-si/event");
        print FILE "softupdate\n";
        close(FILE);
    }

    system( "/usr/sbin/goto-action &" );

    return;
}

sub trigger_goto_settings_reload {
    my ($msg, $msg_hash) = @_;

    # Execute goto settings reload
    my $cmd = "/etc/init.d/goto-agents";
    my $pram = "start";
    if (-f $cmd){
        my $feedback = system("$cmd $pram") or &main::daemon_log("ERROR: $@");
    } else {
        &main::daemon_log("ERROR: cannot exec $cmd, file not found!");
    }

    return;
}


1;
