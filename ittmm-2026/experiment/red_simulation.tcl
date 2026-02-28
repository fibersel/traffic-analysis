# ============================================================================
# NS-2 RED Queue Dynamics Simulation
# Based on Velieva 2019 PhD Thesis (RUDN), Appendix A.4
# Korolkova 2010 PhD Thesis, Section 3.3
# ============================================================================

# Create simulator object
set ns [new Simulator]

# Set simulation duration
set simTime 60.0

# Open trace file for queue statistics (CRITICAL for your graph)
set redtrace [open red_queue.tr w]

# ============================================================================
# RED Algorithm Parameters (From Velieva 2019, Section 3.3)
# ============================================================================
Queue/RED set bytes_ false
Queue/RED set queue_in_bytes_ false
Queue/RED set gentle_ false
Queue/RED set mean_pktsize_ 1000
Queue/RED set q_weight_ 0.002
Queue/RED set linterm_ 10
Queue/RED set setbit_ false
Queue/RED set drop_tail_ false
Queue/RED set cur_max_p_ 0.1

# RED Thresholds (q_min = 20, q_max = 60 packets)
Queue/RED set thresh_ 20
Queue/RED set maxthresh_ 60

# Number of TCP sources (N = 60 from Velieva 2019)
set numSrc 60

# ============================================================================
# Create Network Topology
# ============================================================================

# Create router nodes (R1 and R2)
set R1 [$ns node]
set R2 [$ns node]

# Create bottleneck link with RED queue (R1 -> R2)
# 15 Mbps bandwidth, 35ms delay (from Velieva 2019)
$ns simplex-link $R1 $R2 15Mb 35ms RED
$ns simplex-link $R2 $R1 15Mb 35ms DropTail
$ns queue-limit $R1 $R2 300

# ============================================================================
# Create TCP Sources and Sinks
# ============================================================================

for {set i 1} {$i <= $numSrc} {incr i} {
    # Create source node
    set n($i) [$ns node]
    
    # Create link from source to R1 (100 Mbps, 20ms)
    $ns duplex-link $n($i) $R1 100Mb 20ms DropTail
    
    # Create TCP agent (Reno)
    set tcp($i) [new Agent/TCP/Reno]
    $ns attach-agent $n($i) $tcp($i)
    $tcp($i) set window_ 32
    $tcp($i) set packetSize_ 1000
    $tcp($i) set fid_ $i
    
    # Create FTP application
    set ftp($i) [new Application/FTP]
    $ftp($i) attach-agent $tcp($i)
    $ftp($i) set type_ FTP
    
    # Create sink node
    set s($i) [$ns node]
    
    # Create link from R2 to sink (100 Mbps, 20ms)
    $ns duplex-link $s($i) $R2 100Mb 20ms DropTail
    
    # Create TCP sink agent
    set sink($i) [new Agent/TCPSink]
    $ns attach-agent $s($i) $sink($i)
    
    # Connect TCP source to sink
    $ns connect $tcp($i) $sink($i)
}

# ============================================================================
# Queue Monitoring (CRITICAL - This creates red_queue.tr)
# ============================================================================

# Monitor queue on bottleneck link (R1 -> R2)
set qmon [$ns monitor-queue $R1 $R2 [open queue.tr w] 0.01]
[$ns link $R1 $R2] queue-sample-timeout

# Track RED-specific parameters (current queue and average queue)
set redq [[$ns link $R1 $R2] queue]
$redq trace curq_
$redq trace ave_
$redq attach $redtrace

# ============================================================================
# TCP Window Monitoring (Optional, for congestion analysis)
# ============================================================================

proc plotWindow {tcpSource file k} {
    global ns numSrc
    set time 0.05
    set now [$ns now]
    set cwnd [$tcpSource set cwnd_]
    
    if {$k == 1} {
        puts -nonewline $file "$now\t$cwnd\t"
    } else {
        if {$k < $numSrc} {
            puts -nonewline $file "$cwnd\t"
        }
    }
    
    if {$k == $numSrc} {
        puts -nonewline $file "$cwnd\n"
    }
    
    $ns at [expr $now + $time] "plotWindow $tcpSource $file $k"
}

# Open file for TCP window tracking
set cwndtrace [open cwnd.tr w]

# Start window monitoring for all TCP sources
for {set j 1} {$j <= $numSrc} {incr j} {
    $ns at 0.1 "plotWindow $tcp($j) $cwndtrace $j"
}

# ============================================================================
# Schedule Events
# ============================================================================

# Start all FTP applications at time 0
for {set i 1} {$i <= $numSrc} {incr i} {
    $ns at 0.0 "$ftp($i) start"
    $ns at $simTime "$ftp($i) stop"
}

# Finish procedure
proc finish {} {
    global ns redtrace cwndtrace
    $ns flush-trace
    close $redtrace
    close $cwndtrace
    exit 0
}

$ns at $simTime "finish"

# ============================================================================
# Run Simulation
# ============================================================================

puts "Starting NS-2 RED Simulation..."
puts "Simulation Time: $simTime seconds"
puts "Number of TCP Sources: $numSrc"
puts "RED q_min: 20 packets, q_max: 60 packets"
puts "Output files: red_queue.tr, queue.tr, cwnd.tr"

$ns run

puts "Simulation completed successfully!"