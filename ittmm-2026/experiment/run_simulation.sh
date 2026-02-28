#!/bin/bash
# Run NS-2 RED Simulation and Generate Graphs

echo "=============================================="
echo "NS-2 RED Simulation - RUDN Conference"
echo "Based on Korolkova 2010, Velieva 2019"
echo "=============================================="

# Check if NS-2 is installed
if ! command -v ns &> /dev/null; then
    echo "Error: NS-2 not found!"
    echo "Install with: sudo apt install ns2"
    exit 1
fi

# Clean previous results
rm -f queue.tr red_queue.tr cwnd.tr queue_graph.*

# Run NS-2 simulation
echo "Running NS-2 simulation..."
ns red_simulation.tcl

# Check if trace files were created
if [ ! -f "red_queue.tr" ]; then
    echo "Error: red_queue.tr not created!"
    echo "Check NS-2 simulation for errors."
    exit 1
fi

# Process and plot results
echo "Processing queue data..."
python3 process_queue.py

echo "=============================================="
echo "Simulation complete!"
echo "Check queue_graph.pdf for results"
echo "=============================================="