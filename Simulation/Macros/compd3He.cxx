// Macro to compare our efficiency with Juan's
// To serve as a validation of our computation

#include "Interpolators.h"

void compd3He()
{
    // Init object
    // Interpolators::Efficiency effsd3He;
    // effsd3He.Add("Ours", "../Outputs/e796_beam_20O_target_2H_light_3He_Eex_0.00_nPS_0_pPS_0.root");
    // effsd3He.Add("Juan's", "/media/miguel/FICA_4/Juan/Asimp/ProducedEfficiencyFile/20O_and_2H_to_3He_NumN_0_NumP_0_Ex0_Date_2024_9_18_Time_10_43.root");
    // effsd3He.Draw();
    //
    // Interpolators::Efficiency effsd3H;
    // effsd3H.Add("Ours", "../Outputs/e796_beam_20O_target_2H_light_3H_Eex_0.00_nPS_0_pPS_0.root");
    // effsd3H.Add("Juan's", "/media/miguel/FICA_4/Juan/Asimp/ProducedEfficiencyFile/20O_and_2H_to_3H_NumN_0_NumP_0_Ex0_Date_2024_9_18_Time_15_57.root");
    // effsd3H.Draw();
    
    // Interpolators::Efficiency effsSet;
    // effsSet.Add("Onte", "../Outputs/Old/e796_beam_20O_target_2H_light_3H_Eex_0.00_nPS_0_pPS_0.root");
    // effsSet.Add("Hoje", "/media/Data/E796v2/Simulation/Outputs/tree_20O_2H_3H_0.00_nPS_0_pPS_0_low_RPx.root");
    // effsSet.Draw();

    // Interpolators::Efficiency effsdd {};
    // effsdd.Add("Ours", "/media/Data/E796v2/Simulation/Outputs/juan_RPx/tree_20O_2H_2H_0.00_nPS_0_pPS_0.root");
    // effsdd.Add("Juan's", "/media/miguel/FICA_4/Juan/Asimp/ProducedEfficiencyFile/20O_and_2H_to_2H_NumN_0_NumP_0_Ex0_Date_2024_10_2_Time_10_23.root");
    // effsdd.Draw();
    
    Interpolators::Efficiency effspp {};
    effspp.Add("Ours", "/media/Data/E796v2/Simulation/Outputs/juan_RPx/tree_20O_1H_1H_0.00_nPS_0_pPS_0.root");
    effspp.Add("Juan's", "/media/miguel/FICA_4/Juan/Asimp/ProducedEfficiencyFile/20O_and_1H_to_1H_NumN_0_NumP_0_Ex0_Date_2024_10_4_Time_9_27.root");
    effspp.Draw();
}