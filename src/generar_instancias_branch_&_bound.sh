#!/bin/sh

for nodos in 20
do
	for d in 25 50 75
	do
		for k in 5 8 10
		do
			for i in $(seq 1 5)
			do
				# Genero 10 instancias distintas
				./generador/generador $nodos 0.$d instancias/branching/instancia-$nodos-$d-$k.$i.col
				
				for j in $(seq 1 5)
				do
 					# Genero 10 particiones distintas para cada instancia
					./particionador/particionar instancias/branching/instancia-$nodos-$d-$k.$i.col instancias/branching/instancia-$nodos-$d-$k.$i.$j.col $k
				done
			done
		done
	done
done
