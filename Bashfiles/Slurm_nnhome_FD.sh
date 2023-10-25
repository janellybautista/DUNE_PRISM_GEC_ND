#!/bin/bash
#
#SBATCH --job-name=FD_hadron_muon
#SBATCH --output=FD_maketree-%j.log
#SBATCH --mail-type=END,FAIL
#SBATCH --mail-user=Flynn.Y.Guo@stonybrook.edu
#SBATCH --nodelist=aspen # --nodes=4 --gres=gpu
#SBATCH -c 30
#SBATCH --time=180:00:00
#!/bin/bash

cd /home/fyguo/testbaroncode
python3 ~/NeutrinoPhysics/GEC/BaronNewCode/code/FD_maketree.py
wait
