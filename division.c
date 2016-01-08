#include "division.h"

void divide(int comp_no, int szer, int wys, int** tabi, int** tabj){



	int pierw = (int)sqrt(comp_no);
	int comp_temp = comp_no;
	int przedzialy[2]; //przedzialy w pionie i poziomie
	przedzialy[0] = 1;
	przedzialy[1] = 1;
	int k = 2;
	int roznica = comp_temp - 1;
        przedzialy[1] = comp_temp;
        for(k = 2; k <= pierw; k++){
                if((comp_temp % k) == 0){
                        if(roznica > (comp_temp/k - k)){
                                roznica = (comp_temp/k - k);
                                if(szer >= wys){
                                        przedzialy[0] = k;
                                        przedzialy[1] = comp_temp/k;
                                }else{
                                        przedzialy[0] = comp_temp/k;
                                        przedzialy[1] = k;
                                }
                        } 
                }
        }
	
	//printf("Liczba przedzialow wierszy: %d\n", przedzialy[0]);
        //printf("Liczba przedzialow kolumn: %d\n", przedzialy[1]);

        //wyznaczamy szerokosci i wysokosci przedzialow
        int szer_przedz = szer/przedzialy[1];
        int wys_przedz = wys/przedzialy[0];

	int i_temp = 0;
	int j_temp = 0;
	int i;
	//ustalanie poczatku i konca przedzialow i, j dla kazdego procesora
	for(i = 0; i < comp_no; i++){
		//jesli przejechalismy pierwszy wiersz przedzialow
		if(((i % przedzialy[1]) == 0)&&(i!=0)){
			i_temp += wys_przedz; //zwiekszamy wartosc poczatkowa przedzialu na "osi" i
			j_temp = 0; //zerujemy wartosc poczatkowa przedzialu na "osi" j
		}

		//przypisanie do tablic odpowiednich granic 
		tabi[i][0] = i_temp;
		tabi[i][1] = tabi[i][0] + wys_przedz - 1;
		
		tabj[i][0] = j_temp;
		tabj[i][1] = tabj[i][0] + szer_przedz - 1;
		j_temp += szer_przedz;

	}	

	for(i = przedzialy[1] - 1; i < comp_no; i += przedzialy[1]){
		tabj[i][1] = szer - 1;
	}

	for(i = przedzialy[0]*przedzialy[1]; i < comp_no; i++){
		tabi[i][1] = wys - 1;
	}

}
