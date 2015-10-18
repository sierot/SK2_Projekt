#include "division.h"

void divide(int comp_no, int szer, int wys, int** tabi, int** tabj){

	//zakladamy ze mamy n^2 procesorow gdzie n to liczba naturalna
	int liczba_przedzialow = (int)sqrt(comp_no);

	//wyznaczamy szerokosci i wysokosci przedzialow
	int szer_przedz = szer/liczba_przedzialow;
	int wys_przedz = wys/liczba_przedzialow ;

	int i_temp = 0;
	int j_temp = 0;
	int i, j;
	//ustalanie poczatku i konca przedzialow i, j dla kazdego procesora
	for(i = 0; i < comp_no; i++){
		//jesli przejechalismy pierwszy wiersz przedzialow
		if(((i % (comp_no/liczba_przedzialow)) == 0)&&(i!=0)){
			i_temp += wys_przedz; //zwiekszamy wartosc poczatkowa przedzialu na "osi" i
			j_temp = 0; //zerujemy wartosc poczatkowa przedzialu na "osi" j
		}
		//przypisanie do tablic odpowiednich granic 
		tabi[i][0] = i_temp;
		tabi[i][1] = tabi[i][0] + wys_przedz - 1;
		
		tabj[i][0] = j_temp;
		tabj[i][1] = tabj[i][0] + szer_przedz - 1;
		j_temp += wys_przedz;

	}
	
	/*for(i = 0; i < comp_no; i++){
		printf("Procesor %d ma przedzial: i:[%d,%d]   j:[%d,%d]\n", i, tabi[i][0], tabi[i][1], tabj[i][0], tabj[i][1]);
	}*/
}
