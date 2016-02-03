## randomTrips
http://sumo.dlr.de/wiki/Tools/Trip
python /media/sda4/prog/sumo-0.21.0/tools/trip/randomTrips.py --help
http://sumo.dlr.de/wiki/FAQ#How_do_I_generate_random_routes.3F

## Generate the Grid
netgenerate  -g --grid.number=5 --grid.length=250 --default.lanenumber=1 --no-turnarounds --default.speed 15 -o test.net.xml
# Change --default.lanenumber to 1; added --no-turnarounds and the default.speed to 15

## Generate the trips and the routes (with various distance)
python /media/sda4/prog/sumo-0.21.0/tools/trip/randomTrips.py -n test.net.xml --min-distance=100000 -b 0 -e 30000 -i 200 -s 1 -r test.rou.xml
# Change added -s (seed) 1, for the same routes everytime.

To test "is the same routes?"
cat test.rou.xml | grep -v generated | md5sum
f649688c8178367ce3f5dade28de4f3b  -