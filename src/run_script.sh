para1_AddErr=("0.1" "0.01" "0.001" "0.0001" "1e-05" "1e-06")
para1_RelaErr=("0.001" "0.0001" "1e-05" "1e-06")
para2_arr=(0.2)

data_arr=("dblp-author" "indochina-2004" "orkut-links" "it-2004" "twitter-2010")
#data_arr=("toy")

rm STPPR
make
echo "=== Approximately queries for single-target PPR (STPPR) ==="
for data_name in "${data_arr[@]}"
do	
	for((j=0;j<${#para2_arr[@]};j++))
	do
		echo "./STPPR -d ./ -f ${data_name} -algo powermethod -qn 10 -a ${para2_arr[$j]}" |bash;
		echo "=== STPPR approximations by RBS with additive error==="
		for((i=0;i<${#para1_AddErr[@]};i++))
		do
			echo "./STPPR -d ./ -f ${data_name} -algo RBS -e ${para1_arr[$i]} -qn 10 -a ${para2_arr[$j]}" -type 1 |bash;
		done

		echo "=== STPPR approximations by RBS with relative error==="
		for((i=0;i<${#para1_RelaErr[@]};i++))
		do
			echo "./STPPR -d ./ -f ${data_name} -algo RBS -e ${para1_arr[$i]} -qn 10 -a ${para2_arr[$j]}" -type 2 |bash;
		done
	done
done



