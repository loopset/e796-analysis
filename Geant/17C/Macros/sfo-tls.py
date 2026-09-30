import pyphysics as phys

sfo = phys.ShellModel(
    [
        "../Inputs/SFO-tls/log_C18_C17_sfotls_tr_j0n_j3p.txt",
        "../Inputs/SFO-tls/log_C18_C17_sfotls_tr_j0p_j3p.txt",
        "../Inputs/SFO-tls/log_C18_C17_sfotls_tr_j2n_j3p.txt",
        "../Inputs/SFO-tls/log_C18_C17_sfotls_tr_j2p_j3p.txt",
        "../Inputs/SFO-tls/log_C18_C17_sfotls_tr_j4n_j3p.txt",
        "../Inputs/SFO-tls/log_C18_C17_sfotls_tr_j4p_j3p.txt",
        "../Inputs/SFO-tls/log_C18_C17_sfotls_tr_j6n_j3p.txt",
        "../Inputs/SFO-tls/log_C18_C17_sfotls_tr_j6p_j3p.txt",
        "../Inputs/SFO-tls/log_C18_C17_sfotls_tr_j8n_j3p.txt",
        "../Inputs/SFO-tls/log_C18_C17_sfotls_tr_j8p_j3p.txt",
        "../Inputs/SFO-tls/log_C18_C17_sfotls_tr_j10n_j3p.txt",
        "../Inputs/SFO-tls/log_C18_C17_sfotls_tr_j10p_j3p.txt",
        "../Inputs/SFO-tls/log_C18_C17_sfotls_tr_j12n_j3p.txt",
        "../Inputs/SFO-tls/log_C18_C17_sfotls_tr_j12p_j3p.txt",
        "../Inputs/SFO-tls/log_C18_C17_sfotls_tr_j14n_j3p.txt",
        "../Inputs/SFO-tls/log_C18_C17_sfotls_tr_j14p_j3p.txt",
        "../Inputs/SFO-tls/log_C18_C17_sfotls_tr_j16n_j3p.txt",
        "../Inputs/SFO-tls/log_C18_C17_sfotls_tr_j16p_j3p.txt",
    ],
    is_adding=True,
)

# Set min C2S
sfo.set_min_SF(0.04)

# Define quantum numbers
qd32 = phys.QuantumNumbers.from_str("0d3/2")
qd52 = phys.QuantumNumbers.from_str("0d5/2")
qs12 = phys.QuantumNumbers.from_str("1s1/2")

# Get strength and max Ex
for q, lis in sfo.data.items():
    if(len(lis) == 0):
        continue
    stre = sum(val.SF for val in lis)  # type: ignore
    max_ex = max(val.Ex for val in lis)  # type: ignore
    print(f"Strength for {q.format_simple()} = {stre:.3f} from Ex [0, {max_ex:.3f}] MeV")
