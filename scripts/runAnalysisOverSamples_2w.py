#!/usr/bin/env python
import os, sys
from os.path import isfile, getmtime, expanduser, expandvars
from time import time
import json
import optparse
import commands
import LaunchOnCondor
import UserCode.llvv_fwk.storeTools_cff as storeTools

PROXYDIR = "~/x509_user_proxy"
DatasetFileDB = "DAS"  #DEFAULT: will use das_client.py command line interface
#DatasetFileDB = "DBS" #OPTION:  will use curl to parse https GET request on DBSserver

def getFileList(procData, DefaultNFilesPerJob):
    FileList = []
    miniAODSamples = procData.get('miniAOD', '')
    dset = procData.get('dset', '')
    isMINIAODDataset = ("/MINIAOD" in dset) or  ("amagitte" in dset)
    if isMINIAODDataset or miniAODSamples :
        instance = "instance=prod/" + procData.get('dbsURL') if procData.get('dbsURL') else ''
        listSites = commands.getstatusoutput('das_client.py --query="site dataset='+dset + ' ' + instance + ' | grep site.name,site.dataset_fraction " --limit=0')[1]
        IsOnLocalTier = False
        for site in listSites.split('\n'):
            if localTier and localTier in site and '100.00%' in site :
               IsOnLocalTier=True
               print ("Sample is found to be on the local grid tier (%s): %s") %(localTier, site)
               break

        site_list = []
        if IsOnLocalTier or isMINIAODDataset:
           site_list = []
           if DatasetFileDB == "DAS":
              site_list = commands.getstatusoutput('das_client.py --query="file dataset='+dset + ' ' + instance + '" --limit=0')[1].split()
           elif DatasetFileDB == "DBS":
              curlCommand = "curl -ks --key $X509_USER_PROXY --cert $X509_USER_PROXY -X GET "
              dbsPath = "https://cmsweb.cern.ch/dbs/prod/global/DBSReader"
              sedTheList = ' | sed \"s#logical_file_name#\\nlogical_file_name#g\" | sed \"s#logical_file_name\': \'##g\" | sed \"s#\'}, {u\'##g\" | sed \"s#\'}]##g\" | grep store '
              site_list = commands.getstatusoutput(initialCommand + curlCommand+'"'+dbsPath+'/files?dataset='+dset+'"'+sedTheList)[1].split()

           site_list = [x for x in site_list if ".root" in x] #make sure that we only consider root files

           if IsOnLocalTier:
               if  "iihe.ac.be" in hostname: host = "dcap://maite.iihe.ac.be:/pnfs/iihe/cms/ph/sc4"
               elif "ucl.ac.be" in hostname: host = "/storage/data/cms"
               else: host = "root://eoscms//eos/cms"
           else: host = "root://cms-xrd-global.cern.ch/"
           site_list = [ host + i for i in site_list ]

        elif miniAODSamples:
           print "Processing private local sample: " + miniAODSamples
           site_list = storeTools.fillFromStore(miniAODSamples,0,-1,True)
        else:
           print "Processing an unknown type of sample (assuming it's a private local sample): " + miniAODSamples
           site_list = storeTools.fillFromStore(miniAODSamples,0,-1,True)

        site_list = storeTools.keepOnlyFilesFromGoodRun(site_list, procData.get('lumiMask', ''))
        split = procData.get('split', -1)
        if split > 0:
           NFilesPerJob = max(1,len(site_list)/split)
        else:
           NFilesPerJob = DefaultNFilesPerJob

        for g in range(0, len(site_list), NFilesPerJob):
           groupList = ''
           for f in site_list[g:g+NFilesPerJob]:
              groupList += '"'+f+'",\\n'
           FileList.append(groupList)

    else:
        # the next line always leads to an error:
        #     the variable segment is not defined at this point
        print "Processing a non EDM/miniAOD sample in : " + opt.indir + '/' + origdtag + '_' + str(segment) + '.root'
        for segment in range(0, split):
            eventsFile = opt.indir + '/' + origdtag + '_' + str(segment) + '.root'
            if eventsFile.find('/store/') == 0: eventsFile = commands.getstatusoutput('cmsPfn ' + eventsFile)[1]
            FileList.append('"'+eventsFile+'"')
    return FileList

