max=50
for (( i=1; i <= 9; i++ ));
do
  echo "BLACK0000000000000000000000000$i" > /dev/scullpipe0
done
for (( i=10; i <= max; i++ ));
do
  echo "BLACK000000000000000000000000$i" > /dev/scullpipe0
done
