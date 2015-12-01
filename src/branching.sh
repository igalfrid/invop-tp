#!/bin/sh

for nodos in 20
do
	mkdir -p instancias/branching/solved/
	for densidad in 25 50 75
	do
		for particiones in 5 8 10
		do
			for i in $(seq 1 5)
			do
				for j in $(seq 1 5)
				do
					# dejo fijo varSel en el default 0, y muevo nodeSel
					for nodeSel in 0 1 2 3
					do
						./cut_and_branch/prog instancias/branching/instancia-$nodos-$densidad-$particiones.$i.$j.col instancias/branching/solved/instancia-$nodos-$densidad-$particiones.$i.$j.0.$nodeSel.col.solved 0 0 $nodeSel
					done

					# dejo fijo nodeSel en el default 0, y muevo varSel
					for varSel in -1 0 1 2 3 4
					do
						./cut_and_branch/prog instancias/branching/instancia-$nodos-$densidad-$particiones.$i.$j.col instancias/branching/solved/instancia-$nodos-$densidad-$particiones.$i.$j.$varSel.0.col.solved 0 $varSel 0
					done
				done
			done
		done
	done
done
