#!/bin/sh

for densidad in 10 25 50 75 90
do
	for particiones in 5 10 15 20
	do
		for algs in 19 11 8 3 0
		do
			for i in $(seq 1 5)
			do
				for j in $(seq 1 5)
				do
					for nodos in 30 40 50
					do
						./cut_and_branch/prog instancias/instancia-$nodos-$densidad-$particiones.$i.$j.col instancias/solved/instancia-$nodos-$densidad-$particiones.$i.$j.col.$algs $algs
						
					done
				done
			done
		done
	done
done
