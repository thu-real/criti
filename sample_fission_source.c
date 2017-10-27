//
// Created by xaq on 10/27/17.
//

#include "criticality.h"
#include "particle_state.h"
#include "geometry.h"


extern particle_state_t base_par_state;

extern criti_t base_criti;

extern double base_start_wgt;

extern map *base_cells;

void sample_fission_source(){
    base_par_state.is_killed = false;

    for(int i = 0; i < 3; i++){
        base_par_state.pos[i] = base_criti.fission_src[base_criti.fission_src_cnt].pos[i];
        base_par_state.dir[i] = base_criti.fission_src[base_criti.fission_src_cnt].dir[i];
    }

    base_par_state.erg = base_criti.fission_src[base_criti.fission_src_cnt].erg;
    base_par_state.wgt = base_start_wgt;
    base_criti.fission_src_cnt++;

    /* TODO: complete this function */
    base_par_state.cell = locate_particle();

    if(base_par_state.cell == -1){
        base_par_state.is_killed = true;
        return;
    }

    cell_t *cell = (cell_t*) map_get(base_cells, base_par_state.cell);
    if(cell && cell->imp == 0){
        base_par_state.wgt = 0.0;
        base_par_state.is_killed = true;
    }

    base_par_state.mat = cell->mat;
}