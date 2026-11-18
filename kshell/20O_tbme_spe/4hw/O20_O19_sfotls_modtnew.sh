#!/bin/sh 
# export OMP_STACKSIZE=1g
export GFORTRAN_UNBUFFERED_PRECONNECTED=y
# ulimit -s unlimited

# ---------- O20_sfotls_modtnew --------------
echo "start running log_O20_sfotls_modtnew_j0p.txt ..."
cat > O20_sfotls_modtnew_0.input <<EOF
&input
  beta_cm = 10.0
  eff_charge = 1.5, 0.5, 
  fn_int = "sfotls_modtnew.snt"
  fn_ptn = "O20_sfotls_modtnew_p.ptn"
  fn_save_wave = "O20_sfotls_modtnew_j0p.wav"
  gl = 1.0, 0.0, 
  gs = 5.585, -3.826, 
  hw_type = 1
  is_double_j = .true.
  max_lanc_vec = 200
  maxiter = 300
  mode_lv_hdd = 1
  mtot = 0
  n_block = 0
  n_eigen = 1
  n_restart_vec = 10
&end
EOF
nice ./kshell.exe O20_sfotls_modtnew_0.input > log_O20_sfotls_modtnew_j0p.txt 2>&1 

rm -f tmp_snapshot_O20_sfotls_modtnew_p.ptn_0_* tmp_lv_O20_sfotls_modtnew_p.ptn_0_* O20_sfotls_modtnew_0.input 


./collect_logs.py log_*O20_sfotls_modtnew* > summary_O20_sfotls_modtnew.txt
echo "Finish computing O20_sfotls_modtnew.    See summary_O20_sfotls_modtnew.txt"
echo 

# ---------- O19_sfotls_modtnew --------------
echo "start running log_O19_sfotls_modtnew_m1p.txt ..."
cat > O19_sfotls_modtnew_1.input <<EOF
&input
  beta_cm = 10.0
  eff_charge = 1.5, 0.5, 
  fn_int = "sfotls_modtnew.snt"
  fn_ptn = "O19_sfotls_modtnew_p.ptn"
  fn_save_wave = "O19_sfotls_modtnew_m1p.wav"
  gl = 1.0, 0.0, 
  gs = 5.585, -3.826, 
  hw_type = 1
  is_double_j = .false.
  max_lanc_vec = 200
  maxiter = 300
  mode_lv_hdd = 1
  mtot = 1
  n_block = 0
  n_eigen = 20
  n_restart_vec = 30
&end
EOF
nice ./kshell.exe O19_sfotls_modtnew_1.input > log_O19_sfotls_modtnew_m1p.txt 2>&1 

rm -f tmp_snapshot_O19_sfotls_modtnew_p.ptn_1_* tmp_lv_O19_sfotls_modtnew_p.ptn_1_* O19_sfotls_modtnew_1.input 


echo "start running log_O19_sfotls_modtnew_m1n.txt ..."
cat > O19_sfotls_modtnew_1.input <<EOF
&input
  beta_cm = 10.0
  eff_charge = 1.5, 0.5, 
  fn_int = "sfotls_modtnew.snt"
  fn_ptn = "O19_sfotls_modtnew_n.ptn"
  fn_save_wave = "O19_sfotls_modtnew_m1n.wav"
  gl = 1.0, 0.0, 
  gs = 5.585, -3.826, 
  hw_type = 1
  is_double_j = .false.
  max_lanc_vec = 200
  maxiter = 300
  mode_lv_hdd = 1
  mtot = 1
  n_block = 0
  n_eigen = 20
  n_restart_vec = 30
&end
EOF
nice ./kshell.exe O19_sfotls_modtnew_1.input > log_O19_sfotls_modtnew_m1n.txt 2>&1 

rm -f tmp_snapshot_O19_sfotls_modtnew_n.ptn_1_* tmp_lv_O19_sfotls_modtnew_n.ptn_1_* O19_sfotls_modtnew_1.input 


./collect_logs.py log_*O19_sfotls_modtnew* > summary_O19_sfotls_modtnew.txt
echo "Finish computing O19_sfotls_modtnew.    See summary_O19_sfotls_modtnew.txt"
echo 

# --------- spectroscocpic factor --------- 
echo 
echo "spectroscopic factor calc."
echo "start running log_O20_O19_sfotls_modtnew_tr_j0p_m1p.txt ..."
cat > O20_O19_sfotls_modtnew_0_1.input <<EOF
&input
  fn_int   = "sfotls_modtnew.snt"
  fn_ptn_l = "O20_sfotls_modtnew_p.ptn"
  fn_ptn_r = "O19_sfotls_modtnew_p.ptn"
  fn_load_wave_l = "O20_sfotls_modtnew_j0p.wav"
  fn_load_wave_r = "O19_sfotls_modtnew_m1p.wav"
  hw_type = 1
  eff_charge = 1.5, 0.5
  gl = 1.0, 0.0
  gs = 5.585, -3.826
&end
EOF
nice ./transit.exe O20_O19_sfotls_modtnew_0_1.input > log_O20_O19_sfotls_modtnew_tr_j0p_m1p.txt 2>&1 

echo "start running log_O20_O19_sfotls_modtnew_tr_j0p_m1n.txt ..."
cat > O20_O19_sfotls_modtnew_0_1.input <<EOF
&input
  fn_int   = "sfotls_modtnew.snt"
  fn_ptn_l = "O20_sfotls_modtnew_p.ptn"
  fn_ptn_r = "O19_sfotls_modtnew_n.ptn"
  fn_load_wave_l = "O20_sfotls_modtnew_j0p.wav"
  fn_load_wave_r = "O19_sfotls_modtnew_m1n.wav"
  hw_type = 1
  eff_charge = 1.5, 0.5
  gl = 1.0, 0.0
  gs = 5.585, -3.826
&end
EOF
nice ./transit.exe O20_O19_sfotls_modtnew_0_1.input > log_O20_O19_sfotls_modtnew_tr_j0p_m1n.txt 2>&1 

