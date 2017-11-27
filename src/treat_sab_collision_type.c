//
// Created by xaq on 9/2/17.
//

#include "acedata.h"
#include "global_fun.h"
#include "RNG.h"
#include "neutron_transport.h"

void treat_sab_colli_type(const nuclide_t *sab_nuc, double SIG_sab_el, double SIG_sab_inel, double incident_erg,
                          double *incident_dir, double *exit_erg_lab, double *exit_dir)
{
    int sab_n_el, sab_n_inel;
    double sab_k_el, sab_k_inel;
    double exit_mu_lab;

    ////////////////// elastic scattering case.  JXS(4) = 0 : without elastic scattering ////////////////
    if(GetLocOfSabElErg(sab_nuc) != 0 && get_rand() * (SIG_sab_el + SIG_sab_inel) > SIG_sab_inel) {
        *exit_erg_lab = incident_erg;

        int NE_el = (int)(sab_nuc->XSS[GetLocOfSabElErg(sab_nuc)]);
        int min = GetLocOfSabElErg(sab_nuc) + 1;
        int max = GetLocOfSabElErg(sab_nuc) + NE_el;

        get_intplt_pos_fr_double(sab_nuc->XSS, incident_erg, min, max, &sab_n_el, &sab_k_el); //// sab_n_el never reach max
        sab_n_el = sab_n_el - GetLocOfSabElErg(sab_nuc);
        int LN = GetLocOfSabElMu(sab_nuc) + (sab_n_el - 1) * (abs(GetSabElDimPara(sab_nuc)) + 1);
        int N_el_mu = GetSabElDimPara(sab_nuc) + 1;
        int Loc;
        int nMode_el = GetSabElMode(sab_nuc);
        if(nMode_el == 2) {     ////// equally-probable angle bins.等概率余弦表
            if(get_rand() <= sab_k_el)
                LN = LN + N_el_mu;
            double ksi = (N_el_mu - 1) * get_rand() + 1;
            Loc = (int)(LN + ksi);
            exit_mu_lab = sab_nuc->XSS[Loc] + (sab_nuc->XSS[Loc - 1] - sab_nuc->XSS[Loc]) * (ksi - (int)(ksi));
        } else if(nMode_el == 3) { /////// qually-probable discrete angles.能/角耦合的等概率离散角  （the only Mode_inel）
            Loc = LN + (int)(N_el_mu * get_rand());
            exit_mu_lab = sab_nuc->XSS[Loc] + sab_k_el * (sab_nuc->XSS[Loc + N_el_mu] - sab_nuc->XSS[Loc]);
        } else if(nMode_el == 4) {///相干弹性散射
            min = GetLocOfSabElXs(sab_nuc);
            max = GetLocOfSabElXs(sab_nuc) + sab_n_el;
            double pi = sab_nuc->XSS[max] * get_rand();
            get_intplt_pos_double(sab_nuc->XSS, pi, min, max, &Loc);
            Loc = Loc + 1 - (int)(sab_nuc->XSS[GetLocOfSabElErg(sab_nuc)]);
            exit_mu_lab = 1 - 2.0 * sab_nuc->XSS[Loc] / incident_erg;
        } else {
            printf("incorrect elastic scattering mode(%d) in sab collision.\n", nMode_el);
            base_warnings++;
        }
    }

        ////////////////// inelastic scattering case.  JXS(4) = 0 : without elastic scattering ////////////////
    else {
        int NE_inel = (int)(sab_nuc->XSS[GetLocOfSabInelErg(sab_nuc)] + 0.5);
        int min = GetLocOfSabInelErg(sab_nuc) + 1;
        int max = GetLocOfSabInelErg(sab_nuc) + NE_inel;
        get_intplt_pos_fr_double(sab_nuc->XSS, incident_erg, min, max, &sab_n_inel, &sab_k_inel);
        sab_n_inel = sab_n_inel - GetLocOfSabInelErg(sab_nuc);

        int LE;
        if(GetSabSecErgMode(sab_nuc) == 0)
            LE = (int)(get_rand() * GetSabInelEoutNum(sab_nuc));
        else {
            double rr = get_rand() * (GetSabInelEoutNum(sab_nuc) - 3);
            if(rr >= 1.0)
                LE = (int)(rr + 1);
            else {
                if(rr >= 0.5) {
                    LE = GetSabInelEoutNum(sab_nuc) - 2;
                    if(rr < 0.6)
                        LE = LE + 1;
                } else {
                    LE = 1;
                    if(rr < 0.1)
                        LE = 0;
                }
            }
        }
        int N_inel_mu = GetSabInelDimPara(sab_nuc) + 1;
        int LN_offset = GetSabInelEoutNum(sab_nuc) * (N_inel_mu + 1);
        int LN = (sab_n_inel - 1) * LN_offset + GetLocOfSabInelErgMu(sab_nuc) + LE * (N_inel_mu + 1);
        *exit_erg_lab = sab_nuc->XSS[LN] + sab_k_inel * (sab_nuc->XSS[LN + LN_offset] - sab_nuc->XSS[LN]);
        if(*exit_erg_lab <= 0) {
            puts("exit energy in sab collision is out of range.");
            base_warnings++;
            *exit_erg_lab = EG0_CUTOFF;
        } else if(*exit_erg_lab <= 1.0E-11) {
            if(*exit_erg_lab > EG0_CUTOFF)
                *exit_erg_lab = 1.0E-11;
        }

        LN = LN + 1;
        int Mode_inel = GetSabInelMode(sab_nuc);
        if(Mode_inel == 2) {  // equally-probable angle bins.
            if(get_rand() <= sab_k_inel)
                LN = LN + LN_offset;
            double ksi = (N_inel_mu - 1) * get_rand() + 1;
            int Loc = (int)(LN + ksi);
            exit_mu_lab = sab_nuc->XSS[Loc] + (sab_nuc->XSS[Loc - 1] - sab_nuc->XSS[Loc]) * (ksi - (int)(ksi));
        } else if(Mode_inel == 3) { // qually-probable discrete angles.
            int Loc = LN + (int)(N_inel_mu * get_rand());
            exit_mu_lab = sab_nuc->XSS[Loc] + sab_k_inel * (sab_nuc->XSS[Loc + LN_offset] - sab_nuc->XSS[Loc]);
        } else {
            printf("incorrect inelastic scattering mode(%d) in sab collision.\n", Mode_inel);
            base_warnings++;
        }
    }


    //////////// check outgoing angle and energy //////////////////////
    exit_mu_lab = MIN(ONE, exit_mu_lab);
    exit_mu_lab = MAX(-ONE, exit_mu_lab);

    rotate_dir(exit_mu_lab, incident_dir, exit_dir);
}