#configure
usage = 'usage: %prog [options]'
parser = optparse.OptionParser(usage)
parser.add_option('-e', '--exe'        ,    dest='theExecutable'      , help='executable'                            , default='')
parser.add_option('-s', '--sub'        ,    dest='queue'              , help='batch queue OR "crab" to use crab3'    , default='8nh')
parser.add_option('-R', '--R'          ,    dest='requirementtoBatch' , help='requirement for batch queue'           , default='pool>30000')
parser.add_option('-j', '--json'       ,    dest='samplesDB'          , help='samples json file'                     , default='')
parser.add_option('-d', '--dir'        ,    dest='indir'              , help='input directory or tag in json file'   , default='aoddir')
parser.add_option('-o', '--out'        ,    dest='outdir'             , help='output directory'                      , default='')
parser.add_option('-t', '--tag'        ,    dest='onlytag'            , help='process only samples matching this tag', default='all')
parser.add_option('-p', '--pars'       ,    dest='params'             , help='extra parameters for the job'          , default='')
parser.add_option('-c', '--cfg'        ,    dest='cfg_file'           , help='base configuration file template'      , default='')
parser.add_option('-r', "--report"     ,    dest='report'             , help='If the report should be sent via email', default=False, action="store_true")
parser.add_option('-D', "--db"         ,    dest='db'                 , help='DB to get file list for a given dset'  , default=DatasetFileDB)
parser.add_option('-F', "--resubmit"   ,    dest='resubmit'           , help='resubmit jobs that failed'             , default=False, action="store_true")
parser.add_option('-S', "--NFile"      ,    dest='NFile'              , help='default #Files per job (for autosplit)', default=5)
parser.add_option('-f', "--localnfiles",    dest='localnfiles'        , help='number of parallel jobs to run locally', default=8)
parser.add_option('-l', "--lfn"        ,    dest='crablfn'            , help='user defined directory for CRAB runs'  , default='')

(opt, args) = parser.parse_args()
scriptFile = expandvars('${CMSSW_BASE}/bin/${SCRAM_ARCH}/wrapLocalAnalysisRun.sh')
DatasetFileDB                      = opt.db

FarmDirectory                      = opt.outdir+"/FARM"
PROXYDIR                           = FarmDirectory+"/inputs/"
JobName                            = opt.theExecutable
LaunchOnCondor.Jobs_RunHere        = 1
LaunchOnCondor.Jobs_Queue          = opt.queue
LaunchOnCondor.Jobs_LSFRequirement = '"'+opt.requirementtoBatch+'"'
LaunchOnCondor.Jobs_EmailReport    = opt.report
LaunchOnCondor.Jobs_InitCmds       = ['ulimit -c 0;']  #disable production of core dump in case of job crash
LaunchOnCondor.Jobs_InitCmds      += [initialCommand]
LaunchOnCondor.Jobs_LocalNJobs     = opt.localnfiles
LaunchOnCondor.Jobs_CRABLFN        = opt.crablfn
LaunchOnCondor.Jobs_ProxyDir       = FarmDirectory+"/inputs/" 

#define local site
localTier = ""
hostname = commands.getstatusoutput("hostname -f")[1]
if "ucl.ac.be" in hostname:  localTier = "T2_BE_UCL"
if "iihe.ac.be" in hostname: localTier = "T2_BE_IIHE"
if "cern.ch" in hostname:    localTier = "T2_CH_CERN"


