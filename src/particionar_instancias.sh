#!/bin/sh

for instancia in myciel3 myciel4 myciel5 myciel6 queen5_5 queen6_6 queen7_7 queen8_8 queen8_12 queen9_9
do
	for k in 5 10 15 20
	do
		for i in $(seq 1 5)
		do
			# Genero 5 particiones distintas para cada instancia
			./particionador/particionar instancias/GSIA/$instancia.col instancias/GSIA/$instancia-$k.$i.col $k
		done
	done
done

