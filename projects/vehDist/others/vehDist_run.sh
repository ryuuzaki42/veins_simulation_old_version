
run_end=$1
if [ "$run_end" == '' ]; then
    run_end=1
fi

run=0
runFile=0
repeat=9

while [ $run -lt $run_end ]; do
    rm vehDist.rou.xml

    if [ $runFile -lt 10 ]; then
        cp t/vehDist.rou0$runFile.xml .
        mv vehDist.rou0$runFile.xml vehDist.rou.xml
    else
        cp t/vehDist.rou$runFile.xml .
        mv vehDist.rou$runFile.xml vehDist.rou.xml
    fi

    opp_run -r $run -n ../../src/veins/ -u Cmdenv -l ../../out/gcc-debug/src/libveins.so vehDist.ini >> run.r #run_$run.r 

    ((run++))
    ((runFile++))

    if [ $run == $repeat ]; then
        runFile=0;
    fi
done

exit 0
#