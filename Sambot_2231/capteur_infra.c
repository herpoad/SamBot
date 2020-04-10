#include <msp430.h>
#include "ADC.h"


#define SEUIL 400

unsigned int sum = 0;

void init_infra(void) {
    ADC_init();
}

int get_distance_infra(void) {
    int i;
    sum = 0;
    for(i = 10; i > 0; i--) {
        ADC_Demarrer_conversion(0);
        sum += ADC_Lire_resultat();
    }

    return sum/200;
}

int is_hole(unsigned int sum) {
    if(sum > SEUIL){
        return 1;
    } else {
        return 0;
    }
}
