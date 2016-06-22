max=50
for (( i=1; i <= 9; i++ ));
do
  echo "GREEN0000000000000000000000000$i" > /dev/scullpipe0
done
for (( i=10; i <= max; i++ ));
do
  echo "GREEN000000000000000000000000$i" > /dev/scullpipe0
done
