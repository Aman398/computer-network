# Define the simulation environment
set ns [new Simulator]

# Setup trace and Nam files for analysis and visualization
set tracefile [open A5_Q2.tr w]
set namfile [open A5_Q2.nam w]
$ns trace-all $tracefile
$ns namtrace-all $namfile

# Define a procedure to terminate the simulation
proc finish {} {
    global ns tracefile namfile
    $ns flush-trace
    close $tracefile
    close $namfile
    exec nam A5_Q2.nam &
    exit 0
}

# Create network nodes
set A [$ns node]
set B [$ns node]
set C [$ns node]
set D [$ns node]

# Create links between nodes with specified bandwidth, delay, and queue type
$ns duplex-link $A $B 2Mb 10ms DropTail
$ns duplex-link $C $B 2Mb 20ms DropTail
$ns duplex-link $B $D 2Mb 10ms DropTail

# FTP connection from A to B
set ftp [new Application/FTP]
set tcp [new Agent/TCP]
$ns attach-agent $A $tcp
set sink [new Agent/TCPSink]
$ns attach-agent $B $sink
$ns connect $tcp $sink
$ftp attach-agent $tcp
$ftp set type_ FTP

# Unreliable connection from C to B using UDP
set udp [new Agent/UDP]
$ns attach-agent $C $udp
set null [new Agent/Null]
$ns attach-agent $B $null
$ns connect $udp $null

# Reliable connection from B to D using TCP
set tcpBD [new Agent/TCP]
$ns attach-agent $B $tcpBD
set sinkBD [new Agent/TCPSink]
$ns attach-agent $D $sinkBD
$ns connect $tcpBD $sinkBD

# Setup a CBR source for the UDP agent to simulate traffic
set cbr [new Application/Traffic/CBR]
$cbr attach-agent $udp
$cbr set packetSize_ 500
$cbr set interval_ 0.05


# Define the simulation start and end times for the applications
$ns at 0.5 "$ftp start"
$ns at 1.0 "$cbr start"
$ns at 5.0 "finish"

# Execute the simulation
$ns run