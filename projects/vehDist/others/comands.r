## randomTrips
    http://sumo.dlr.de/wiki/Tools/Trip
    python /media/sda4/prog/sumo-0.25.0/tools/randomTrips.py --help
    http://sumo.dlr.de/wiki/FAQ#How_do_I_generate_random_routes.3F

## Generate the Grid
    netgenerate -g --grid.number=5 --grid.length=250 --default.lanenumber=1 --default.speed 85 -o vehDist.net.xml
        # netgenerate -g --grid.number=5 --grid.length=250 --default.lanenumber=1 --no-turnarounds --default.speed 85 -o vehDist.net.xml
        # Change --default.lanenumber to 1; added --no-turnarounds and the (max) speed to 85

## Generate the trips and the routes (with various distance)
    python /media/sda4/prog/sumo-0.25.0/tools/randomTrips.py -n vehDist.net.xml --min-distance=100000 -b 0 -e 30000 -i 200 -s 1 -r vehDist_tmp.rou.xml
        # Change added -s (seed) 1, for the same routes every time.

## To test "is the same routes?"
    cat vehDist.rou.xml | grep -v generated | md5sum

## Want TLS?
    # http://sumo.dlr.de/wiki/NETCONVERT
    netgenerate -g --grid.number=5 --grid.length=250 --default.lanenumber=1 --default.speed 85 -o vehDist.net.xml --tls.set "0/0, 0/1, 0/2, 0/3, 0/4, 1/0, 1/1, 1/2, 1/3, 1/4, 2/0, 2/1, 2/2, 2/3, 2/4, 3/0, 3/1, 3/2, 3/3, 3/4, 4/0, 4/1, 4/2, 4/3, 4/4"

## Test same start position
    cat [folder]/vehicle_position_initialize.r | grep -v number  | md5sum

## Change maxSpeed
    sed -i 's/maxSpeed=.* color/maxSpeed=\"15\" color/g' test_end.rou.xml
    sed -i 's/maxSpeed=.* color/maxSpeed=\"25\" color/g' test_end.rou.xml

## See iterations from ini file
    # cat General-*.sca | grep "attr iterationvars2"
    i=0; while [ $i -lt 32 ]; do cat General-$i.sca | grep "attr iterationvars2"; i=$((i+1)); done

## Run on terminal
    -u = mode, -f = arquivo ini -r = nº de execucão -l = libveins -n = arquivos ned
    # sem GUI
        opp_run -u Cmdenv -n .:../../src/ -f vehDist.ini -r 0 -l ../../src/libveins.so
    # com GUI
        opp_run -u Tkenv -n .:../../src/ -f vehDist.ini -r 0 -l ../../src/libveins.so
    # sem repeat
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

## See if the selected vehicle to generate messages are the same
sublime_text E1*/Veh_Messages_Generated.r E3*/Veh_Messages_Generated.r E5*/Veh_Messages_Generated.r E7*/Veh_Messages_Generated.r

sublime_text E2*/Veh_Messages_Generated.r E4*/Veh_Messages_Generated.r E6*/Veh_Messages_Generated.r E8*/Veh_Messages_Generated.r

sublime_text E*/Veh_Position_Initialize.r

## Split results in experiments numbers
v=v001; mkdir $v; i=1; while [ $i -lt 9 ]; do echo Experiment $i $v; cat output_vehDist_$v\_results.r | grep -E "Exp: $i|experiment" > $v/exp$i.r; ((i+=1)); done

## Get the count message received by the "split" experiment
i=1; cat exp$i.r | grep -E "Count|experiment" | sed 's/Exp: '$i' ### Count Messages Received://g'

## Get the count message received by the full experiment file
i=1; cat file.r | grep -E "Exp: $i|Values" |  grep -E "Count Messages Received|Values" | sed 's/Exp: '$i' ### Count Messages Received://g'
#or
i=1; f=1; part=1; ./grep_results.sh $part $i $f | grep -E "Exp: $i|Values" |  grep -E "Count|Values" | sed 's/Exp: '$i' ### Count Messages Received://g'

## Get the count message dropped by the full experiment file
i=1; cat file.r | grep -E "Exp: $i|Values" |  grep -E "drop:|Values" | sed 's/Exp: '$i' ### Final count messages drop://g'

## Get the count message packets send by the full experiment file
i=1; cat file.r | grep -E "Exp: $i|Values" | grep -E "send|Values" | sed 's/Exp: '$i' ### Final count packets messages send://g'

#