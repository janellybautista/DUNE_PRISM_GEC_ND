#!/bin/bash
#
#SBATCH --job-name=ND_hadron_muon
##SBATCH --output=ND_hadron_muon.log
#SBATCH --mail-type=END,FAIL
#SBATCH --mail-user=Flynn.Y.Guo@stonybrook.edu
#SBATCH --nodelist=cedar # --nodes=4 --gres=gpu
#SBATCH -c 30
#SBATCH --time=180:00:00
#!/bin/bash

cd /home/fyguo/testbaroncode
python3 ~/home/fyguo/DUNE_PRISM_GEC_ND/code/new_hadron_muon_mktree.py $1
wait
