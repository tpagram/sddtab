# Example commands:
# $SHELL benchmarks/benchmark2_size.sh bin/sddtab "-size" benchmarks/k
# $SHELL benchmarks/benchmark2_size.sh bin/sddtab "-size" benchmarks/s4
# $SHELL benchmarks/benchmark2_size.sh **path to bddtab** "" benchmarks/k
# $SHELL benchmarks/benchmark2_size.sh **path to bddtab** "" benchmarks/s4

# bddtab will need to be edited in order to print its size. simply add the line:
# 	std::cout << bdd_nodecount(notpsiAndGammaBDD) << std::endl; return 0;
# after the line:
# 	bdd notpsiAndGammaBDD = toBDD(notpsiNNF) & gammaBDD;
# found within bddtab.cpp

SOLVER=$1
SOLVER_ARGS=$2
LOGIC_DIR=$3

for BENCH_DIR in $LOGIC_DIR/*
do
	if [[ -d $BENCH_DIR ]]
	then
		AV=0
		for i in {1..20}
		do			
			RES_STRING=$(gtimeout 30s $SHELL -c "cat $BENCH_DIR/$i.txt.k | $SOLVER $SOLVER_ARGS")
			STAT=$?
			if [[ $STAT == 0 ]]
			then
				AV=$(($AV + $(echo $RES_STRING | head -1)))
			elif [[ $STAT == 124 ]]
			then
				echo "$BENCH_DIR $i out of time"
				break
			else
				echo "$BENCH_DIR $i error"
				break
			fi
		done
		printf "%-20s %0.2f\n" $(basename $BENCH_DIR) $(($AV/20.0))
	fi
done