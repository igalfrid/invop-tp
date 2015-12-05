#!/bin/sh

for nodos in 30 40 50
do
	for d in 10 25 50 75 90
	do
		for k in 5 10 15 20
		do
			for i in $(seq 1 5)
			do
				# Genero 5 instancias distintas
				./generador/generador $nodos 0.$d instancias/instancia-$nodos-$d-$k.$i.col
				
				for j in $(seq 1 5)
				do
 					# Genero 5 particiones distintas para cada instancia
					./particionador/particionar instancias/instancia-$nodos-$d-$k.$i.col instancias/instancia-$nodos-$d-$k.$i.$j.col $k
				done
			done
		done
	done
done
