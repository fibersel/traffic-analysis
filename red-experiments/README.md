# Red Experiments

Instructions for running this NS-3 simulation.

## Setup

Create a symlink from your NS-3 scratch directory to this project:

```bash
ln -s ~/traffic-analysis/red-experiments scratch/red-experiments
```

## Running

To run the simulation in NS-3:

```bash
./ns3 run scratch/red-experiments/red
```

## Parameters

### AQM Type (`--aqm`)

The simulation supports different Active Queue Management (AQM) algorithms:

- RED (default): Random Early Detection
- ARED: Adaptive RED
- GENTLE: Gentle RED variant

### Examples

Run with ARED:
```bash
./ns3 run "scratch/red-experiments/red --aqm=ARED"
```

Run with Gentle RED:
```bash
./ns3 run "scratch/red-experiments/red --aqm=GENTLE"
```

Run with standard RED (default):
```bash
./ns3 run "scratch/red-experiments/red --aqm=RED"
```
