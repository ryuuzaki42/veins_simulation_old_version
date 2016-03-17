## randomTrips
    http://sumo.dlr.de/wiki/Tools/Trip
    python /media/sda4/prog/sumo-0.21.0/tools/trip/randomTrips.py --help
    http://sumo.dlr.de/wiki/FAQ#How_do_I_generate_random_routes.3F

## Generate the Grid
    netgenerate -g --grid.number=5 --grid.length=250 --default.lanenumber=1 --default.speed 85 -o test.net.xml
        #netgenerate -g --grid.number=5 --grid.length=250 --default.lanenumber=1 --no-turnarounds --default.speed 85 -o test.net.xml
        # Change --default.lanenumber to 1; added --no-turnarounds and the (max) speed to 85

## Generate the trips and the routes (with various distance)
    python /media/sda4/prog/sumo-0.21.0/tools/trip/randomTrips.py -n test.net.xml --min-distance=100000 -b 0 -e 30000 -i 200 -s 1 -r test.rou.xml
        # Change added -s (seed) 1, for the same routes every time.

## To test "is the same routes?"
    cat test.rou.xml | grep -v generated | md5sum

    f649688c8178367ce3f5dade28de4f3b  - with --no-turnarounds
    026a7f8c52c9f5c6e613ccd82e00d831  - without --no-turnarounds

## Test same start position
    cat [folder]/vehicle_position_initialize.r | grep -v number  | md5sum

## Change maxSpeed
    sed -i 's/maxSpeed=".."/maxSpeed="15"/g' test_end.rou.xml
    sed -i 's/maxSpeed=".."/maxSpeed="25"/g' test_end.rou.xml

## See iterations from ini file
    #cat General-*.sca | grep "attr iterationvars2"
    i=0; while [ $i -lt 32 ]; do cat General-$i.sca | grep "attr iterationvars2"; i=$((i+1)); done

## Run on terminal
    -u = mode, -f = arquivo ini -r = nº de execucão -l = libveins -n = arquivos ned
    # sem GUI
        opp_run -u Cmdenv -n .:../../src/ -f vehDist.ini -r 0 -l ../../src/libveins.so
    # com GUI
        opp_run -u Tkenv -n .:../../src/ -f vehDist.ini -r 0 -l ../../src/libveins.so

    #sem repeat
        opp_run -u Cmdenv -n .:../../src/ -f vehDist.ini -l ../../src/libveins.so
        opp_run -u Tkenv -n .:../../src/ -f vehDist.ini -l ../../src/libveins.so

## Script in R ##
## Execute R < script_r.r or source('script_r.r', echo=TRUE)

    calculeDistance <- function(){
        ## copy in a=c("values") and b=c("values")
        a=c(1030.0,1300.0,0)
        b=c(520,520,3)
        sqrt((a[1]-b[1])^2 + (a[2] - b[2])^2)
    }

    calculeDistance()

## End