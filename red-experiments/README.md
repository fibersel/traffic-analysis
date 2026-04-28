# Red Experiments

Instructions for running this NS-3 simulation.

## Installing NS-3

### Prerequisites

**macOS**
```bash
brew install cmake python3 ninja git
```

**Ubuntu / Debian**
```bash
sudo apt install g++ cmake python3 python3-dev ninja-build git
```

### Clone and build

The simulation targets **ns-3-dev**. Clone it to `~/ns-3-dev`:

```bash
git clone https://gitlab.com/nsnam/ns-3-dev.git ~/ns-3-dev
cd ~/ns-3-dev
```

Configure and build (optimized build is faster for long simulations):

```bash
./ns3 configure --enable-examples --enable-tests
./ns3 build
```

For a release build (recommended when running many experiments):

```bash
./ns3 configure --build-profile=optimized
./ns3 build
```

Build takes several minutes on first run. Subsequent builds are incremental.

### Python visualization dependencies

```bash
pip install pandas matplotlib
```

The `run_simulations.sh` script expects Python at `~/venv/bin/python`. Create the venv if needed:

```bash
python3 -m venv ~/venv
~/venv/bin/pip install pandas matplotlib
```

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
