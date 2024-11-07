#!/bin/bash

# 定义参数范围
snd_dest_fn_cfg=(0 1 2 3)
snd_freq_fn_cfg=(0 1)
rcv_freq_fn_cfg=(0 1)

# 外层循环，遍历所有参数组合
for SND_DEST_FN in "${snd_dest_fn_cfg[@]}"; do
    for SND_FREQ_FN in "${snd_freq_fn_cfg[@]}"; do
        for RCV_FREQ_FN in "${rcv_freq_fn_cfg[@]}"; do
            
             make SndDestFn_CFG="$SND_DEST_FN" SndFreqFn_CFG="$SND_FREQ_FN" RcvFreqFn_CFG="$RCV_FREQ_FN"

            # 当前组合
            combination="snd_dest_${SND_DEST_FN}_snd_freq_${SND_FREQ_FN}_rcv_freq_${RCV_FREQ_FN}"


            # 内层循环20次
            for ((j=1; j<=20; j++)); do
                # 生成随机种子
                RANDOM_SEED=$((RANDOM))

                # 创建文件夹路径
                OUTPUT_DIR="out/${combination}_${j}"

                # 创建输出文件夹
                mkdir -p "$OUTPUT_DIR"

                # 运行make命令

                # 运行测试命令并保存到输出文件夹
                DUMP="${OUTPUT_DIR}/dump"  ./obj_dir/Vtest_bsg "$RANDOM_SEED" +max_core_cycles=100000 > test.log
                
                echo "Completed run ${combination}_${j} with Seed=${RANDOM_SEED}"
            done
        done
    done
done
