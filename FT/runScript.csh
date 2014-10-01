#!/bin/csh

# Start from current working directory
#$ -cwd
#
#Set virtual memory limit to 2GB
#$ -l h_vmem=2G
#
# Join stdout and stderr
#$ -j y
#
# and put them somewhere
#$ -o /global/u1/j/jgruszko/software/Logs
#
# Specify mj just to be sure
#$ -P majorana
#
# Resources: use projectio=1 as requested by Lisa Gerhart, even if there
# will be a large number of jobs accessing /project
#$ -l projectio=1

cd $TMPDIR

echo root -b -q /global/u1/j/jgruszko/software/MJ_Scripts/FT/FTWF_power.C > /global/u1/j/jgruszko/software/Logs/FTWF_power.log 
root -b -q /global/u1/j/jgruszko/software/MJ_Scripts/FT/FTWF_power.C > /global/u1/j/jgruszko/software/Logs/FTWF_power.log 

