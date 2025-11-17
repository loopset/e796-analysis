# How to use FRESCO

Brief manual on how to use FRESCO for (in)elastic scattering, DWBA and ADWBA calculations.

## Solving equations parameters

Introduced by the namelist `&FRESCO`, they control the solving of equations and the verbosity of the code.

```
7Li(d,p) @ 7.5AMeV ADWA (d) + KD (p)
NAMELIST
! rmatch= 60 but now 30 fm to mimic 2fnr
! jtmax=40 also as in 2fnr
&FRESCO
hcm=0.10 rmatch=30.0 rintp=0.10
hnl=0.10 rnl=4.50 centre=0.0
jtmin=0.0 jtmax=40.0 absend=-0.1000
thmin=0.00 thmax=-180.00 thinc=1.00
! inh = 2 for zero-range transfer; otherwise, 0
it0=1 iter=1 iblock=0 nnu=48 inh=2
chans=1 listcc=0 treneg=0 cdetr=0 smats=2 xstabl=1 
elab(1)=15 / ! Energy of deuteron
```

- A one-line title is allowed. Useful to describe what reaction and channel the input represents.
- `rmatch` is the maximum integration radius, equivalent to `rmax` in TWOFNR.
- Number of partial waves is `[jtmin, jtmax]`.
- `absend<0` to tell Fresco to include all partial waves and
do not stop early if absorption (non-elastic cross-section) is below `|absend|`.
- Cross-section are calculated in $\theta_{\text{CM}} \in [\text{thmin}, \text{thmax}]$ range and with step `thinc`.
- `ìt0 = iter = 1` usually. Solves just **one-step (A)DBWA**.
- `ìblock > 1` enables coupled-channels calculations by coupling `iblock` pairs of excitation.
energy levels. Useful for **inelastic** excitations.
- `ình=0` in general unless `inh=2` for **zero-range** transfer calculations. See below.
- `elab` is the **total** kinetic energy of the projectile. Remember that we usually work in **direc** kinematics
for theoretical calculations.

## Declaring mass partitions

A `&PARTITION` defines the projectile-like (`p`) and the target-like (`t`) particles per
incoming and outgoing channels, enabling any number of excited states `&STATES` per channel.

```
&PARTITION namep='DEUTERON' massp=2.0136 zp=1 nex=1 namet='Li7' masst=7.014358 zt=3 qval=0/
&STATES jp=1 bandp=1 kkp=0 ep=0.0000 cpot=4 jt=1.5 bandt=-1 kkt=0 et=0.0000/

&PARTITION namep='PROTON' massp=1.0073 zp=1 nex=1 namet='Li8' masst=8.02084 zt=3 qval=-0.1919/
&STATES jp=0.5 bandp=1 ep=0.0000 kkp=0 cpot=1 jt=2.0 bandt=+1 kkt=0 et=0/
&partition /
```

- `nex>=1` is the number of excited states per partition. Remember to change it
if you add more states.
- `j(p,t)` is the spin of the state.
- `band(p,t)` is the band index of the state (first, second, third,... that can be found in collective excitations).
Its sign represents the **parity** of the state.
- `kk(p,t)` is the *K* quantum number of the rotational band. Usually is 0.
- `cpot` is the index of the Optical Model Potential (OMP) representing the elastic scattering of this state.

## Adding potentials

Each potential is employed in a calculation in a different manner. However, their inputs are always similar.

```
! 8Li + p elastic OMP: Koning-Delaroche @ 12.90 MeV equivalent beam energy
&pot kp=1 type=0 p(1:3)= 8.0000  0.0000  1.7783 /
&pot kp=1 type=1 p(1:7)= 55.7641  1.1012  0.6766  1.1129   1.1012  0.6766  0.0000 /
&pot kp=1 type=2 p(1:7)= 0.0000  0.0000   0.000   9.7340  1.3107  0.5229  0.0000 /
&pot kp=1 type=3 p(1:7)= 5.4624  0.8619   0.5900   -0.0535   0.8619   0.5900/
```

