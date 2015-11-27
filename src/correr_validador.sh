#!/bin/sh

for nodos in 30
do
	mkdir -p instancias/solved/$nodo/
	for d in 10 25 50
	do
		for k in 5 10 15
		do
			for p in 0 1 2 10 15
			do
				echo "Validando instancias/instancia-$nodos-$d-$k.col vs instancias/solved/$nodos/instancia-$nodos-$d-$k.col-$p.solved"
				./validador.py instancias/instancia-$nodos-$d-$k.col instancias/solved/$nodos/instancia-$nodos-$d-$k.col-$p.solved
			done
		done
	done
done
