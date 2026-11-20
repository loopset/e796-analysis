#!/bin/sh 
# export OMP_STACKSIZE=1g
export GFORTRAN_UNBUFFERED_PRECONNECTED=y
# ulimit -s unlimited

# ---------- O20_sfotls_modtnew --------------
echo "start running log_O20_sfotls_modtnew_m0p.txt ..."
cat > O20_sfotls_modtnew_0.input <<EOF
&input
  beta_cm = 10.0
  eff_charge = 1.5, 0.5, 
  fn_int = "sfotls_modtnew.snt"
  fn_ptn = "O20_sfotls_modtnew_p.ptn"
  fn_save_wave = "O20_sfotls_modtnew_m0p.wav"
  gl = 1.0, 0.0, 
  gs = 5.585, -3.826, 
  hw_type = 1
  is_double_j = .false.
  max_lanc_vec = 200
  maxiter = 300
  mode_lv_hdd = 1
  mtot = 0
  n_block = 0
  n_eigen = 20
  n_restart_vec = 30
&end
EOF
nice ./kshell.exe O20_sfotls_modtnew_0.input > log_O20_sfotls_modtnew_m0p.txt 2>&1 

rm -f tmp_snapshot_O20_sfotls_modtnew_p.ptn_0_* tmp_lv_O20_sfotls_modtnew_p.ptn_0_* O20_sfotls_modtnew_0.input 


echo "start running log_O20_sfotls_modtnew_m0n.txt ..."
cat > O20_sfotls_modtnew_0.input <<EOF
&input
  beta_cm = 10.0
  eff_charge = 1.5, 0.5, 
  fn_int = "sfotls_modtnew.snt"
  fn_ptn = "O20_sfotls_modtnew_n.ptn"
  fn_save_wave = "O20_sfotls_modtnew_m0n.wav"
  gl = 1.0, 0.0, 
  gs = 5.585, -3.826, 
  hw_type = 1
  is_double_j = .false.
  max_lanc_vec = 200
  maxiter = 300
  mode_lv_hdd = 1
  mtot = 0
  n_block = 0
  n_eigen = 10
  n_restart_vec = 30
&end
EOF
nice ./kshell.exe O20_sfotls_modtnew_0.input > log_O20_sfotls_modtnew_m0n.txt 2>&1 

rm -f tmp_snapshot_O20_sfotls_modtnew_n.ptn_0_* tmp_lv_O20_sfotls_modtnew_n.ptn_0_* O20_sfotls_modtnew_0.input 


./collect_logs.py log_*O20_sfotls_modtnew* > summary_O20_sfotls_modtnew.txt
echo "Finish computing O20_sfotls_modtnew.    See summary_O20_sfotls_modtnew.txt"
echo 

