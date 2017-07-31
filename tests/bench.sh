UM=0
APP=$1
WORDS=$2
DICT=$3
APPREF=$4
DICTREF=$5

min_number()
{
    printf "%s\n" "$@" | sort -g | head -n1
}
for j in `seq 0 3`; do
  SUMOUR=0
  SUMTHEM=0
  echo distance $j
  for i in `seq 1 10`; do
      shuf $WORDS | cut -f1 | sed "s/.*/approx $j &/" > /tmp/queries.txt
      timeout 1 bash -c "cat /tmp/queries.txt | $APP $DICT" > us.txt
      timeout 1 bash -c "cat /tmp/queries.txt | $APPREF $DICTREF" > them.txt
      size_a=`wc -l us.txt | cut -d' ' -f1`
      size_b=`wc -l them.txt | cut -d' ' -f1`
      tmp_min=$((${size_a} < ${size_b} ? ${size_a} : ${size_b}))
      min=$(($tmp_min - 1))
      head -n ${min} us.txt > us$j$i.txt
      head -n ${min} them.txt > them$j$i.txt
      diff us$j$i.txt them$j$i.txt | head -n 10
      wc -l us.txt | cut -d' ' -f1> qpsour
      wc -l them.txt | cut -d' ' -f1> qpsthem
      echo our
      cat qpsour
      echo them
      cat qpsthem
      SUMOUR=$(($SUMOUR + $(cat qpsour)))
      SUMTHEM=$(($SUMTHEM + $(cat qpsthem)))
      rm us$j$i.txt
      rm them$j$i.txt
        done
        echo "our: $(($SUMOUR / 10))"
        echo "them: $(($SUMTHEM / 10))"
      rm qpsour
      rm qpsthem
      rm us.txt
      rm them.txt
  done
