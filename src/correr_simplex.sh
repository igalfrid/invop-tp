#!/bin/sh

for nodos in 30 40 50 60
do
	mkdir -p instancias/solved/$nodo/
	for d in 10 25 50 75 90
	do
		for k in 5 10 15 20 25
		do
			for p in 0
			do
				./cut_and_branch/prog instancias/instancia-$nodos-$d-$k.col instancias/solved/$nodo/instancia-$nodos-$d-$k.col-$p.solved $p
			done
		done
	done
done