initialCommand = ''
validCertificate = True
# --voms cms, otherwise it does not work normally
voms_proxy_filename = expanduser(PROXYDIR + '/x509_proxy')
voms_proxy_timeleft_cmd = '(export X509_USER_PROXY='+PROXYDIR+'/x509_proxy; voms-proxy-init --voms cms --noregen; voms-proxy-info -all) | grep timeleft | tail -n 1'
voms_proxy_timeleft  = int(commands.getstatusoutput(voms_proxy_timeleft_cmd)[1].split(':')[2])
# TODO: checked the voms_proxy_timeleft_cmd output:
#     $ voms-proxy-info -all | grep timeleft | tail -n 1
#     timeleft  : 06:53:23
# -- so we check minutes?
# >>> getstatusoutput('voms-proxy-info -all | grep timeleft | tail -n 1')[1].split(':')
# ['timeleft  ', ' 06', '49', '56']
if not isfile(voms_proxy_filename) or time() - getmtime(voms_proxy_filename) > 600 or voms_proxy_timeleft < 8:
    validCertificate = False
# TODO: now actually these two ifs can be merged
if not validCertificate:
    # then let's renew it
    print "You are going to run on a sample over grid using either CRAB or the AAA protocol, it is therefore needed to initialize your grid certificate"
    voms = 'cms:/cms/becms' if "iihe.ac.be" in hostname else 'cms'
    # calling the proxy init command:
    os.system('mkdir -p '+PROXYDIR+'; voms-proxy-init --voms %s  -valid 192:00 --out '+PROXYDIR+'/x509_proxy' % (voms))
    # TODO: use --out voms_proxy_filename instead?
initialCommand = 'export X509_USER_PROXY=' + PROXYDIR + '/x509_proxy;voms-proxy-init --voms cms --noregen; ' #no voms here, otherwise I (LQ) have issues

