import pyphysics as phys

exp = phys.parse_txt("../Outputs/xs/g1_xs.dat", 3)
c = phys.Comparator(exp)
c.add_model("BG 0.2", "../Search/g1_BG/beta_0.200/fort.202")
c.fit(True)
