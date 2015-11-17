## randomTrips
http://sumo.dlr.de/wiki/Tools/Trip
python /media/sda4/prog/sumo-0.21.0/tools/trip/randomTrips.py --help
http://sumo.dlr.de/wiki/FAQ#How_do_I_generate_random_routes.3F

## Generate the Grid
netgenerate  -g --grid.number=5 --grid.length=250 --default.lanenumber=2 -o test.net.xml

## Generate the trips and the routes (with various distance)
python /media/sda4/prog/sumo-0.21.0/tools/trip/randomTrips.py -n test.net.xml --min-distance=10000 -b 0 -e 6000 -i 20 -r test.rou.xml

 #media/sda4/prog/sumo-0.21.0/bin/duarouter -n test.net.xml -t trips.trips.xml -o test.rou.xml  -b 0 -e 6000
