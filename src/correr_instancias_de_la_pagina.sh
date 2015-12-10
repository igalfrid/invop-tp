#!/bin/sh

for instancia in queen5_5 queen6_6 queen7_7 queen8_8 queen8_12 queen9_9
do
	for k in 5 10
	do
		for algs in 19 11 8 3 0
		do
			for i in $(seq 1 5)
			do
				./cut_and_branch/prog instancias/GSIA/$instancia-$k.$i.col instancias/GSIA/solved/$instancia-$k.$i.col.$algs $algs
			done
		done
	done
done

for k in 15
do
	for algs in 19 11 8 3 0
	do
		for instancia in queen5_5 queen6_6 queen7_7 queen8_8 queen8_12 queen9_9
		do
			for i in $(seq 1 5)
			do
				./cut_and_branch/prog instancias/GSIA/$instancia-$k.$i.col instancias/GSIA/solved/$instancia-$k.$i.col.$algs $algs
			done
		done
	done
done

for k in 20
do
	for algs in 19 11 8 3 0
	do
		for instancia in queen5_5 queen6_6 queen7_7 queen8_8 queen8_12 queen9_9
		do
			for i in $(seq 1 5)
			do
				./cut_and_branch/prog instancias/GSIA/$instancia-$k.$i.col instancias/GSIA/solved/$instancia-$k.$i.col.$algs $algs
			done
		done
	done
done
