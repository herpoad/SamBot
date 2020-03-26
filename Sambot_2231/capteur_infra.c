#include <msp430.h>
#include "ADC.h"


#define SEUIL 400

volatile unsigned int thex = 0;
void init_infra(void) {
    ADC_init();
}

int get_distance_infra(void)
{
	volatile unsigned int i, somme=0;

	for(i=0; i<5; i++){
	    if (i != 0){
	        ADC_Demarrer_conversion(0);
	        thex = ADC_Lire_resultat();
	    }
	}

    if(thex < SEUIL){
        return 1;
    }
    else{
        return 0;
    }



}
