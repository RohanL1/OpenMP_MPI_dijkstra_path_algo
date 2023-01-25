#!/bin/bash
#
#SBATCH --job-name=pr3_10k
#SBATCH --partition=cpu
#SBATCH --cpus-per-task=8
#SBATCH --mem-per-cpu=3G
#SBATCH --nodes=1
#SBATCH --output='/WAVE/users/unix/rlagare/pr3 final/log/'dj_log-%j.out
#SBATCH --time=24:00:00
#SBATCH --mail-type=ALL
#SBATCH --mail-user=rlagare@scu.edu
#

export OMP_PLACES=cores
export OMP_PROC_BIND=spread

function banner() {
 [[ -d ${LOG_DIR} ]] || mkdir -p ${LOG_DIR}
 echo "Start : $( date )"
 echo "GRPH : ${GRPH}"
 echo "NSRCH : ${NSRCH}"
 echo "LOG_DIR : ${LOG_DIR}"
 echo "LOG : ${LOG}"
}

function clean_old_log() {
    if (( $(ls -1 ${LOG_DIR}./dj_*  | wc -l ) > 10 ))
    then
        echo "Cleaning old logs"
        ls -t1 dj_* | while read ln
        do
            rm $ln
        done
    fi
}



### MAIN ###

export GRPH_DIR='/WAVE/projects/COEN-319-Fa22/data/pr3/'
GRPH_TMP='SER.graph'
export GRPH=${GRPH_DIR}${GRPH_TMP}
export LOG_DIR=./log/
export SOL_LOG=${LOG_DIR}./dj_$(echo $GRPH_TMP | cut -d '.' -f 1 )_log_id_$$_$( date +%Y%m%d_%H%M%S_%N)
export RUN_LOG=${LOG_DIR}./dj_$(echo $GRPH_TMP | cut -d '.' -f 1 )_log_id_$$_$( date +%Y%m%d_%H%M%S_%N).log

[[ '' == $1 ]] && export NSRCH=1000 || NSRCH=${1}

banner
for GRPH_TMP in '1000.graph' '5000.graph' '10000.graph'
do
    export GRPH=${GRPH_DIR}${GRPH_TMP}
    #./dijkstra  ${GRPH} ${NSRCH} ${LOG}.data >> ${RUN_LOG} 2>&1
    echo "executing > ./dijkstra  ${GRPH} ${NSRCH} ${THD} ${SOL_LOG}_para.data >> ${RUN_LOG} 2>&1"
    ./dijkstra  ${GRPH} ${NSRCH} ${THD} ${SOL_LOG}_para.data >> ${RUN_LOG} 2>&1
done

#echo -n "MATCH : "
#[[ $(diff 1 1 | wc -l) == "0" ]] && echo "EQUAL" ||  echo "NOT EQUAL"
echo "FIN."
clean_old_log