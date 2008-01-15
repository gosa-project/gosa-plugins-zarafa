package GOSA::GosaSupportDaemon;

use Exporter;
@ISA = qw(Exporter);
@EXPORT = qw(create_xml_hash send_msg_hash2address get_content_from_xml_hash add_content2xml_hash create_xml_string encrypt_msg decrypt_msg create_ciphering transform_msg2hash get_time send_msg); 

use strict;
use warnings;
use IO::Socket::INET;
use Crypt::Rijndael;
use Digest::MD5  qw(md5 md5_hex md5_base64);
use MIME::Base64;
use XML::Simple;



BEGIN {}

END {}

### Start ######################################################################

my $xml = new XML::Simple();

sub process_incoming_msg {
    return;
}

sub daemon_log {
    my ($msg, $level) = @_ ;
    &main::daemon_log($msg, $level);
    return;
}


#===  FUNCTION  ================================================================
#         NAME:  create_xml_hash
#   PARAMETERS:  header - string - message header (required)
#                source - string - where the message come from (required)
#                target - string - where the message should go to (required)
#                [header_value] - string - something usefull (optional)
#      RETURNS:  hash - hash - nomen est omen
#  DESCRIPTION:  creates a key-value hash, all values are stored in a array
#===============================================================================
sub create_xml_hash {
    my ($header, $source, $target, $header_value) = @_;
    my $hash = {
            header => [$header],
            source => [$source],
            target => [$target],
            $header => [$header_value],
    };
    return $hash
}


sub transform_msg2hash {
    my ($msg) = @_ ;
    my $hash = $xml->XMLin($msg, ForceArray=>1);
    return $hash;
}


#===  FUNCTION  ================================================================
#         NAME:  send_msg_hash2address
#   PARAMETERS:  msg_hash - hash - xml_hash created with function create_xml_hash
#                PeerAddr string - socket address to send msg
#                PeerPort string - socket port, if not included in socket address
#      RETURNS:  nothing
#  DESCRIPTION:  ????
#===============================================================================
sub send_msg_hash2address ($$$){
    my ($msg_hash, $address, $passwd) = @_ ;

    # fetch header for logging
    my $header = @{$msg_hash->{header}}[0];  

    # generate xml string
    my $msg_xml = &create_xml_string($msg_hash);
    
    # create ciphering object
    my $act_cipher = &create_ciphering($passwd);
    
    # encrypt xml msg
    my $crypted_msg = &encrypt_msg($msg_xml, $act_cipher);
    
    # opensocket
    my $socket = &open_socket($address);
    if(not defined $socket){
        daemon_log("cannot send '$header'-msg to $address , server not reachable", 5);
        return 1;
    }
    
    # send xml msg
    print $socket $crypted_msg."\n";
    
    close $socket;

    daemon_log("send '$header'-msg to $address", 1);
    daemon_log("$msg_xml", 5);
    return 0;
}


#===  FUNCTION  ================================================================
#         NAME:  get_content_from_xml_hash
#   PARAMETERS:  xml_ref - ref - reference of the xml hash
#                element - string - key of the value you want
#      RETURNS:  value - string - if key is either header, target or source
#                value - list - for all other keys in xml hash
#  DESCRIPTION:
#===============================================================================
sub get_content_from_xml_hash {
    my ($xml_ref, $element) = @_ ;
    #my $result = $main::xml_ref->{$element};
    #if( $element eq "header" || $element eq "target" || $element eq "source") {
    #    return @$result[0];
    #}
    my @result = $xml_ref->{$element};
    return \@result;
}


#===  FUNCTION  ================================================================
#         NAME:  add_content2xml_hash
#   PARAMETERS:  xml_ref - ref - reference to a hash from function create_xml_hash
#                element - string - key for the hash
#                content - string - value for the hash
#      RETURNS:  nothing
#  DESCRIPTION:  add key-value pair to xml_ref, if key alread exists, 
#                then append value to list
#===============================================================================
sub add_content2xml_hash {
    my ($xml_ref, $element, $content) = @_;
    if(not exists $$xml_ref{$element} ) {
        $$xml_ref{$element} = [];
    }
    my $tmp = $$xml_ref{$element};
    push(@$tmp, $content);
    return;
}


#===  FUNCTION  ================================================================
#         NAME:  create_xml_string
#   PARAMETERS:  xml_hash - hash - hash from function create_xml_hash
#      RETURNS:  xml_string - string - xml string representation of the hash
#  DESCRIPTION:  transform the hash to a string using XML::Simple module
#===============================================================================
sub create_xml_string {
    my ($xml_hash) = @_ ;
    my $xml_string = $xml->XMLout($xml_hash, RootName => 'xml');
    #$xml_string =~ s/[\n]+//g;
    #daemon_log("create_xml_string:",7);
    #daemon_log("$xml_string\n", 7);
    return $xml_string;
}


