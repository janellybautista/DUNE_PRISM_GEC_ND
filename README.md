# DUNE_PRISM_GEC_ND
DUNE-PRISM: GEC from muon side
> - ND CAF Maker: https://internal.dunescience.org/doxygen/ND__CAFMaker_2dumpTree_8py_source.html
> - Some are copied from: [FNAL instruction](https://github.com/FlynnYGUO/NeutrinoPhysics/blob/main/GEC/BaronCodeOutdated/NDGEC.md) and [NNhome instruction](https://github.com/FlynnYGUO/NeutrinoPhysics/blob/main/GEC/BaronNewCode/Instructions.md)
> - [DUNE Computing Training](https://dune.github.io/computing-basics/index.html)  

## FNAL machine
### 0. Setup
#### 1. Log in & DUNE FNAL machines (dunegpvm*) environment setup:
```
kfnal                                      # Short for kinit -f <username>@FNAL.GOV. 
ssh -X janelly@dunegpvm01.fnal.gov      
exit                                       # Quit FNAL
```
or to directly access the DUNE FNAL machine through the vnc server, use
```
ssh -Y -L 5901:localhost:5901 janelly@dunegpvm01.fnal.gov
```
 [DUNE: SL7 to ALMA9](https://wiki.dunescience.org/wiki/SL7_to_Alma9_conversion):
```
/cvmfs/oasis.opensciencegrid.org/mis/apptainer/current/bin/apptainer shell --shell=/bin/bash \
-B /cvmfs,/exp,/nashome,/pnfs/dune,/opt,/run/user,/etc/hostname,/etc/hosts,/etc/krb5.conf --ipc --pid \
/cvmfs/singularity.opensciencegrid.org/fermilab/fnal-dev-sl7:latest
```
Environment setup (only do it once):
```
cd /exp/dune/app/users/$USER                                            
git clone https://github.com/FlynnYGUO/DUNE_PRISM_GEC_ND.git
# This allows using pip
source /cvmfs/dune.opensciencegrid.org/products/dune/setup_dune.sh 
setup dunetpc v09_41_00_02 -q e20:prof
```
Next time once you log in to the FNAL machine, do the following (do it every time you log in):
```
cd /exp/dune/app/users/$USER
source /cvmfs/dune.opensciencegrid.org/products/dune/setup_dune.sh 
setup dunetpc v09_41_00_02 -q e20:prof
export PYTHONPATH=/exp/dune/app/users/$USER/lib/python3.9/site-packages:$PYTHONPATH
```
Files I/O on DUNE machines
Input ND CAF files are here: ```/pnfs/dune/persistent/physicsgroups/dunelbl/abooth/PRISM/Production/Simulation/ND_CAFMaker/v7/CAF```
Output files from grid jobs are written to the scratch area ```/pnfs/dune/scratch/users/<your username>```.

Please avoid reading from, copying from, or writing massive amount of files directly to the pnfs area ```/pnfs/dune/persistent```, this will slow down the file system. Refer to [this wiki](https://mu2ewiki.fnal.gov/wiki/DataTransfer) for good practices on data transfer.  
> To transfer files, highly recommended to tar all files first ```tar -czvf <Filename.tar.gz> all_files``` and copy the .tar.gz to your directory then untar it ```tar -xvzf``` instead of copying individual root files.

#### 2. Interactive run
If you want to run code interactively on ```dunegpvm*``` for debugging, follow instruction in this section.
```
cd DUNE_PRISM_GEC_ND/code
python3 new_hadron_muon_mktree.py /pnfs/dune/persistent/physicsgroups/dunelbl/abooth/PRISM/Production/Simulation/ND_CAFMaker/v7/CAF/0mgsimple/101/FHC.1101999.CAF.root
```



Run this script to draw histograms directly using the FD CAFs and efficiency files produced by the above shell script:
```
root -l -b
.L /home/fyguo/DUNE_PRISM_GEC_ND/code/draw_histograms_FD.cpp
draw_histograms_FD(<geoeff_cut>) #For example: draw_histograms_FD(0.1) means 10% geoeff_cut
```

### III. Get ND ratios vs FD ratios
Generate all ND ratios, FD ratios and NDvsFD ratios.
```
root -l -b /home/fyguo/DUNE_PRISM_GEC_ND/code/NDaFD_RatioPlots.cpp
```

#### 3.1 Submit grid job (Geometric efficiency for ND events at ND)
```
# Make a tarball to send everything you need to run your program on grid node
cd DUNE_PRISM_GEC_ND/code
# Write list of files into a txt file, remember to change the foldername before submitting job
ls -d "/pnfs/dune/persistent/physicsgroups/dunelbl/abooth/PRISM/Production/Simulation/ND_CAFMaker/v7/CAF/<folder_name>”/* | sed "s\/pnfs\root://fndca1.fnal.gov:1094/pnfs/fnal.gov/usr\g" > NDCAFs.txt
# For example:
ls -d "/pnfs/dune/persistent/physicsgroups/dunelbl/abooth/PRISM/Production/Simulation/ND_CAFMaker/v7/CAF/0mgsimple/"{110,111,112,113,114,115,116,117,118,119}/* | sed "s\/pnfs\root://fndca1.fnal.gov:1094/pnfs/fnal.gov/usr\g" > NDCAFs.txt
# Now make a tarball
tar -czvf ND_work.tar.gz setup_NDcombEff.sh new_hadron_muon_mktree.py muonEff30.nn muonEffModel.py NDCAFs.txt


# The following long command submits your job:
# -N 2 means 2 jobs, this is now set as running 1 file per job as I have two files in txt file.
# If you have X files in your text file, set: -N X
# You can use ```wc -l NDCAFs.txt``` to check the number of files in txt file
jobsub_submit -G dune -N 9980 --memory=5GB --disk=10GB --expected-lifetime=1.5h --cpu=1 --resource-provides=usage_model=DEDICATED,OPPORTUNISTIC,OFFSITE --tar_file_name=dropbox:///exp/dune/app/users/flynnguo/DUNE_PRISM_GEC_ND/code/ND_work.tar.gz --use-cvmfs-dropbox -l '+SingularityImage=\"/cvmfs/singularity.opensciencegrid.org/fermilab/fnal-wn-sl7:latest\"' --append_condor_requirements='(TARGET.HAS_Singularity==true&&TARGET.HAS_CVMFS_dune_opensciencegrid_org==true&&TARGET.HAS_CVMFS_larsoft_opensciencegrid_org==true&&TARGET.CVMFS_dune_opensciencegrid_org_REVISION>=1105&&TARGET.HAS_CVMFS_fifeuser1_opensciencegrid_org==true&&TARGET.HAS_CVMFS_fifeuser2_opensciencegrid_org==true&&TARGET.HAS_CVMFS_fifeuser3_opensciencegrid_org==true&&TARGET.HAS_CVMFS_fifeuser4_opensciencegrid_org==true)' file:///exp/dune/app/users/flynnguo/DUNE_PRISM_GEC_ND/code/run_NDcombEff.sh
```

To query the job status: ```jobsub_q <usrname> -G dune```. [See job details](https://fifemon.fnal.gov/monitor/d/000000115/job-cluster-summary?orgId=1&var-cluster=73871417&var-schedd=jobsub02.fnal.gov)  
To remove a job status: ```condor_rm <usrname> -G dune```
To remove a specific job: ```jobsub_rm --jobid=<jobid>@jobsub<xx>.fnal.gov```

If your job gets held (HoldReasonCode 26), subcode 1 is memory, 2 is disk, 4 is too many starts (max of 10 attempts to run a job), 8 is run time. You can can also get HoldReasonCode 34 and subcode 0, which is also memory.

The job script (```run_NDcombEff.sh```) can be adjusted to run more than one file per job, if desired, contact me. The magic happens at these lines:
```
(( LINE_N = ${PROCESS} + 1 ))

for ifile in $(cat ${INPUT_TAR_DIR_LOCAL}/NDCAFs.txt | head -${LINE_N} | tail -1); do
```
#### 3.2 Submit grid job (Geometric efficiency for FD events at ND)
```
# Make a tarball to send everything you need to run your program on grid node
cd DUNE_PRISM_GEC_ND/code
# Write list of files into a txt file, remember to change the foldername before submitting job
ls -d "/pnfs/dune/persistent/users/flynnguo/FDGeoEffinND/<FDGeoEff_JOBID>"/* | sed "s\/pnfs\root://fndca1.fnal.gov:1094/pnfs/fnal.gov/usr\g" > FDCAFs.txt
# For example:
ls -d "/pnfs/dune/persistent/users/flynnguo/FDGeoEffinND/FDGeoEff_2811722"/* | sed "s\/pnfs\root://fndca1.fnal.gov:1094/pnfs/fnal.gov/usr\g" > FDCAFs.txt
# Now make a tarball
tar -czvf FD_work.tar.gz setup_NDcombEff.sh FD_maketree.py muonEff30.nn muonEffModel.py FDCAFs.txt
# You can use ```wc -l FDCAFs.txt``` to check the number of files in txt file
jobsub_submit -G dune -N 9635 --memory=5GB --disk=10GB --expected-lifetime=8h --cpu=1 --resource-provides=usage_model=DEDICATED,OPPORTUNISTIC,OFFSITE --tar_file_name=dropbox:///exp/dune/app/users/flynnguo/DUNE_PRISM_GEC_ND/code/FD_work.tar.gz --use-cvmfs-dropbox -l '+SingularityImage=\"/cvmfs/singularity.opensciencegrid.org/fermilab/fnal-wn-sl7:latest\"' --append_condor_requirements='(TARGET.HAS_Singularity==true&&TARGET.HAS_CVMFS_dune_opensciencegrid_org==true&&TARGET.HAS_CVMFS_larsoft_opensciencegrid_org==true&&TARGET.CVMFS_dune_opensciencegrid_org_REVISION>=1105&&TARGET.HAS_CVMFS_fifeuser1_opensciencegrid_org==true&&TARGET.HAS_CVMFS_fifeuser2_opensciencegrid_org==true&&TARGET.HAS_CVMFS_fifeuser3_opensciencegrid_org==true&&TARGET.HAS_CVMFS_fifeuser4_opensciencegrid_org==true)' file:///exp/dune/app/users/flynnguo/DUNE_PRISM_GEC_ND/code/run_FDcombEff.sh
```


Run the script using python3, this creates a TTree containing all efficiency information:
```
python3 /home/fyguo/DUNE_PRISM_GEC_ND/code/new_hadron_muon_mktree.py [folder]/[subfolder]
```
The batch job should generate 1000 efficiency files. These files contain the efficiency information of each event, the cut information, as well as 3 additional sets of information about the total lepton momentum, longitudinal lepton momentum and the cosine of the angle between the neutrino and lepton momentum vectors.

To convert all the efficiency files into a set of histograms, run:
```
root -l -b /home/fyguo/DUNE_PRISM_GEC_ND/code/histogram_files_ND.cpp
```
They have been organized depending on what parameter is being plotted, for example, neutrino energy or position. In each group, it contains graphs for raw, selection-cut and
geometrically-corrected distributions.

Now run the script to draw the histograms using the aforementioned TFiles:
```
root -l -b
.L /home/fyguo/DUNE_PRISM_GEC_ND/code/draw_histograms_ND.cpp
draw_histograms_ND(<geoeff_cut>) #For example: draw_histograms_ND(0.1) means 10% geoeff_cut
```
There will be a set of all graphs available, and a copy of each of those graphs organized by selection-cut into different TCanvases. Ratio plots for each graph are also available. The graphs and ratio plots will automatically save once they are finished loading.

### II. Get FD eff files
Run this script:
```
python3 /home/fyguo/DUNE_PRISM_GEC_ND/code/FD_maketree.py
```
This generates efficiency files for far detector events.

To submit a job:
```
sbatch Slurm_nnhome_FD.sh
```
