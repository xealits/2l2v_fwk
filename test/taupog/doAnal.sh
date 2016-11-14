#!/bin/bash

# Main analysis script.

# 1: run the analysis (must merge submit script here)

JSONFILE=$CMSSW_BASE/src/UserCode/llvv_fwk/test/taupog/samples2016.json

QUEUE=batch
#QUEUE=crab
QUEUE=8nh

OUTDIR=$CMSSW_BASE/src/UserCode/llvv_fwk/test/taupog/poggami/

BASEWEBDIR=/eos/user/v/vischia/www/taufakes_new/

PILEUP=datapileup_2016


if [ "${1}" = "submit" ]; then
    # cleanup (comment it out if you have smaller jsons for running only on a few sets while the others are OK
    ### rm -r ${OUTDIR}
    # recreate
    mkdir -p ${OUTDIR}
    
    KEY=" --key subnottbar "

    if   [ "${2}" != "" ]; then
        KEY=" --key ${2} "
    else
        echo "Keep using the base json, i.e. ${JSONFILE}"
    fi
    
    LFN=" "
    if [ "${3}" = "--lfn" ]; then
	LFN=" --lfn ${4} "
    fi

    runAnalysisOverSamples.py -e runTauFakesStudy -j ${JSONFILE} -o ${OUTDIR} -d  /dummy/ -c $CMSSW_BASE/src/UserCode/llvv_fwk/test/runAnalysis_cfg.py.templ -p "@data_pileup="${PILEUP}" @useMVA=False @saveSummaryTree=False @runSystematics=False @automaticSwitch=False @is2011=False @jacknife=0 @jacks=0" ${LFN} -s ${QUEUE} ${KEY} --NFile 8
    
