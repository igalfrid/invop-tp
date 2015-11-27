#!/bin/sh

for nodos in 30 40 50
do
	mkdir -p instancias/solved/$nodos/
	for d in 10 25 50
	do
		for k in 5 10 15
		do
			for p in 0 1 2 10 15
			do
				./cut_and_branch/prog instancias/instancia-$nodos-$d-$k.col instancias/solved/$nodos/instancia-$nodos-$d-$k.col-$p.solved $p
			done
		done
	done
done
