#!/usr/bin/env python3
"""
Process NS-2 Queue Trace File
Based on Velieva 2019, Appendix A.5 (pp. 112-115)
Fixed for NS-2 trace format: [event_type] [time] [value]
"""

import matplotlib.pyplot as plt
import numpy as np

plt.rcParams['font.family'] = 'DejaVu Sans'

def parse_ns2_trace(filename='red_queue.tr'):
    """Parse NS-2 RED queue trace file"""
    time_avg = []
    queue_avg = []  # Average queue length (EWMA) - 'a' events
    time_inst = []
    queue_inst = []  # Instantaneous queue length - 'Q' events
    
    try:
        with open(filename, 'r') as f:
            for line in f:
                parts = line.split()
                if len(parts) >= 3:
                    try:
                        event_type = parts[0]
                        timestamp = float(parts[1])
                        value = float(parts[2])
                        
                        if event_type == 'Q':
                            # Instantaneous queue length
                            queue_inst.append(value)
                            time_inst.append(timestamp)
                        elif event_type == 'a':
                            # Average queue length (EWMA)
                            queue_avg.append(value)
                            time_avg.append(timestamp)
                    except (ValueError, IndexError) as e:
                        pass
    except FileNotFoundError:
        print(f"Error: {filename} not found!")
        print("Run NS-2 simulation first: ns red_simulation.tcl")
        return None, None, None, None
    
    if len(time_inst) == 0 and len(time_avg) == 0:
        print("No valid data found in trace file!")
        return None, None, None, None
    
    return time_inst, queue_inst, time_avg, queue_avg

def plot_queue_dynamics(time_inst, queue_inst, time_avg, queue_avg):
    """Plot queue dynamics with RED thresholds"""
    
    if time_inst is None or (len(time_inst) == 0 and len(time_avg) == 0):
        print("No data to plot!")
        return
    
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(14, 10))
    
    # Plot 1: Queue Length Over Time
    if len(time_inst) > 0:
        ax1.plot(time_inst, queue_inst, label='Instantaneous Queue (Q)', 
                color='blue', linewidth=1, alpha=0.5)
    if len(time_avg) > 0:
        ax1.plot(time_avg, queue_avg, label='Average Queue (EWMA)', 
                color='red', linewidth=2)
    
    # RED thresholds
    ax1.axhline(y=20, color='green', linestyle='--', label='q_min (20)', linewidth=2)
    ax1.axhline(y=60, color='orange', linestyle='--', label='q_max (60)', linewidth=2)
    
    ax1.set_xlabel('Time (seconds)', fontsize=12)
    ax1.set_ylabel('Queue Length (packets)', fontsize=12)
    ax1.set_title('Динамика длины очереди при работе алгоритма RED\n(Queue Dynamics During RED Algorithm Operation)', 
                  fontsize=14, fontweight='bold')
    ax1.legend(loc='upper right', fontsize=10)
    ax1.grid(True, which='both', linestyle='--', alpha=0.7)
    
    # Set axis limits
    if len(time_inst) > 0:
        ax1.set_xlim(0, max(time_inst))
        ax1.set_ylim(0, max(max(queue_inst) if queue_inst else 100, 100))
    elif len(time_avg) > 0:
        ax1.set_xlim(0, max(time_avg))
        ax1.set_ylim(0, max(max(queue_avg) if queue_avg else 100, 100))
    
    # Plot 2: Queue Length Histogram
    if len(queue_inst) > 0:
        ax2.hist(queue_inst, bins=50, color='blue', alpha=0.7, edgecolor='black')
    elif len(queue_avg) > 0:
        ax2.hist(queue_avg, bins=50, color='red', alpha=0.7, edgecolor='black')
    
    ax2.axvline(x=20, color='green', linestyle='--', label='q_min (20)', linewidth=2)
    ax2.axvline(x=60, color='orange', linestyle='--', label='q_max (60)', linewidth=2)
    ax2.set_xlabel('Queue Length (packets)', fontsize=12)
    ax2.set_ylabel('Frequency', fontsize=12)
    ax2.set_title('Queue Length Distribution', fontsize=14, fontweight='bold')
    ax2.legend(loc='upper right', fontsize=10)
    ax2.grid(True, linestyle='--', alpha=0.7)
    
    plt.tight_layout()
    plt.savefig('queue_graph.pdf', format='pdf', bbox_inches='tight')
    plt.savefig('queue_graph.png', format='png', dpi=300, bbox_inches='tight')
    
    print("\nGraphs saved:")
    print("  - queue_graph.pdf (for LaTeX article)")
    print("  - queue_graph.png (for presentations)")
    
    # Print statistics
    print(f"\n=== Queue Statistics ===")
    if len(time_inst) > 0:
        print(f"Simulation Duration: {max(time_inst):.2f} seconds")
        print(f"Data points (instantaneous): {len(queue_inst)}")
        print(f"Instantaneous Queue - Min: {min(queue_inst):.2f}, Max: {max(queue_inst):.2f}, Avg: {np.mean(queue_inst):.2f}")
    if len(time_avg) > 0:
        if len(time_inst) == 0:
            print(f"Simulation Duration: {max(time_avg):.2f} seconds")
        print(f"Data points (average): {len(queue_avg)}")
        print(f"Average Queue - Min: {min(queue_avg):.2f}, Max: {max(queue_avg):.2f}, Avg: {np.mean(queue_avg):.2f}")
    print(f"Queue oscillates between q_min and q_max: {len(queue_inst) > 0 and (min(queue_inst) <= 20 and max(queue_inst) >= 60)}")
    print("="*40)
    
    plt.show()

def main():
    print("="*60)
    print("NS-2 RED Queue Dynamics Analysis")
    print("Based on Korolkova 2010, Velieva 2019 PhD Research (RUDN)")
    print("="*60)
    
    time_inst, queue_inst, time_avg, queue_avg = parse_ns2_trace()
    plot_queue_dynamics(time_inst, queue_inst, time_avg, queue_avg)
    print("\nAnalysis complete!")

if __name__ == "__main__":
    main()