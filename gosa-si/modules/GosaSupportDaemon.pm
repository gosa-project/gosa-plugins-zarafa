package GOSA::GosaSupportDaemon;

use Exporter;
@ISA = qw(Exporter);
my @functions = (
    "create_xml_hash",
    "get_content_from_xml_hash",
    "add_content2xml_hash",
    "create_xml_string",
    "encrypt_msg",
    "decrypt_msg",
    "create_ciphering",
    "transform_msg2hash",
    "get_time",
    "build_msg",
    "get_where_statement",
    "get_select_statement",
    "get_update_statement",
    "get_limit_statement",
    "get_orderby_statement",
    ); 
@EXPORT = @functions;
use strict;
use warnings;
use IO::Socket::INET;
use Crypt::Rijndael;
use Digest::MD5  qw(md5 md5_hex md5_base64);
use MIME::Base64;
use XML::Simple;

my $op_hash = {
    'eq' => '=',
    'ne' => '!=',
    'ge' => '>=',
    'gt' => '>',
    'le' => '<=',
    'lt' => '<',
};


BEGIN {}

END {}

### Start ######################################################################

my $xml = new XML::Simple();

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


sub transform_msg2hash {
    my ($msg) = @_ ;
    my $hash = $xml->XMLin($msg, ForceArray=>1);
    
    # xml tags without a content are created as an empty hash
    # substitute it with an empty list
    eval {
        while( my ($xml_tag, $xml_content) = each %{ $hash } ) {
            if( 1 == @{ $xml_content } ) {
                # there is only one element in xml_content list ...
                my $element = @{ $xml_content }[0];
                if( ref($element) eq "HASH" ) {
                    # and this element is an hash ...
                    my $len_element = keys %{ $element };
                    if( $len_element == 0 ) {
                        # and this hash is empty, then substitute the xml_content
                        # with an empty string in list
                        $hash->{$xml_tag} = [ "none" ];
                    }
                }
            }
        }
    };
    if( $@ ) {  
        $hash = undef;
    }

    return $hash;
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
#         NAME:  encrypt_msg
#   PARAMETERS:  msg - string - message to encrypt
#                my_cipher - ref - reference to a Crypt::Rijndael object
#      RETURNS:  crypted_msg - string - crypted message
#  DESCRIPTION:  crypts the incoming message with the Crypt::Rijndael module
#===============================================================================
sub encrypt_msg {
#    my ($msg, $my_cipher) = @_;
#    if(not defined $my_cipher) { print "no cipher object\n"; }
#    {
#      use bytes;
#      $msg = "\0"x(16-length($msg)%16).$msg;
#    }
#    $msg = $my_cipher->encrypt($msg);
#    chomp($msg = &encode_base64($msg));
#
#    # there are no newlines allowed inside msg
#    $msg=~ s/\n//g;
#
#    return $msg;
    my ($msg, $key) = @_;
    my $my_cipher = &create_ciphering($key);
    {
      use bytes;
      $msg = "\0"x(16-length($msg)%16).$msg;
    }
    $msg = $my_cipher->encrypt($msg);
    chomp($msg = &encode_base64($msg));
    # there are no newlines allowed inside msg
    $msg=~ s/\n//g;
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
#    my ($msg, $my_cipher) = @_ ;
#    
#    if(defined $msg && defined $my_cipher) {
#        $msg = &decode_base64($msg);
#    }
#    $msg = $my_cipher->decrypt($msg); 
#    $msg =~ s/\0*//g;
#    return $msg;
    my ($msg, $key) = @_ ;
    $msg = &decode_base64($msg);
    my $my_cipher = &create_ciphering($key);
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
#sub open_socket {
#    my ($PeerAddr, $PeerPort) = @_ ;
#    if(defined($PeerPort)){
#        $PeerAddr = $PeerAddr.":".$PeerPort;
#    }
#    my $socket;
#    $socket = new IO::Socket::INET(PeerAddr => $PeerAddr,
#            Porto => "tcp",
#            Type => SOCK_STREAM,
#            Timeout => 5,
#            );
#    if(not defined $socket) {
#        return;
#    }
#    &daemon_log("open_socket: $PeerAddr", 7);
#    return $socket;
#}


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
#         NAME: build_msg
#  DESCRIPTION: Send a message to a destination
#   PARAMETERS: [header] Name of the header
#               [from]   sender ip
#               [to]     recipient ip
#               [data]   Hash containing additional attributes for the xml
#                        package
#      RETURNS:  nothing
#===============================================================================
sub build_msg ($$$$) {
	my ($header, $from, $to, $data) = @_;

	my $out_hash = &create_xml_hash($header, $from, $to);

	while ( my ($key, $value) = each(%$data) ) {
		if(ref($value) eq 'ARRAY'){
			map(&add_content2xml_hash($out_hash, $key, $_), @$value);
		} else {
			&add_content2xml_hash($out_hash, $key, $value);
		}
	}
    my $out_msg = &create_xml_string($out_hash);
    return $out_msg;
}


sub get_where_statement {
    my ($msg, $msg_hash) = @_;
    my $error= 0;
    
    my $clause_str= "";
    if( (not exists $msg_hash->{'where'}) || (not exists @{$msg_hash->{'where'}}[0]->{'clause'}) ) { 
        $error++; 
    }

    if( $error == 0 ) {
        my @clause_l;
        my @where = @{@{$msg_hash->{'where'}}[0]->{'clause'}};
        foreach my $clause (@where) {
            my $connector = $clause->{'connector'}[0];
            if( not defined $connector ) { $connector = "AND"; }
            $connector = uc($connector);
            delete($clause->{'connector'});

            my @phrase_l ;
            foreach my $phrase (@{$clause->{'phrase'}}) {
                my $operator = "=";
                if( exists $phrase->{'operator'} ) {
                    my $op = $op_hash->{$phrase->{'operator'}[0]};
                    if( not defined $op ) {
                        &main::daemon_log("Can not translate operator '$operator' in where ".
                                "statement to sql valid syntax. Please use 'eq', ".
                                "'ne', 'ge', 'gt', 'le', 'lt' in xml message\n", 1);
                        &main::daemon_log($msg, 8);
                        $op = "=";
                    }
                    $operator = $op;
                    delete($phrase->{'operator'});
                }

                my @xml_tags = keys %{$phrase};
                my $tag = $xml_tags[0];
                my $val = $phrase->{$tag}[0];
                push(@phrase_l, "$tag$operator'$val'");
            }
            my $clause_str .= join(" $connector ", @phrase_l);
            push(@clause_l, $clause_str);
        }

        if( not 0 == @clause_l ) {
            $clause_str = join(" AND ", @clause_l);
            $clause_str = "WHERE $clause_str ";
        }
    }

    return $clause_str;
}

sub get_select_statement {
    my ($msg, $msg_hash)= @_;
    my $select = "*";
    if( exists $msg_hash->{'select'} ) {
        my $select_l = \@{$msg_hash->{'select'}};
        $select = join(' AND ', @{$select_l});
    }
    return $select;
}


sub get_update_statement {
    my ($msg, $msg_hash) = @_;
    my $error= 0;
    my $update_str= "";
    my @update_l; 

    if( not exists $msg_hash->{'update'} ) { $error++; };

    if( $error == 0 ) {
        my $update= @{$msg_hash->{'update'}}[0];
        while( my ($tag, $val) = each %{$update} ) {
            my $val= @{$update->{$tag}}[0];
            push(@update_l, "$tag='$val'");
        }
        if( 0 == @update_l ) { $error++; };   
    }

    if( $error == 0 ) { 
        $update_str= join(', ', @update_l);
        $update_str= "SET $update_str ";
    }

    return $update_str;
}

sub get_limit_statement {
    my ($msg, $msg_hash)= @_; 
    my $error= 0;
    my $limit_str = "";
    my ($from, $to);

    if( not exists $msg_hash->{'limit'} ) { $error++; };

    if( $error == 0 ) {
        eval {
            my $limit= @{$msg_hash->{'limit'}}[0];
            $from= @{$limit->{'from'}}[0];
            $to= @{$limit->{'to'}}[0];
        };
        if( $@ ) {
            $error++;
        }
    }

    if( $error == 0 ) {
        $limit_str= "LIMIT $from, $to";
    }   
    
    return $limit_str;
}

sub get_orderby_statement {
    my ($msg, $msg_hash)= @_;
    my $error= 0;
    my $order_str= "";
    my $order;
    
    if( not exists $msg_hash->{'orderby'} ) { $error++; };

    if( $error == 0) {
        eval {
            $order= @{$msg_hash->{'orderby'}}[0];
        };
        if( $@ ) {
            $error++;
        }
    }

    if( $error == 0 ) {
        $order_str= "ORDER BY $order";   
    }
    
    return $order_str;
}

1;
