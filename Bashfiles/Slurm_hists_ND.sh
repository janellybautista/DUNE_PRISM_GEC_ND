#!/bin/bash
#
#SBATCH --job-name=NDhist_files
#SBATCH --output=hist_files-%j_0.001_eff_cut_200_bins.log
#SBATCH --mail-type=END,FAIL
#SBATCH --mail-user=Flynn.Y.Guo@stonybrook.edu
#SBATCH --nodelist=birch # --nodes=4 --gres=gpu
#SBATCH --time=72:00:00
#!/bin/bash

cd /home/fyguo/testbaroncode
root ~/NeutrinoPhysics/GEC/BaronNewCode/code/histogram_files_ND.cpp
wait
