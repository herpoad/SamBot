#include <msp430.h>
#include "ADC.h"


#define SEUIL 400

void init_infra(void) {
    ADC_init();
}

int get_distance_infra(void) {
    unsigned int sum = 0;
    for(i=0; i<5; i++){
	    if (i != 0){
	        ADC_Demarrer_conversion(0);
	        sum = ADC_Lire_resultat();
	    }
	}
    return sum;
}

int is_hole(unsigned int sum) {
    if(sum > SEUIL){
        return 1;
    } else {
        return 0;
    }
}
