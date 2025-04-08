// Macro to compare our efficiency with Juan's
// To serve as a validation of our computation

#include "Interpolators.h"

void compEffsJuan()
{
    // Init object
    // Interpolators::Efficiency effsd3He;
    // effsd3He.Add("Ours", "../Outputs/e796_beam_20O_target_2H_light_3He_Eex_0.00_nPS_0_pPS_0.root");
    // effsd3He.Add("Juan's",
    // "/media/miguel/FICA_4/Juan/Asimp/ProducedEfficiencyFile/20O_and_2H_to_3He_NumN_0_NumP_0_Ex0_Date_2024_9_18_Time_10_43.root");
    // effsd3He.Draw();

    // Interpolators::Efficiency effsd3H;
    // effsd3H.Add("Ours", "/media/Data/E796v2/Simulation/Outputs/juan_RPx/tree_20O_2H_3H_0.00_nPS_0_pPS_0.root");
    // effsd3H.Add("Juan's",
    // "/media/miguel/FICA_4/Juan/Asimp/ProducedEfficiencyFile/20O_and_2H_to_3H_NumN_0_NumP_0_Ex0_Date_2025_1_20_Time_14_28.root");
    // effsd3H.Draw();

    // Interpolators::Efficiency effsSet;
    // effsSet.Add("Onte", "../Outputs/Old/e796_beam_20O_target_2H_light_3H_Eex_0.00_nPS_0_pPS_0.root");
    // effsSet.Add("Hoje", "/media/Data/E796v2/Simulation/Outputs/tree_20O_2H_3H_0.00_nPS_0_pPS_0_low_RPx.root");
    // effsSet.Draw();

    // Interpolators::Efficiency effsdd {};
    // effsdd.Add("Ours", "/media/Data/E796v2/Simulation/Outputs/juan_RPx/tree_20O_2H_2H_0.00_nPS_0_pPS_0.root");
    // effsdd.Add("Juan's",
    // "/media/miguel/FICA_4/Juan/Asimp/ProducedEfficiencyFile/20O_and_2H_to_2H_NumN_0_NumP_0_Ex0_Date_2024_10_2_Time_10_23.root");
    // effsdd.Draw();

    // Interpolators::Efficiency effspp {};
    // effspp.Add("Ours", "/media/Data/E796v2/Simulation/Outputs/juan_RPx/tree_20O_2H_3H_0.00_nPS_0_pPS_0.root");
    // effspp.Add("Juan's",
    // "/media/miguel/FICA_4/Juan/Asimp/ProducedEfficiencyFile/20O_and_2H_to_3H_NumN_0_NumP_0_Ex0_Date_2024_9_27_Time_9_34.root");
    // effspp.Add("Juan's",
    // "/media/miguel/FICA_4/Juan/Asimp/ProducedEfficiencyFile/20O_and_2H_to_2H_NumN_0_NumP_0_Ex0_Date_2024_10_2_Time_10_23.root");
    // effspp.Draw();

    // Interpolators::Efficiency effspd;
    // effspd.Add("Ours", "/media/Data/E796v2/Simulation/Outputs/juan_RPx/tree_20O_1H_2H_0.00_nPS_0_pPS_0.root");
    // effspd.Add("Juan's", "/media/miguel/FICA_4/Juan/Asimp/ProducedEfficiencyFile/20O_and_1H_to_2H_NumN_0_NumP_0_Ex0_Date_2025_2_7_Time_9_43.root");
    // effspd.Draw();

    Interpolators::Efficiency effsdt;
    effsdt.Add("Ours", "/media/Data/E796v2/Simulation/Outputs/juan_RPx/tree_20O_2H_3H_14.90_nPS_0_pPS_0.root");
    effsdt.Add("Juan's", "/media/miguel/FICA_4/Juan/Asimp/ProducedEfficiencyFile/20O_and_2H_to_3H_NumN_0_NumP_0_Ex14.9_Date_2025_4_8_Time_9_21.root");
    effsdt.Draw();
}
