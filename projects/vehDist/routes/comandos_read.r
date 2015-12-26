## randomTrips
http://sumo.dlr.de/wiki/Tools/Trip
python /media/sda4/prog/sumo-0.21.0/tools/trip/randomTrips.py --help
http://sumo.dlr.de/wiki/FAQ#How_do_I_generate_random_routes.3F

g++ -std=c++0x -o script_conversor script_conversor.cc ^C

## Generate the Grid
netgenerate  -g --grid.number=5 --grid.length=250 --default.lanenumber=2 -o test.net.xml

## Generate the trips and the routes (with various distance)
python /media/sda4/prog/sumo-0.21.0/tools/trip/randomTrips.py -n test.net.xml --min-distance=100000 -b 0 -e 30000 -i 200 -r test.rou.xml