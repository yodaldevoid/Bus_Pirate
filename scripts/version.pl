#!/usr/bin/env perl -w

# This file is part of the Bus Pirate project
# (http://code.google.com/p/the-bus-pirate/).
#
# Written and maintained by the Bus Pirate project.
#
# To the extent possible under law, the project has waived all copyright and
# related or neighboring rights to Bus Pirate. This work is published from
# United States.
#
# For details see: http://creativecommons.org/publicdomain/zero/1.0/.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.

use strict;
use warnings;

use Carp;
use Config;
use English qw(-no_match_vars);
use Getopt::Std;
use Term::ANSIColor;

our $VERSION = '0.0.1';

my %options;
my $port;
my @version;
my $reset_tag;
my $hardware_type;
my $firmware_version;
my $hardware_version;
my $update_url;

getopt( 'ph', \%options );

if ( exists $options{'h'} ) {
    print "# Get version string. Use with firmware v7.0 and later.\n";
    print "# -h This screen.\n";
    print "# -p Serial port name (default " . get_default_port_name() . ").\n";
    print "# Example: "
      . $PROGRAM_NAME . " -p "
      . get_default_port_name() . "\n";
    exit 0;
}

$port = open_port( $options{'p'} || get_default_port_name() );

$port->databits(8);
$port->baudrate(115_200);
$port->parity('none');
$port->stopbits(1);
$port->buffers( 1, 1 );
$port->write_settings() || undef $port;

if ( not defined $port ) {
    print "Could not write serial port settings!\n";
    exit -1;
}

$port->write("\n\n\n\n\n\n\n\n\n\n#\n");
sleep 1;
@version = split( /\r\n/smx, $port->read(1000) );
$port->close();

if ( scalar(@version) < 6 ) {
    print "\nPlease reset the Bus Pirate unit and try again\n";
    exit -2;
}

(
    $reset_tag, $hardware_type, $firmware_version, $hardware_version,
    $update_url
) = @version[ -6 .. -1 ];

if ( $reset_tag ne 'RESET' ) {
    print "\nPlease reset the Bus Pirate unit and try again\n";
    exit -2;
}

print color('yellow')
  . 'Hardware type:    '
  . color('bold white')
  . $hardware_type
  . color('reset') . "\n";
print color('yellow')
  . 'Firmware version: '
  . color('bold white')
  . $firmware_version
  . color('reset') . "\n";
print color('yellow')
  . 'Hardware version: '
  . color('bold white')
  . $hardware_version
  . color('reset') . "\n";
print color('yellow')
  . 'Update URL:       '
  . color('bold white')
  . $update_url
  . color('reset') . "\n";

exit 0;

sub get_default_port_name {
    my $osname = $Config{'osname'};

    if ( $osname eq 'MSWin32' ) {
        return 'COM1';
    }
    elsif ( $osname eq 'linux' ) {
        return '/dev/ttyACM0';
    }
    elsif ( $osname eq 'darwin' ) {
        return '/dev/cu.usbmodem00000001';
    }
    else {
        return '/dev/ttyUSB0';
    }
}

sub open_port {
    my $port_name = shift;

    if ( $Config{'osname'} eq 'MSWin32' ) {
        require Win32::SerialPort;
        return Win32::SerialPort->new($port_name);
    }
    else {
        require Device::SerialPort;
        return Device::SerialPort->new($port_name);
    }
}

# vim:number:cc=80:

