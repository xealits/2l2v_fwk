import FWCore.ParameterSet.Config as cms

process = cms.Process("AnalysisProc")
process.load("FWCore.MessageService.MessageLogger_cfi")
process.MessageLogger.cerr.FwkReport.reportEvery = 10000

import PhysicsTools.PythonAnalysis.LumiList as LumiList
LumiList.LumiList().getVLuminosityBlockRange()

process.source = cms.Source("PoolSource", fileNames =  cms.untracked.vstring('') )
from RecoJets.JetProducers.PileupJetIDParams_cfi import cutbased as pu_jetid

###### Electron VID
from RecoEgamma.ElectronIdentification.Identification.cutBasedElectronID_Spring15_25ns_V1_cff import *

if hasattr(cutBasedElectronID_Spring15_25ns_V1_standalone_loose,'isPOGApproved'):
    del cutBasedElectronID_Spring15_25ns_V1_standalone_loose.isPOGApproved
if hasattr(cutBasedElectronID_Spring15_25ns_V1_standalone_medium,'isPOGApproved'):
    del cutBasedElectronID_Spring15_25ns_V1_standalone_medium.isPOGApproved
if hasattr(cutBasedElectronID_Spring15_25ns_V1_standalone_tight,'isPOGApproved'):
    del cutBasedElectronID_Spring15_25ns_V1_standalone_tight.isPOGApproved

myVidElectronId = cms.PSet(
    loose = cutBasedElectronID_Spring15_25ns_V1_standalone_loose,
    medium = cutBasedElectronID_Spring15_25ns_V1_standalone_medium,
    tight = cutBasedElectronID_Spring15_25ns_V1_standalone_tight
)
#######

#from UserCode.llvv_fwk.mvaConfig_cfi import ewkzp2jFullNoQG as mySignalMVA
from UserCode.llvv_fwk.mvaConfig_cfi import ewkzp2jFull as mySignalMVA
#from UserCode.llvv_fwk.mvaConfig_cfi import ewkzp2jBase as mySignalMVA

datapileup_70300_2012abcd=cms.vdouble(8.09479e-06,5.64253e-05,0.000234438,0.000708129,0.0017105,0.00349358,0.00625553,0.0100708,0.0148505,0.020349,0.0262113,0.0320441,0.0374847,0.0422506,0.0461603,0.0491294,0.0511502,0.0522664,0.0525496,0.0520821,0.0509474,0.0492268,0.0469996,0.0443453,0.0413455,0.0380853,0.0346522,0.0311346,0.0276184,0.0241833,0.0208995,0.0178247,0.0150024,0.0124611,0.0102148,0.00826464,0.00660065,0.00520452,0.00405205,0.00311564,0.00236635,0.00177563,0.00131662,0.000964921,0.000699098,0.000500836,0.000354862,0.00024873,0.000172505,0.00011841,8.04614e-05,5.41399e-05,3.60823e-05,2.38255e-05,1.55917e-05,1.01156e-05,6.50871e-06,4.15502e-06,2.63279e-06,1.65666e-06,1.03577e-06,6.43829e-07,3.98156e-07,2.45158e-07,1.50427e-07,9.20683e-08,5.62672e-08,3.43759e-08,2.10198e-08,1.28799e-08,7.91828e-09,4.88962e-09,3.03584e-09,1.89664e-09,1.19296e-09,7.55627e-10,4.81954e-10,3.09438e-10,1.99876e-10,1.2979e-10,8.46506e-11,5.54024e-11,3.63523e-11,2.38919e-11,1.57154e-11,1.0338e-11,6.79661e-12,4.46326e-12,2.9262e-12,1.91457e-12,1.24969e-12,8.13541e-13,5.2808e-13,3.41728e-13,2.20422e-13,1.417e-13,9.07784e-14,5.79506e-14,3.68613e-14,2.33614e-14)

datapileup_70300_2012abcd_singleMu=cms.vdouble( 1.74887e-05, 8.41971e-05, 0.000287341, 0.000803268, 0.00187968, 0.00377078, 0.0066576, 0.0105832, 0.0154249, 0.0209121, 0.0266813, 0.0323481, 0.0375723, 0.0421003, 0.0457791, 0.0485477, 0.0504145, 0.0514311, 0.0516703, 0.0512103, 0.0501269, 0.0484915, 0.046373, 0.0438405, 0.0409668, 0.0378292, 0.0345099, 0.0310934, 0.0276634, 0.0242989, 0.0210703, 0.0180364, 0.015242, 0.0127175, 0.0104787, 0.00852824, 0.00685803, 0.00545122, 0.00428495, 0.00333277, 0.00256669, 0.00195893, 0.00148318, 0.00111545, 0.00083458, 0.000622389, 0.000463664, 0.000345947, 0.000259249, 0.000195714, 0.000149271, 0.000115306, 9.03708e-05, 7.19217e-05, 5.81115e-05, 4.76144e-05, 3.94894e-05, 3.30754e-05, 2.79112e-05, 2.36762e-05, 2.01478e-05, 1.71707e-05, 1.4635e-05, 1.24614e-05, 1.05913e-05, 8.97977e-06, 7.59119e-06, 6.39635e-06, 5.37056e-06, 4.49252e-06, 3.74351e-06, 3.10699e-06, 2.56822e-06, 2.1141e-06, 1.73297e-06, 1.4145e-06, 1.14958e-06, 9.30199e-07, 7.49367e-07, 6.01001e-07, 4.79843e-07, 3.81374e-07, 3.01728e-07, 2.37618e-07, 1.86267e-07, 1.45337e-07, 1.12875e-07, 8.72566e-08, 6.71403e-08, 5.1423e-08, 3.9204e-08, 2.97519e-08, 2.24765e-08, 1.69039e-08, 1.26565e-08, 9.43474e-09, 7.00267e-09, 5.17537e-09, 3.80882e-09, 2.79153e-09)


datapileup_6pb_2015b_doubleMu_plus_doubleEl_LoicPreliminary = cms.vdouble(0.000000, 0.001143, 0.001715, 0.004573, 0.006573, 0.018005, 0.035153, 0.059446, 0.083167, 0.108317, 0.114033, 0.114033, 0.105744, 0.098600, 0.073450, 0.056588, 0.042584, 0.027436, 0.022578, 0.012003, 0.007431, 0.004001, 0.001429, 0.001143, 0.000000, 0.000857, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000)


datapileup_40pb_2015b_singlemu_PietroPreliminary = cms.vdouble(0, 0.000810074, 0.000608754, 0.00108809, 0.00249253, 0.00551713, 0.0105118, 0.0185071, 0.0286402, 0.041285, 0.0545865, 0.066728, 0.0771487, 0.0843004, 0.0874832, 0.0865053, 0.0815586, 0.0744261, 0.0637849, 0.054203, 0.0435427, 0.0336157, 0.0249877, 0.0184448, 0.0129276, 0.00930386, 0.00608754, 0.00412227, 0.00258361, 0.00155783, 0.00104015, 0.000613547, 0.00038826, 0.000282807, 0.000139007, 8.14867e-05, 3.35533e-05, 3.35533e-05, 1.91733e-05, 4.79334e-06, 4.79334e-06, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)

datapileup_42pb_2015b_singlemu_cleanNvtx_PietroPreliminary = cms.vdouble(0, 0.000520232, 0.000399503, 0.000812177, 0.00193605, 0.00449551, 0.00890321, 0.0160218, 0.0253465, 0.036965, 0.0495867, 0.0622983, 0.0730827, 0.082482, 0.0871882, 0.0879104, 0.0847583, 0.0780194, 0.0678628, 0.0582638, 0.0469855, 0.0370682, 0.0278972, 0.0199334, 0.0141692, 0.00980538, 0.00633717, 0.00422771, 0.00259896, 0.00165289, 0.000994367, 0.000557548, 0.000357797, 0.000237068, 0.000158045, 7.24374e-05, 3.73162e-05, 3.0731e-05, 1.53655e-05, 6.58521e-06, 4.39014e-06, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)

datapileup_42pb_2015C_singlemu_cleanNvtx_PietroPreliminary = cms.vdouble( 3.9990225e-05, 0.00064873031, 0.001959521, 0.0054431139, 0.011774899, 0.022367866, 0.038852725, 0.058638999, 0.079758281, 0.094741285, 0.10511208, 0.10750261, 0.10176179, 0.090711159, 0.07544378, 0.060140854, 0.045606629, 0.033391838, 0.023540912, 0.015938326, 0.010077537, 0.0066961409, 0.0040212393, 0.0024971674, 0.0015818356, 0.00084423808, 0.00040878896, 0.00025327142, 0.00013774411, 3.5546866e-05, 4.4433583e-05, 4.4433583e-06, 1.3330075e-05, 0, 0, 4.4433583e-06, 0, 4.4433583e-06, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 )


runProcess = cms.PSet(
    dtag  = cms.string("MC13TeV_TTJets_inclusive"),
    input = cms.untracked.vstring("file:/afs/cern.ch/work/v/vischia/public/miniaod/TT_TuneCUETP8M1_13TeV-powheg-pythia8_MINIAODSIM_74X_mcRun2_asymptotic_v2-v1_40000_021CEA7F-4672-E511-A487-B499BAAC04F0.root"),
    outfile = cms.string("Test_TT.root"),
    isMC = cms.bool(True),
    triggerstudy = cms.bool(False),
    xsec = cms.double(831.76),
    suffix = cms.string(""), 
    cprime = cms.double(-1),	
    brnew = cms.double(-1),	
    mctruthmode = cms.int32(0),
    jacknife = cms.vint32(0,0),
    saveSummaryTree = cms.bool(False),
    runSystematics = cms.bool(False),
    weightsFile = cms.vstring(""),
    muscleDir = cms.string("${CMSSW_BASE}/src/UserCode/llvv_fwk/data/jec/"),
    dirName = cms.string("dataAnalyzer"),
    useMVA = cms.bool(False),
    tmvaInput = mySignalMVA,
    jecDir = cms.string('${CMSSW_BASE}/src/UserCode/llvv_fwk/data/jec/25ns'),
    #datapileup = datapileup_6pb_2015b_doubleMu_plus_doubleEl_LoicPreliminary,
    datapileup = datapileup_42pb_2015C_singlemu_cleanNvtx_PietroPreliminary, 
    datapileupSingleLep = datapileup_42pb_2015C_singlemu_cleanNvtx_PietroPreliminary,
    debug = cms.bool(True),
    lumisToProcess = LumiList.LumiList(filename = "").getVLuminosityBlockRange(),
    pujetidparas = cms.PSet(pu_jetid),
    electronidparas = cms.PSet(myVidElectronId),
    maxevents = cms.int32(-1) # set to -1 when running on grid. 
)


#STUFF BELLOW IS RUN ONLY IF CRAB IS USED
def lfn_to_pfn(f):
    import subprocess
    proc = subprocess.Popen(["edmFileUtil -d %s" %f],
                            stdout=subprocess.PIPE, shell=True)
    (out, err) = proc.communicate()
    pfn = out.strip()
    return pfn


try:
    import PSet
    fnames = [ lfn_to_pfn(f) for f in list(PSet.process.source.fileNames)]    
    inputFilesPfn =  cms.untracked.vstring(fnames)                
    print inputFilesPfn
    runProcess.input = inputFilesPfn
    runProcess.outfile = cms.string("output.root")
    process.source.fileNames = PSet.process.source.fileNames
except:
    pass
