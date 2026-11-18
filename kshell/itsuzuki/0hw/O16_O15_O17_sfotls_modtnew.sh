#!/bin/sh 
# export OMP_STACKSIZE=1g
export GFORTRAN_UNBUFFERED_PRECONNECTED=y
# ulimit -s unlimited

# ---------- O16_sfotls_modtnew --------------
echo "start running log_O16_sfotls_modtnew_m0p.txt ..."
cat > O16_sfotls_modtnew_0.input <<EOF
&input
  beta_cm = 10.0
  eff_charge = 1.5, 0.5, 
  fn_int = "sfotls_modtnew.snt"
  fn_ptn = "O16_sfotls_modtnew_p.ptn"
  fn_save_wave = "O16_sfotls_modtnew_m0p.wav"
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
  n_restart_vec = 15
&end
EOF
nice ./kshell.exe O16_sfotls_modtnew_0.input > log_O16_sfotls_modtnew_m0p.txt 2>&1 

# rm -f tmp_snapshot_O16_sfotls_modtnew_p.ptn_0_* tmp_lv_O16_sfotls_modtnew_p.ptn_0_* O16_sfotls_modtnew_0.input 


./collect_logs.py log_*O16_sfotls_modtnew* > summary_O16_sfotls_modtnew.txt
echo "Finish computing O16_sfotls_modtnew.    See summary_O16_sfotls_modtnew.txt"
echo 

# ---------- O15_sfotls_modtnew --------------
echo "start running log_O15_sfotls_modtnew_m1n.txt ..."
cat > O15_sfotls_modtnew_1.input <<EOF
&input
  beta_cm = 10.0
  eff_charge = 1.5, 0.5, 
  fn_int = "sfotls_modtnew.snt"
  fn_ptn = "O15_sfotls_modtnew_n.ptn"
  fn_save_wave = "O15_sfotls_modtnew_m1n.wav"
  gl = 1.0, 0.0, 
  gs = 5.585, -3.826, 
  hw_type = 1
  is_double_j = .false.
  max_lanc_vec = 200
  maxiter = 300
  mode_lv_hdd = 1
  mtot = 1
  n_block = 0
  n_eigen = 10
  n_restart_vec = 15
&end
EOF
nice ./kshell.exe O15_sfotls_modtnew_1.input > log_O15_sfotls_modtnew_m1n.txt 2>&1 

rm -f tmp_snapshot_O15_sfotls_modtnew_n.ptn_1_* tmp_lv_O15_sfotls_modtnew_n.ptn_1_* O15_sfotls_modtnew_1.input 


./collect_logs.py log_*O15_sfotls_modtnew* > summary_O15_sfotls_modtnew.txt
echo "Finish computing O15_sfotls_modtnew.    See summary_O15_sfotls_modtnew.txt"
echo 

# ---------- O17_sfotls_modtnew --------------
echo "start running log_O17_sfotls_modtnew_m1p.txt ..."
cat > O17_sfotls_modtnew_1.input <<EOF
&input
  beta_cm = 10.0
  eff_charge = 1.5, 0.5, 
  fn_int = "sfotls_modtnew.snt"
  fn_ptn = "O17_sfotls_modtnew_p.ptn"
  fn_save_wave = "O17_sfotls_modtnew_m1p.wav"
  gl = 1.0, 0.0, 
  gs = 5.585, -3.826, 
  hw_type = 1
  is_double_j = .false.
  max_lanc_vec = 200
  maxiter = 300
  mode_lv_hdd = 1
  mtot = 1
  n_block = 0
  n_eigen = 10
  n_restart_vec = 15
&end
EOF
nice ./kshell.exe O17_sfotls_modtnew_1.input > log_O17_sfotls_modtnew_m1p.txt 2>&1 

rm -f tmp_snapshot_O17_sfotls_modtnew_p.ptn_1_* tmp_lv_O17_sfotls_modtnew_p.ptn_1_* O17_sfotls_modtnew_1.input 


./collect_logs.py log_*O17_sfotls_modtnew* > summary_O17_sfotls_modtnew.txt
echo "Finish computing O17_sfotls_modtnew.    See summary_O17_sfotls_modtnew.txt"
echo 