elif [ "${1}" = "lumi" ]; then
    rm ${OUTDIR}/qcd_lumi.json
    rm ${OUTDIR}/wjet_lumi.json
    #cat ${OUTDIR}/*JetHT*json > qcd_lumi.json
    #cat ${OUTDIR}/*SingleMuon*json > wjet_lumi.json

    mergeJSON.py --output=${OUTDIR}/qcd_lumi.json  ${OUTDIR}/Data13TeV_JetHT*.json
    mergeJSON.py --output=${OUTDIR}/wjet_lumi.json ${OUTDIR}/Data13TeV_SingleMu*.json
    GOLDENJSON=$CMSSW_BASE/src/UserCode/llvv_fwk/data/json/
    mergeJSON.py --output=${OUTDIR}/json_in.json  ${GOLDENJSON}/Cert_*.txt

    echo "--------------------- MISSING LUMIS FOR QCD ---------------------"
    compareJSON.py --diff ${OUTDIR}/json_in.json ${OUTDIR}/qcd_lumi.json 
    echo "--------------------- MISSING LUMIS FOR WJET ---------------------"
    compareJSON.py --diff ${OUTDIR}/json_in.json ${OUTDIR}/wjet_lumi.json 

    echo "COMPUTE INTEGRATED LUMINOSITY"
    export PATH=$HOME/.local/bin:/afs/cern.ch/cms/lumi/brilconda-1.0.3/bin:$PATH
    pip install --upgrade --install-option="--prefix=$HOME/.local" brilws &> /dev/null #will be installed only the first time
    brilcalc lumi -b "STABLE BEAMS" --normtag /afs/cern.ch/user/l/lumipro/public/normtag_file/normtag_DATACERT.json -i ${OUTDIR}/qcd_lumi.json -u /pb -o ${OUTDIR}/QCD_LUMI.txt 
    brilcalc lumi -b "STABLE BEAMS" --normtag /afs/cern.ch/user/l/lumipro/public/normtag_file/normtag_DATACERT.json -i ${OUTDIR}/wjet_lumi.json -u /pb -o ${OUTDIR}/WJET_LUMI.txt 

    echo "------------------------------------------------------------"
    echo "QCD"
    tail -n 4 ${OUTDIR}/QCD_LUMI.txt
    echo "------------------------------------------------------------"
    echo "WJET"
    tail -n 4 ${OUTDIR}/WJET_LUMI.txt


    
    ###STARTINGJSON="data/cert/Cert_246908-260627_13TeV_PromptReco_Collisions15_25ns_JSON.txt"
    ###sed -i -e "s#}{#,#g"  qcd_lumi.json; 
    ###sed -i -e "s#, ,#,#g" qcd_lumi.json;
    ###sed -i -e "s#,,#,#g"  qcd_lumi.json;
    ###sed -i -e "s#,,#,#g"  qcd_lumi.json;
    ###sed -i -e "s#,,#,#g"  qcd_lumi.json;
    ###sed -i -e "s#,,#,#g"  qcd_lumi.json;
    ###sed -i -e "s#,,#,#g"  qcd_lumi.json;
    ###sed -i -e "s#,,#,#g"  qcd_lumi.json;
    ###sed -i -e "s#{,#{#g"  qcd_lumi.json;
    ###sed -i -e "s#{,#{#g"  qcd_lumi.json;
    ###sed -i -e "s#{ ,#{#g"  qcd_lumi.json;
    ###
    ###sed -i -e "s#}{#,#g"  wjet_lumi.json; 
    ###sed -i -e "s#, ,#,#g" wjet_lumi.json;
    ###sed -i -e "s#,,#,#g"  wjet_lumi.json;
    ###sed -i -e "s#,,#,#g"  wjet_lumi.json;
    ###sed -i -e "s#,,#,#g"  wjet_lumi.json;
    ###sed -i -e "s#,,#,#g"  wjet_lumi.json;
    ###sed -i -e "s#,,#,#g"  wjet_lumi.json;
    ###sed -i -e "s#,,#,#g"  wjet_lumi.json;
    ###sed -i -e "s#{,#{#g"  wjet_lumi.json;
    ###sed -i -e "s#{,#{#g"  wjet_lumi.json;
    ###sed -i -e "s#{ ,#{#g"  wjet_lumi.json;
    ###
    ###export PATH=$HOME/.local/bin:/afs/cern.ch/cms/lumi/brilconda-1.0.3/bin:$PATH 
    ###### Official v1 brilcalc lumi --normtag /afs/cern.ch/user/c/cmsbril/public/normtag_json/OfflineNormtagV1.json -i myjson.json
    ###brilcalc lumi --normtag ~lumipro/public/normtag_file/OfflineNormtagV2.json  -i wjet_lumi.json
    ###echo "DONE WJET"
    ###brilcalc lumi --normtag ~lumipro/public/normtag_file/OfflineNormtagV2.json  -i qcd_lumi.json
    ###echo "DONE QCD"
    ###echo "Take care. This uses the offline tag V2, which is not yet blessed by Physics Coordinators https://hypernews.cern.ch/HyperNews/CMS/get/luminosity/544.html "
    ###echo "To be compared with the output of the full json:"
    ###echo "brilcalc lumi --normtag ${STARTINGJSON}"
    ###brilcalc lumi --normtag ~lumipro/public/normtag_file/OfflineNormtagV2.json -i ${STARTINGJSON}
    ###echo "DONE FULL"
    ###echo "Take care. This uses the offline tag V2, which is not yet blessed by Physics Coordinators https://hypernews.cern.ch/HyperNews/CMS/get/luminosity/544.html "
    ###
    ###exit 0
    ###JSONFILE=$CMSSW_BASE/src/TauAnalysis/JetToTauFakeRate/data/data_samples.json
    ###OUTDIR=$CMSSW_BASE/src/TauAnalysis/JetToTauFakeRate/test/results_lumi
    #### cleanup (comment it out if you have smaller jsons for running only on a few sets while the others are OK
    #### rm -r ${OUTDIR}
    #### recreate
    ###mkdir -p ${OUTDIR}
    ###runAnalysisOverSamples.py -e extractLumiJSON -j ${JSONFILE} -o ${OUTDIR} -d  /dummy/ -c $CMSSW_BASE/src/TauAnalysis/JetToTauFakeRate/test/runAnalysis_cfg.py.templ -p "@useMVA=False @saveSummaryTree=False @runSystematics=False @automaticSwitch=False @is2011=False @jacknife=0 @jacks=0" -s 8nh

