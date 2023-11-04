# DUNE_PRISM_GEC_ND
DUNE-PRISM: GEC from muon side
> Some are copied from: [FNAL instruction](https://github.com/FlynnYGUO/NeutrinoPhysics/blob/main/GEC/BaronCodeOutdated/NDGEC.md) and [NNhome instruction](https://github.com/FlynnYGUO/NeutrinoPhysics/blob/main/GEC/BaronNewCode/Instructions.md)
## FNAL machine
### 0. Setup
#### 1. Log in:
```
kfnal                                      # Short for kinit -f <username>@FNAL.GOV. In my laptop, alias kfnal="/usr/bin/kinit flynnguo@FNAL.GOV" in ~/.zshrc
ssh -X flynnguo@dunegpvm15.fnal.gov
exit                                       # Quit FNAL
```
#### 2. DUNE FNAL machines (dunegpvm*) environment setup
Only do it once:
```
cd /dune/app/users/flynnguo                                             # Replace with your username for all commands below
git clone https://github.com/FlynnYGUO/DUNE_PRISM_GEC_ND.git
# This allows using pip
source /cvmfs/dune.opensciencegrid.org/products/dune/setup_dune.sh
setup dunetpc v09_41_00_02 -q e20:prof
pip install --target=/dune/app/users/flynnguo/lib/python3.9/site-packages uproot4
pip install --target=/dune/app/users/flynnguo/lib/python3.9/site-packages uproot3
pip install --target=/dune/app/users/flynnguo/lib/python3.9/site-packages torch
pip install --target=/dune/app/users/flynnguo/lib/python3.9/site-packages scipy
```


## NNhome machine
### 0. Setup
#### 1. Log in:
```
ssh -X fyguo@nnhome.physics.sunysb.edu       # Log my ivy account: <username>@ivy.physics.sunysb.edu
passwd                                     # Reset my password  
exit                                       # Quit ivy
```
#### 2. Install packages/tools
```
pip install --target=<a directory you specify> uproot  # Install uproot, you may also need to install torch
source /home/rrazakami/workspace/ROOT/root_binary/bin/thisroot.sh  # Use other's ROOT source file instead of installing a new one under my repository
# Remember to source root.sh every time once log in the NNhome machine
```
#### 3. Muon NN
The network outputs how probable a muon is fully contained in ND LAr and tracker matched in TMS downstream: https://github.com/weishi10141993/MuonEffNN
The current used network file is located at ```/home/barwu/repos/MuonEffNN/8thTry/muonEff30.nn```

### I. Get ND eff files
The ND CAFs have been copied from Fermilab to NNhome under this path:
```
/storage/shared/barwu/10thTry/NDCAF
```
They are grouped into different folders, as well as numerical subfolders. You will need to check these names to decide which of the subfolders you will run.

The following scripts are used to produce raw, selected, and efficiency-corrected distributions. These are the red-green-blue curves for muon only (fully contained and tracker matched), hadron only, and combined.

Run the script using python3, this creates a TTree containing all efficiency information:
```
python3 /home/fyguo/DUNE_PRISM_GEC_ND/code/new_hadron_muon_mktree.py [folder]/[subfolder]
```
A batch submission is also available, it usually takes half a day to complete:
```
sbatch Slurm_nnhome_ND.sh [folder]/[subfolder] # For example, Omgsimple/104
# There are four GPU node lists: fir, birch, aspen, cedar
```
The batch job should generate 1000 efficiency files. These files contain the efficiency information of each event, the cut information, as well as 3 additional sets of information about the total lepton momentum, longitudinal lepton momentum and the cosine of the angle between the neutrino and lepton momentum vectors.

To convert all the efficiency files into a set of histograms, run:
```
root -l -b
.L /home/fyguo/DUNE_PRISM_GEC_ND/code/histogram_files_ND.cpp
histogram_files_ND(<geoeff_cut>) #For example: histogram_files_ND(0.1) means 10% geoeff_cut
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

Run this script to draw histograms directly using the FD CAFs and efficiency files produced by the above shell script:
```
root -l -b
.L /home/fyguo/DUNE_PRISM_GEC_ND/code/draw_histograms_FD.cpp
draw_histograms_FD(<geoeff_cut>) #For example: draw_histograms_FD(0.1) means 10% geoeff_cut
```

### III. Get ND ratios vs FD ratios
Generate all ND ratios, FD ratios and NDvsFD ratios.
```
root -l -b
.L /home/fyguo/DUNE_PRISM_GEC_ND/code/NDaFD_RatioPlots.cpp
NDaFD_RatioPlots(<geoeff_cut>) #For example: NDaFD_RatioPlots(0.1) means 10% geoeff_cut
```