#open the file which describes the sample
jsonFile = open(opt.samplesDB,'r')
procList = json.load(jsonFile,encoding='utf-8')
#run over sample
for _, procBlock in procList.items():
  for proc in procBlock:
    if proc.get('interpollation'): continue #skip interpollated processes
    isdata = proc.get('isdata', False)
    mctruthmode = proc.get('mctruthmode', 0)
    data = proc['data']

    for procData in data :
        origdtag = procData.get('dtag')
        if not origdtag: continue
        dtag = origdtag
        xsec = procData.get('xsec', -1)
        br = procData.get('br', [])
        suffix = str(procData.get('suffix', ""))
        split = procData.get('split', -1)
        if opt.onlytag != 'all' and opt.onlytag not in dtag: continue
        filt = ''
        if mctruthmode != 0: filt = '_filt' + str(mctruthmode)      
        if xsec > 0 and not isdata:
            for ibr in br:  xsec = xsec*ibr
        
        # not sure what opt.resubmit can be, but probably it can be changed to if not opt.resubmit:
        if opt.resubmit == False:
            FileList = ['"' + procData.get('dset', 'UnknownDataset') + '"']
            LaunchOnCondor.SendCluster_Create(FarmDirectory, JobName + '_' + dtag)
            if LaunchOnCondor.subTool != 'crab': FileList = getFileList(procData, int(opt.NFile) )
           
            for s, eventsFile in enumerate(FileList):
                #create the cfg file
                eventsFile = eventsFile.replace('?svcClass=default', '')
                prodfilepath=opt.outdir +'/'+ dtag + suffix + '_' + str(s) + filt
                sedcmd = 'sed \''
                # maybe it is better to use _ or | as delimeter in sed's substitute
                # then one can do %s insert into the string
                sedcmd += 's%"@dtag"%"'     + dtag + '"%;'
                sedcmd += 's%"@input"%'     + eventsFile + '%;'
                sedcmd += 's%@outfile%'     + prodfilepath +'.root%;'
                sedcmd += 's%@isMC%'        + str(not isdata) + '%;'
                sedcmd += 's%@mctruthmode%' + str(mctruthmode) + '%;'
                sedcmd += 's%@xsec%'        + str(xsec) + '%;'
                sedcmd += 's%@cprime%'      + str(procData.get('cprime', -1)) + '%;'
                sedcmd += 's%@brnew%'       + str(procData.get('brnew',  -1)) + '%;'
                sedcmd += 's%@suffix%'      + suffix + '%;'
                sedcmd += 's%@lumiMask%"'   + procData.get('lumiMask', '') + '"%;'
                if '@useMVA' not in opt.params:          opt.params = '@useMVA=False ' + opt.params
                if '@weightsFile' not in opt.params:     opt.params = '@weightsFile= ' + opt.params
                if '@evStart' not in opt.params:         opt.params = '@evStart=0 '    + opt.params
                if '@evEnd' not in opt.params:           opt.params = '@evEnd=-1 '     + opt.params
                if '@saveSummaryTree' not in opt.params: opt.params = '@saveSummaryTree=False ' + opt.params
                if '@runSystematics' not in opt.params:  opt.params = '@runSystematics=False '  + opt.params
                if '@jacknife' not in opt.params:        opt.params = '@jacknife=-1 ' + opt.params
                if '@jacks' not in opt.params:           opt.params = '@jacks=-1 '    + opt.params
                if '@trig' not in opt.params:            opt.params = '@trig=False ' + opt.params
                if opt.params:
                    # if only pairs are valid then one could:
                    #for a, b in [for icfg in opt.params.split(' ') if len(icfg.split('=')) == 2]:
                    #for icfg in opt.params.split(' '):
                      #varopt = icfg.split('=')
                      #if len(varopt) < 2: continue
                      #sedcmd += 's%' + varopt[0] + '%' + varopt[1] + '%;'
                    valid_options = [icfg.split('=') for icfg in opt.params.split(' ') if len(icfg.split('=')) > 1]
                    for key, val in valid_options[:2]:
                      sedcmd += 's%' + key + '%' + val + '%;'
                sedcmd += '\''
                cfgfile = prodfilepath + '_cfg.py'
                os.system('cat ' + opt.cfg_file + ' | ' + sedcmd + ' > ' + cfgfile)
                
                #run the job
                if not opt.queue:
                    os.system(opt.theExecutable + ' ' + cfgfile)
                else:
                    if LaunchOnCondor.subTool == 'crab':
                       LaunchOnCondor.Jobs_CRABDataset  = FileList[0]
                       LaunchOnCondor.Jobs_CRABcfgFile  = cfgfile
                       LaunchOnCondor.Jobs_CRABexe      = opt.theExecutable
                       if commands.getstatusoutput("whoami")[1] in ('vischia', 'olek'):
                           LaunchOnCondor.Jobs_CRABStorageSite = 'T2_PT_NCG_Lisbon'
                       else:
                           LaunchOnCondor.Jobs_CRABStorageSite = 'T2_BE_UCL'
                       LaunchOnCondor.Jobs_CRABname  = dtag
                       LaunchOnCondor.Jobs_CRABInDBS = procData.get('dbsURL', 'global')
                       if split > 0:
                           LaunchOnCondor.Jobs_CRABUnitPerJob = 100 / split 
                       else:
                           LaunchOnCondor.Jobs_CRABUnitPerJob = int(opt.NFile)
                    LaunchOnCondor.SendCluster_Push(["BASH", initialCommand + str(opt.theExecutable + ' ' + cfgfile)])
           
            LaunchOnCondor.SendCluster_Submit()
        
        else:
            configList = commands.getstatusoutput('ls ' + opt.outdir +'/'+ dtag + suffix + '*_cfg.py')[1].split('\n')
            failedList = [ cfgfile for cfgfile in configList
                                   if not isfile(cfgfile.replace('_cfg.py','.root')) ]
            
            if failedList:
                LaunchOnCondor.SendCluster_Create(FarmDirectory, JobName + '_' + dtag)
                for cfgfile in failedList:                  
                   LaunchOnCondor.SendCluster_Push(["BASH", initialCommand + str(opt.theExecutable + ' ' + cfgfile)])
                LaunchOnCondor.SendCluster_Submit()


if LaunchOnCondor.subTool == 'criminal':
    LaunchOnCondor.SendCluster_CriminalSubmit()