The form of the OMP employed by FRESCO is:
$$V(r) = -V_Vf(r, r_V, a_V) - iW_Vf(r, r_W, a_W)$$
$$ -i4a_SW_S\frac{d}{dr}f(r, r_S, a_S)$$
$$-(V_{so} + iW_{so})\left(\frac{\hbar}{m_{\pi}c}\right)^2(2\vec{l}\cdot\vec{s})\frac{1}{r}\frac{d}{dr}f(r, r_{so}, a_{so})$$
where the $f(r, r_i, a_i)$ are usually **Wood-Saxon** functions (`shape=0`). Note the $(\hbar/m_{\pi}c)^2 \simeq 2$ and $\vec{s} = \hbar/2 \vec{\sigma}$ factors when comparing with OMP parametrisations found in the literature.

The different `type`s are:

- `type=0` for Coulomb potential. `p1` is target mass number and `p3` is the Coulomb radius.
- `type=1` for real (`p(1:3)`) and imaginary (`p(4:6)`) **volume** parts.
- `type=2` for imaginary (`p(4:6)`) **surface** part.
- `type=3` for real (`p(1:3)`) and imaginary (`p(4:6)`) **spin-orbit** parts.

## Analysing data

Each subsection below describes how to calculate cross-sections for a particular reaction channel.

### Elastic scattering

With the combination of `&FRESCO + &PARTITION + &POT` namelists, the elastic cross-section in the centre-of-mass
is obtained in the `fort.201` output file.

### Inelastic scattering

An example is found below, corresponding to the inelastic excitation to the first $2^+$ state of $^{20}$0.

```
20O + d 2+1 inelastic;
NAMELIST
 &FRESCO hcm=0.03 rmatch=60 
  jtmin=0.0 jtmax=50 absend=-1
  thmin=3.00 thmax=-180.00 thinc=1.00 
   iter=1 iblock=2
   chans=1 smats=2  xstabl=1 
  elab=70/

 &PARTITION namep='d' massp=2.0135532 zp=1 namet='O20' masst=19.999687 zt=8 qval=-0.000 nex=2  /
 &STATES jp=1 bandp=1 ep=0.0000 cpot=1 jt=0.0 bandt=1 et=0.0000  /
 &STATES copyp=1  cpot=1 jt=2.0 bandt=1 et=1.633 /
 &partition /
 
 &POT kp=1 at=20.000 rc=1.3  /
 &POT kp=1 type=11 p2=5.3 /
 &POT kp=1 type=1 p(1:3)=72.894 1.170 0.828 / ! Real volume
 &POT kp=1 type=11 p2=0.831 /
 &POT kp=1 type=1 p(4:6)=5.431 1.325 0.719  / ! Imaginary volume
 &POT kp=1 type=11 p2=0.937 /
 &POT kp=1 type=2 p(4:6)=8.589 1.325 0.719 /  ! Imaginary surface
 &POT kp=1 type=11 p2=0.937 /
 &POT kp=1 type=3 p(1:3)=2.65 1.07 0.66 / ! Spin-orbit
 &pot / 
 &overlap / 
 &coupling / 
```

Remember to set `iblock>1` (usually `iblock=2`) in `&FRESCO`. Then, each potential
component must be **deformed** in the parameter `pk`, where `k` is the multipolarity:

- Coulomb: $\sqrt{B(Ek\uparrow)}$, in units of $e\cdot fm^{k}$
- Volume/surface: `pk` $=\beta_k \cdot R_{\text{OMP}}$, where $\beta_k = \frac{4\pi}{3ZR_0^k}\sqrt{\frac{B(Ek\uparrow)}{e^2}}$, and $R_0$ is the Coulomb radius. `r` is the radius of each OMP component, so $R_{\text{OMP}} = r_{v,w,s,so} A^{1/3}$ and a deformation must be defined by each OMP displaying a different radius parameter.
- An approach to compute the experimental $\beta_k$ is to generate multiple theoretical calculations
for different $\beta_k$ values and then found that that scales perfectly with data (*scaling factor* $=1$).

### DWBA


