#!/bin/sh

for nodos in 30 40 50 60
do
	for d in 10 25 50 75 90
	do
		for k in 5 10 15 20 25
		do
			./generador/generador $nodos 0.$d $k instancias/instancia-$nodos-$d-$k.col
		done
	done
done
