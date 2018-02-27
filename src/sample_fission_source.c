//
// Created by xaq on 10/27/17.
//

#include "criticality.h"
#include "geometry.h"


/* 全局变量，所有从核都相同 */
extern double base_start_wgt;
extern universe_t *root_universe;

void sample_fission_source(particle_state_t *par_state, int fis_src_cnt, fission_bank_t *fis_src_slave){
    int i;

    memset(par_state, 0x0, sizeof(particle_state_t));
    for(i = 0; i < 3; i++){
        par_state->pos[i] = fis_src_slave[fis_src_cnt].pos[i];
        par_state->dir[i] = fis_src_slave[fis_src_cnt].dir[i];
    }

    par_state->erg = fis_src_slave[fis_src_cnt].erg;
    par_state->wgt = base_start_wgt;
    par_state->cell = locate_particle(par_state, root_universe, par_state->pos, par_state->dir);

    if(!par_state->cell){
        par_state->is_killed = true;
        return;
    }

    cell_t *cell = par_state->cell;
    if(cell->imp == 0){
        par_state->wgt = ZERO;
        par_state->is_killed = true;
    }

    par_state->mat = cell->mat;
    par_state->cell_tmp = cell->tmp;
}
