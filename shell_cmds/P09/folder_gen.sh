max=$1
i=1

while [ $i -le $max ]
do
    if [ $i -lt 10 ]
    then
        mkdir prob0$i
    else
        mkdir prob$i
    fi
    i=$((i+1))
done