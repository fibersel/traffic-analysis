#!/bin/zsh

NS3_DIR="$HOME/ns-3-dev"
EXP_DIR="$HOME/disser/traffic-analysis/red-experiments"
SCRATCH_PATH="scratch/red-experiments"
CONFIG_DIR="$EXP_DIR/configs"
ANALYSIS_DIR="$EXP_DIR/analysis"

CONFIGS=("red" "ared" "gentle")

echo "Running simulations..."

for cfg in "${CONFIGS[@]}"
do
    echo "---------------------------------"
    echo "Running $cfg"
    echo "---------------------------------"

    cd "$NS3_DIR" || exit

    # -------------------------
    # 1. запуск симуляции
    # -------------------------
    ./ns3 run "$SCRATCH_PATH/red.cc --config=$SCRATCH_PATH/configs/${cfg}.txt"

    # -------------------------
    # 2. перенос логов
    # -------------------------
    mv queue.csv "$ANALYSIS_DIR/${cfg}_queue.csv"
    mv drops.csv "$ANALYSIS_DIR/${cfg}_drops.csv"
    mv throughput.csv "$ANALYSIS_DIR/${cfg}_throughput.csv"

done

echo "All simulations finished."

# -------------------------
# визуализация
# -------------------------

echo "Launching visualization..."

cd "$ANALYSIS_DIR" || exit

~/venv/bin/python vis.py \
    --algorithms "${CONFIGS[@]}" \
    --window 30
