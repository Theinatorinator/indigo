#!/usr/bin/perl -w
use strict;
use IO::Socket;
use Time::HiRes qw/gettimeofday/;

my($sock, $newmsg, $MAXLEN, $PORTNO);

my @relay = (0,0,0,0,0,0,0,0);
my @end = (0,0,0,0,0,0,0,0);
my @sensor = (11,12,13,14,15,16,17,18);

$MAXLEN = 1024;
$PORTNO = 10000;

$sock = IO::Socket::INET->new(
	LocalPort => $PORTNO,
	Proto => 'udp'
) or die "socket: $@";
       
print "Listening for UDP messages on port $PORTNO\n";

while ($sock->recv($newmsg, $MAXLEN)) {
	print "COMMAND: $newmsg\n";
	$newmsg =~ s/(\!)//;
	$newmsg =~ s/(^\n)//;
	$newmsg =~ s/(\#)//;

	my $now = int(gettimeofday() * 1000);
	for (my $i = 0; $i < 8; $i++) {
		if ($end[$i] > 0 and $now >= $end[$i]) {
			$relay[$i] = 0;
			$end[$i] = 0;
		}
	}

	my @command = split(/\s+/, $newmsg);
	if (($command[0] eq "seletek") and ($command[1] eq "version")) {
		$sock->send("\!seletek version:4529\#");
	} elsif ($command[0] eq "relio") {
		if ($command[1] eq "rlpulse" and @command == 5) {
			$end[$command[3]] = $command[4] + $now;
			$relay[$command[3]] = 1;
			$sock->send("\!$newmsg:0\#");
		} elsif ($command[1] eq "rldgrd" and @command == 5) {
			my $resp = "\!$newmsg:".$relay[0].",".$relay[1].",".$relay[2].",".$relay[3].",".$relay[4].",".$relay[5].",".$relay[6].",".$relay[7]."\#";
			$sock->send($resp);
		} elsif ($command[1] eq "rldgrd" and @command == 4) {
			$sock->send("\!$newmsg:".$relay[$command[3]]."\#");
		} elsif ($command[1] eq "rlset" and @command == 5) {
			$relay[$command[3]] = $command[4];
		       	my $resp = "\!$newmsg:".$relay[$command[3]]."\#";	
                        $sock->send("$resp");
                } elsif ($command[1] eq "snanrd" and @command == 5) {
                        my $resp = "\!$newmsg:".$sensor[0].",".$sensor[1].",".$sensor[2].",".$sensor[3].",".$sensor[4].",".$sensor[5].",".$sensor[6].",".$sensor[7]."\#";
                        $sock->send($resp);
		} elsif ($command[1] eq "snanrd" and @command == 4) {
                        $sock->send("\!$newmsg:".$sensor[$command[3]]."\#");
		} elsif ($command[1] eq "sndgrd" and @command == 4) {
			if ($sensor[$command[3]] > 512) {
                        	$sock->send("\!$newmsg:1\#");
			} else {
				$sock->send("\!$newmsg:0\#");
			}
		} elsif ($command[1] eq "rlchg" and @command == 4) {
                        if ($sensor[$command[3]] > 0) {
				$sensor[$command[3]] = 0;
                                $sock->send("\!$newmsg:0\#");
                        } else {
				$sensor[$command[3]] = 1;
                                $sock->send("\!$newmsg:1\#");
                        }
		# for debug only does not respond
		} elsif ($command[1] eq "snset" and @command == 5) {
                        $sensor[$command[3]] = $command[4];
                        my $resp = "\!$newmsg:".$sensor[$command[3]]."\#";
                        print "$resp\n";
		} else {
			$sock->send("\!$newmsg:-1\#");
		}
	} else {
		$sock->send("\!$newmsg:-1\#");
	}
} 
die "recv: $!";
