package gosaTriggered;
use Exporter;
@ISA = qw(Exporter);
my @events = (
    "get_events",
    "trigger_action_localboot",
    "trigger_action_halt",
    "trigger_action_faireboot",
    "trigger_action_reboot",
    "trigger_action_memcheck",
    "trigger_action_reinstall",
    "trigger_action_update",
    "trigger_action_instant_update",
    "trigger_action_sysinfo",
    "trigger_action_rescan",
    );
@EXPORT = @events;

use strict;
use warnings;
use utf8;


BEGIN {}

END {}


sub get_events { return \@events; }


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


sub trigger_action_faireboot {
    my ($msg, $msg_hash) = @_;
    system("/usr/sbin/faireboot");
    return;
}


sub trigger_action_reboot {
    my ($msg, $msg_hash) = @_;
    print STDERR "jetzt würde ich trigger_action_reboot ausführen\n";
    return;
}


sub trigger_action_halt {
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
        print STDERR ("shutdown -h +$timeout\n");
    }
    elsif( $timeout > 0 ) {
        print STDERR ("shutdown -h +$timeout\n");
    }
    elsif( $timeout < 0 ) {
        print STDERR "The administrator has sent a signal to shutdown this workstation. It will shutdown after you've logged out.\n";
        open(FILE, "> /etc/gosa-si/event");
        print FILE "trigger_action_halt\n";
        close(FILE);
    }
    else {
        # TODO do something, error handling, logging
    }

    return;
}


# TODO
sub trigger_action_memcheck {
    my ($msg, $msg_hash) = @_ ;
    print STDERR "\n\njetzt würde ich trigger_action_memcheck ausführen\n\n";
    return;
}


sub trigger_action_reinstall {
    my ($msg, $msg_hash) = @_;
    my $timeout;

    # check timeout
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
        print STDERR "The administrator has sent a signal to reinstall this workstation. It will reinstall after you've logged out.\n";
        open(FILE, "> /etc/gosa-si/event");
        print FILE "trigger_action_reinstall\n";
        close(FILE);
    }
    else {
        # TODO do something, error handling, logging
    }

    return;
}


sub trigger_action_update {
    my ($msg, $msg_hash) = @_;

    # execute function
    open(FILE, "> /etc/gosa-si/event");
    print FILE "trigger_action_reinstall\n";
    close(FILE);

    # check logged in user
    my $logged_in_user = 1;
    if( $logged_in_user ) {
        print STDERR "This system has been sent a signal for an update. The update will take place after you log out.\n";
    }
    else {
        # not jet
        #system( "DEBIAN_FRONTEND=noninteractive /usr/sbin/fai -N softupdate &" )
    }

    return;
}


sub trigger_action_instant_update {
    my ($msg, $msg_hash) = @_;

    # check logged in user
    my $logged_in_user = 1;
    if( $logged_in_user ) {
        print STDERR "This system has been sent a signal for an update. The update will take place now.\n";
    }

    # not jet
    #system( "DEBIAN_FRONTEND=noninteractive /usr/sbin/fai -N softupdate &" )

    return;
}


# TODO
sub trigger_action_sysinfo {
    my ($msg, $msg_hash) = @_;
    print STDERR "jetzt würde ich trigger_action_sysinfo ausführen\n";
    return;
}

# TODO
sub trigger_action_rescan{
    my ($msg, $msg_hash) = @_;
    print STDERR "jetzt würde ich trigger_action_rescan ausführen\n";
    return;
}

1;
