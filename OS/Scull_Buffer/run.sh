make clean
make
sudo ./scull_unload
sudo ./scull_load
ls -ltr /dev/scull*
sudo chmod 777 /dev/scull*
