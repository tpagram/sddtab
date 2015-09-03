# Example commands:
# zsh benchmarks/benchmark2.sh ./bin/sddtab "-k" benchmarks/k benchmarks/output.txt 21 30
# zsh benchmarks/benchmark2.sh ./bin/sddtab "-s4" benchmarks/s4 benchmarks/output.txt 21 30

SOLVER=$1
SOLVER_ARGS=$2
LOGIC_DIR=$3
OUTPUT=$4
MAX=$5
TIMEOUT=$6

for BENCH_DIR in $LOGIC_DIR/*
do
	if [[ -d $BENCH_DIR ]] #&& $(basename $BENCH_DIR) == "s4_t4p_n" ]]
	then
		MAX_FIN=0
		ERROR=0
		for i in {1..$MAX}
		do
			RES_STRING=$(gtimeout "$TIMEOUT"s zsh -c "cat $BENCH_DIR/$i.txt.k | $SOLVER $SOLVER_ARGS")
			STAT=$?
			if [[ $STAT == 0 ]]
			then
				echo "$BENCH_DIR $i complete"
				MAX_FIN=$((MAX_FIN+1))
				continue
			elif [[ $STAT == 124 ]]
			then
				echo "$BENCH_DIR $i out of time"
				break
			else
				echo "$BENCH_DIR $i error"
				ERROR=1
				continue
			fi
		done
		BENCH_NAME=$(echo "$BENCH_DIR" | sed "s:$LOGIC_DIR/::")
		if [[ $ERROR == 1 ]]
		then
				echo "$BENCH_NAME error" >> $OUTPUT
		else
				echo "$BENCH_NAME $MAX_FIN" >> $OUTPUT
		fi

	fi
done