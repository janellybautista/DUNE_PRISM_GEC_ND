#!/bin/bash
#
#SBATCH --job-name=NDhist_files
#SBATCH --output=NDhist_files_%j.log
#SBATCH --mail-type=END,FAIL
#SBATCH --mail-user=Flynn.Y.Guo@stonybrook.edu
#SBATCH --nodelist=birch # --nodes=4 --gres=gpu
#SBATCH --time=180:00:00
#!/bin/bash

cd /home/fyguo/DUNE_PRISM_GEC_ND/code
root -l -b histogram_files_ND.cpp
wait