elif [ "${1}" = "plot" ]; then
    DIR="${BASEWEBDIR}/"
    mkdir -p ${DIR}
    mkdir -p ~/www/temptemp/
    mv ${DIR}*vischia*pdf ~/www/temptemp/
    rm -r ${DIR}*
    mv ~/www/temptemp/* ${DIR}
    cp /eos/user/v/vischia/www/HIG-13-026/index.php ${DIR}


    RUNINBACKGROUND="&"
    RUNINBACKGROUND=""
    # Different tests
    LUMI=12900
    LUMIWJETS=10936.978
    LUMIQCD=12884.276
    #LUMIWJETS=307
    #LUMIQCD=277
    #LUMIWJETS=336
    #LUMIQCD=729
    #LUMIWJETS=365
    #LUMIQCD=380
    DEBUG=""
    DEBUG=" --debug "
    
    #PLOTTER=runFixedPlotter
    PLOTTER=runPlotter


    ONLYWJETS="--only (wjet_wjet_eventflow|    )"
    ONLYQCD="--only   (qcd_qcd_eventflow|      )"

    VARIABLES="pt|met|eta|radius|nvtx"
    WPS="Loose|Medium|Tight"
    DISCRIMINATORS="CombinedIsolationDeltaBetaCorr3Hits|IsolationMVArun2v1DBdR03oldDMwLT|IsolationMVArun2v1DBnewDMwLT|IsolationMVArun2v1DBoldDMwLT|IsolationMVArun2v1PWdR03oldDMwLT|IsolationMVArun2v1PWnewDMwLT|IsolationMVArun2v1PWoldDMwLT"
    
    ONLYWJETS=' --only "wjet_(step5|step6|step7|step8)(((by(${WPS})(${DISCRIMINATORS}))(${VARIABLES})_numerator)|(${VARIABLES})_denominator)" '
    ONLYQCD=' --only "qcd_step3(((by(${WPS})(${DISCRIMINATORS}))(${VARIABLES})_numerator)|(${VARIABLES})_denominator)" '
    
    
    #INDIR=$CMSSW_BASE/src/TauAnalysis/JetToTauFakeRate/test/results/
    INDIR=${OUTDIR}
    PLOTTERWJETS=${DIR}plotter_wjet.root
    PLOTTERQCD=${DIR}plotter_qcd.root
    ONLYWJETS="--only wjet_step6eta_denominator"
    ONLYQCD="--only qcd_step2eta_denominator"
    #ONLYWJETS=' --only "wjet_*" '
    #ONLYQCD=' --only "qcd_*" '
    #ONLYWJETS=' '
    #ONLYQCD=' '
    #ONLYWJETS="--only wjet_wjet_eventflow"
    #ONLYQCD="--only qcd_qcd_eventflow"
    PLOTEXT=" --plotExt .png --plotExt .pdf --plotExt .C "
    
    #MERGE="--forceMerge"
    #MERGE="--useMerged"
    MERGE=""

    ## Create plotter files from which the ratio for fake rate will be computed
    # WJets
    ${PLOTTER} --iEcm 13 ${MERGE} --iLumi ${LUMIWJETS} --inDir ${INDIR} --outDir ${DIR} --outFile ${PLOTTERWJETS} --json ${JSONFILE} --cutflow all_initNorm --no2D --noPowers ${PLOTEXT} ${ONLYWJETS} --key wjet ${DEBUG} ${RUNINBACKGROUND}
    #${PLOTTER} --iEcm 13 --debug --forceMerge --iLumi ${LUMIWJETS} --inDir ${INDIR} --outDir ${DIR} --outFile ${PLOTTERWJETS} --json ${JSONFILEWJETS} --cutflow all_initNorm --no2D --noPowers ${PLOTEXT} ${ONLYWJETS} ${RUNINBACKGROUND} 
    
    # QCD
    ${PLOTTER} --iEcm 13 ${MERGE} --iLumi ${LUMIQCD} --inDir ${INDIR} --outDir ${DIR} --outFile ${PLOTTERQCD}   --json ${JSONFILE}   --cutflow all_initNorm --no2D --noPowers ${PLOTEXT} ${ONLYQCD} --key qcd ${DEBUG} ${RUNINBACKGROUND}
    
    ### DIR="${BASEWEBDIR}_split/"
    ### PLOTTERWJETS=${DIR}plotter_wjet.root
    ### PLOTTERQCD=${DIR}plotter_qcd.root
    ### mkdir -p ${DIR}
    ### mkdir -p ~/www/temptemp/
    ### mv ${DIR}*vischia*pdf ~/www/temptemp/
    ### rm -r ${DIR}*
    ### mv ~/www/temptemp/* ${DIR}
    ### cp ~/www/HIG-13-026/index.php ${DIR}
    ### JSONFILEWJETS=$CMSSW_BASE/src/TauAnalysis/JetToTauFakeRate/data/wjets_ttsplit_samples.json
    ### JSONFILEQCD=$CMSSW_BASE/src/TauAnalysis/JetToTauFakeRate/data/qcd_ttsplit_samples.json
    ### 
    ### ## Create plotter files from which the ratio for fake rate will be computed
    ### # WJets
    ### ${PLOTTER} --iEcm 13 ${MERGE} --iLumi ${LUMIWJETS} --inDir ${INDIR} --outDir ${DIR} --outFile ${PLOTTERWJETS} --json ${JSONFILEWJETS} --cutflow all_initNorm --no2D --noPowers ${PLOTEXT} ${ONLYWJETS} ${RUNINBACKGROUND} 
    ### 
    ### # QCD
    ### ${PLOTTER} --iEcm 13 ${MERGE} --iLumi ${LUMIQCD} --inDir ${INDIR} --outDir ${DIR} --outFile ${PLOTTERQCD}   --json ${JSONFILEQCD}   --cutflow all_initNorm --no2D --noPowers ${PLOTEXT} ${ONLYQCD} ${RUNINBACKGROUND} 


 
    # Now run test/harvest.sh by hand please
    # root -l -b bin/macros/plotFR.C  and sh set.sh. Currently coding this step into runFakeRate.cc
    exit 0
    # Lessen the burden on the web browser
    for CHAN in qcd wjet
    do
        mkdir ${DIR}temp${CHAN}/
        mv ${DIR}${CHAN}* ${DIR}temp${CHAN}/
        mv ${DIR}temp${CHAN}/ ${DIR}${CHAN}/
        cp ~/www/HIG-13-026/index.php ${DIR}${CHAN}/
    done

elif [ "${1}" = "crab" ]; then
    COMMAND="${2}"
    # Evolve by fetching the list using for i in ls
    
    for CRABWORKINGDIR in `ls ${OUTDIR}/FARM/inputs/ | grep "crab_"`
    do
	crab ${COMMAND} -d ${OUTDIR}/FARM/inputs/${CRABWORKINGDIR}
    done

elif [ "${1}" = "merge" ]; then
    mkdir -p ${3}
    mergeEDMtuples.py --inDir /gstore/t3cms/store/user/vischia/${2}/ --outDir ${3} --json ${JSONFILE} --n 10

    
elif [ "${1}" = "harvest" ]; then
    # Fix. --plotExt does not really impact (extensions are multiple and hardcoded)
    # Configurable input directory
    COMPUTE="${2}"
    if [ "${COMPUTE}" = "" ]; then
        COMPUTE="all"
    fi
    runFakeRate --inDir ${BASEWEBDIR}/ --outDir fakerate --plotExt .png --debug --compute ${COMPUTE}
    #--debug

elif [ "${1}" = "runtests" ]; then
    root -l macros/ht/plotStuff.C
    root -l macros/qcdtest/plotStuff.C

fi

# Done!
exit 0

