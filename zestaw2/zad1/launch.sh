#!/bin/bash
bytes=(4 512 4096 8192)

records=(32 512 1024)

for record in "${records[@]}"
do
	for byte in "${bytes[@]}"
	do	
		echo "Bytes: $byte, records: $record"
		./main sys generate res $record $byte >> result.txt
		cp res res2
		./main sys shuffle res2 $record $byte >> result.txt
		cp res res2
		./main sys sort res2 $record $byte >> result.txt
		cp res res2
		./main lib shuffle res2 $record $byte >> result.txt
		cp res res2
		./main lib sort res2 $record $byte >> result.txt
		
	done
done
`rm res`
`rm res2`