#===  FUNCTION  ================================================================
#         NAME:  encrypt_msg
#   PARAMETERS:  msg - string - message to encrypt
#                my_cipher - ref - reference to a Crypt::Rijndael object
#      RETURNS:  crypted_msg - string - crypted message
#  DESCRIPTION:  crypts the incoming message with the Crypt::Rijndael module
#===============================================================================
sub encrypt_msg {
    my ($msg, $my_cipher) = @_;
    if(not defined $my_cipher) { print "no cipher object\n"; }
    $msg = "\0"x(16-length($msg)%16).$msg;
    $msg = $my_cipher->encrypt($msg);
    chomp($msg = &encode_base64($msg));
    return $msg;
}


#===  FUNCTION  ================================================================
#         NAME:  decrypt_msg
#   PARAMETERS:  crypted_msg - string - message to decrypt
#                my_cipher - ref - reference to a Crypt::Rijndael object
#      RETURNS:  msg - string - decrypted message
#  DESCRIPTION:  decrypts the incoming message with the Crypt::Rijndael module
#===============================================================================
sub decrypt_msg {
    my ($msg, $my_cipher) = @_ ;
    if(defined $msg && defined $my_cipher) {
        $msg = &decode_base64($msg);
    }
    $msg = $my_cipher->decrypt($msg); 
    $msg =~ s/\0*//g;
    return $msg;
}


#===  FUNCTION  ================================================================
#         NAME:  create_ciphering
#   PARAMETERS:  passwd - string - used to create ciphering
#      RETURNS:  cipher - object
#  DESCRIPTION:  creates a Crypt::Rijndael::MODE_CBC object with passwd as key
#===============================================================================
sub create_ciphering {
    my ($passwd) = @_;
    $passwd = substr(md5_hex("$passwd") x 32, 0, 32);
    my $iv = substr(md5_hex('GONICUS GmbH'),0, 16);

    #daemon_log("iv: $iv", 7);
    #daemon_log("key: $passwd", 7);
    my $my_cipher = Crypt::Rijndael->new($passwd , Crypt::Rijndael::MODE_CBC());
    $my_cipher->set_iv($iv);
    return $my_cipher;
}


#===  FUNCTION  ================================================================
#         NAME:  open_socket
#   PARAMETERS:  PeerAddr string something like 192.168.1.1 or 192.168.1.1:10000
#                [PeerPort] string necessary if port not appended by PeerAddr
#      RETURNS:  socket IO::Socket::INET
#  DESCRIPTION:  open a socket to PeerAddr
#===============================================================================
sub open_socket {
    my ($PeerAddr, $PeerPort) = @_ ;
    if(defined($PeerPort)){
        $PeerAddr = $PeerAddr.":".$PeerPort;
    }
    my $socket;
    $socket = new IO::Socket::INET(PeerAddr => $PeerAddr,
            Porto => "tcp",
            Type => SOCK_STREAM,
            Timeout => 5,
            );
    if(not defined $socket) {
        return;
    }
    &daemon_log("open_socket:", 7);
    &daemon_log("\t$PeerAddr", 7);
    return $socket;
}


sub get_time {
    my ($seconds, $minutes, $hours, $monthday, $month,
            $year, $weekday, $yearday, $sommertime) = localtime(time);
    $hours = $hours < 10 ? $hours = "0".$hours : $hours;
    $minutes = $minutes < 10 ? $minutes = "0".$minutes : $minutes;
    $seconds = $seconds < 10 ? $seconds = "0".$seconds : $seconds;
    $month+=1;
    $month = $month < 10 ? $month = "0".$month : $month;
    $monthday = $monthday < 10 ? $monthday = "0".$monthday : $monthday;
    $year+=1900;
    return "$year$month$monthday$hours$minutes$seconds";

}


#===  FUNCTION  ================================================================
#         NAME: send_msg
#  DESCRIPTION: Send a message to a destination
#   PARAMETERS: [header] Name of the header
#               [from]   sender ip
#               [to]     recipient ip
#               [data]   Hash containing additional attributes for the xml
#                        package
#      RETURNS:  nothing
#===============================================================================
sub send_msg ($$$$$) {
	my ($header, $from, $to, $data, $hostkey) = @_;

	my $out_hash = &create_xml_hash($header, $from, $to);

	while ( my ($key, $value) = each(%$data) ) {
		if(ref($value) eq 'ARRAY'){
			map(&add_content2xml_hash($out_hash, $key, $_), @$value);
		} else {
			&add_content2xml_hash($out_hash, $key, $value);
		}
	}

	&send_msg_hash2address($out_hash, $to, $hostkey);
}

1;
