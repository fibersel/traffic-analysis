import os
import pandas as pd
import matplotlib.pyplot as plt
import argparse


def load_data(prefix):
    return {
        "queue": pd.read_csv(f"{prefix}_queue.csv", header=None, names=["time", "queue"]),
        "drops": pd.read_csv(f"{prefix}_drops.csv", header=None, names=["time", "drop"]),
        "throughput": pd.read_csv(f"{prefix}_throughput.csv", header=None, names=["time", "mbps"])
    }


def smooth(df, column, window):
    return df[column].rolling(window=window).mean()


def main():
    parser = argparse.ArgumentParser(description="NS-3 AQM comparison visualizer")

    parser.add_argument("--algorithms", nargs="+", required=True,
                        help="List of algorithm prefixes (e.g. red ared gentle)")

    parser.add_argument("--window", type=int, default=30,
                        help="Smoothing window")

    args = parser.parse_args()

    data = {}

    for algo in args.algorithms:
        data[algo] = load_data(algo)

    script_dir = os.path.dirname(os.path.abspath(__file__))

    # ------------------------
    # QUEUE
    # ------------------------
    fig_queue = plt.figure()

    for algo, d in data.items():
        q = d["queue"]
        q["smooth"] = smooth(q, "queue", args.window)

        plt.plot(q["time"], q["smooth"], label=algo.upper())

    plt.xlabel("Time (s)")
    plt.ylabel("Queue size (packets)")
    plt.title("Queue Dynamics Comparison")
    plt.legend()
    plt.grid()
    fig_queue.savefig(os.path.join(script_dir, "queue.png"), dpi=150, bbox_inches="tight")

    # ------------------------
    # THROUGHPUT
    # ------------------------
    fig_throughput = plt.figure()

    for algo, d in data.items():
        t = d["throughput"]
        t["smooth"] = smooth(t, "mbps", args.window)

        plt.plot(t["time"], t["smooth"], label=algo.upper())

    plt.xlabel("Time (s)")
    plt.ylabel("Throughput (Mbps)")
    plt.title("Throughput Comparison")
    plt.legend()
    plt.grid()
    fig_throughput.savefig(os.path.join(script_dir, "throughput.png"), dpi=150, bbox_inches="tight")

    # ------------------------
    # DROPS
    # ------------------------
    fig_drops = plt.figure()

    for algo, d in data.items():
        dr = d["drops"]

        plt.scatter(dr["time"], [algo]*len(dr), s=5, label=algo.upper())

    plt.xlabel("Time (s)")
    plt.ylabel("Algorithm")
    plt.title("Packet Drops Comparison")
    plt.grid()
    fig_drops.savefig(os.path.join(script_dir, "drops.png"), dpi=150, bbox_inches="tight")

    plt.show()


if __name__ == "__main__":
    main()
