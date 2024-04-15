# Define the simulation
set ns [new Simulator]

# Setup trace and Nam files
set tracefile [open ques_1.tr w]
set namfile [open ques_1.nam w]
$ns trace-all $tracefile
$ns namtrace-all $namfile

# Define a procedure to end the simulation
proc finish {} {
    global ns tracefile namfile
    $ns flush-trace
    close $tracefile
    close $namfile
    exec nam ques_1.nam &
    exit 0
}

# Create nodes
set A [$ns node]
set B [$ns node]
set C [$ns node]
set D [$ns node]

# Create links
$ns duplex-link $A $D 1Mb 10ms DropTail
$ns duplex-link $C $D 1Mb 20ms DropTail
$ns duplex-link $D $B 1Mb 10ms DropTail

# Setup FTP from A to B via D

set tcp [new Agent/TCP]
$ns attach-agent $A $tcp
set sink [new Agent/TCPSink]
$ns attach-agent $B $sink
$ns connect $tcp $sink


set ftp [new Application/FTP]
$ftp attach-agent $tcp
$ftp set type_ FTP


# Setup an unreliable connection from C to D
set udp [new Agent/UDP]
set null [new Agent/Null]
$ns attach-agent $C $udp
$ns attach-agent $D $null
$ns connect $udp $null

# Generate traffic for the unreliable connection
set cbr [new Application/Traffic/CBR]
$cbr set packetSize_ 500
$cbr set interval_ 0.05
$cbr attach-agent $udp

# Start and end the simulation
$ns at 0 "$ftp start"
$ns at 0 "$cbr start"
$ns at 0.5 "finish"

# Run the simulation
$ns run