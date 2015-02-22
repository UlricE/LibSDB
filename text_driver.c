/*******************************************************/
/*                Lahcen  EL HADDATI                   */
/*                    Juin 2002                        */
/*         text_driver pour la librairie sdb           */
/*     recupperation des infos d'un fichier texte      */
/*******************************************************/



#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "sdb.h"

static FILE *f;			//fichier ou on cherchera les donnees


//structure d'une requete avec un select
struct select {
	int nbre_champs;
	char **champs;
	char *fichier;
	char *contrainte;
};

//structure d'une requete avec un update
struct update {
	int nbre_champs;
	char **champs;
	char **val_champs;
	char *fichier;
	char *contrainte;
};

//structure d'une requete avec un insert
struct insert {
	int nbre_champs;
	char **val_champs;
	char *fichier;
};

//structure d'une requete avec un delete
struct delete {
	char *fichier;
	char *contrainte;
};


/*************************************************************/
/*************************************************************/

//on cherche dans requete la chaine buffer et on renvoie la chaine se trouvant apres buffer
static char *cherche(char *requete, char *buffer)
{
	int i, j;
	int test = 1;
	int ok = 0;

	i = j = 0;

	while (test == 1) {
		if (requete[j] == ' ' && ok == 0) {
			j++;
		} else if (requete[i + j] == buffer[i]) {
			i++;
			ok = 1;
		} else if (ok == 1 && requete[i + j] == ' ' && buffer[i] == '\0') {
			test = 0;
		} else if (requete[i + j] != buffer[i]) {
			test = -1;
		}
	}
	if (test == -1)
		return (NULL);
	else
		return (requete + i + j + 1);
}


/**************************************************************/
/**************************************************************/

//on cherche dans requete la chaine buffer et on renvoie ce la chaine qui se trouve avant buffer
static char *avant(char *requete, char *buffer)
{
	int j;
	char *res;

	res = (char *) sdb_calloc(100, sizeof(char));

	if (!strstr(requete, buffer)) {
		return (NULL);
	}
	j = strlen(requete) - strlen((char *) strstr(requete, buffer));

	strncpy(res, requete, j);
	res[j] = '\0';
	return res;
}



/**************************************************************/
/**************************************************************/

// ouverture d'un fichier
static int file_open(char *d, char *t)
{
	f = fopen(d, t);
	if (f == NULL) {
		return (-1);
	} else
		return 0;
}



/*************************************************************/
/*************************************************************/


//renvoie un tableau de chaines contenant les valeurs separees par sep dans buffer
static char **get_valeurs(char *buffer, char sep, int nbre)
{
	int test;
	int i, jr, jb;
	char **result;

	result = (char **) sdb_calloc(nbre, sizeof(char *));
	jb = 0;

	for (i = 0; i < nbre; i++) {
		result[i] = (char *) sdb_calloc(100, sizeof(char *));
		jr = 0;
		test = 1;
		while (test == 1) {
			if (buffer[jb] != ' ' && buffer[jb] != sep && buffer[jb] != '\0') {
				result[i][jr] = buffer[jb];
				jb++;
				jr++;
			} else if (buffer[jb] == ' ') {
				jb++;
			} else if (buffer[jb] == sep) {
				jb++;
				test = 0;
			} else {
				test = 0;
			}
		}
	}
	return result;
}



/*************************************************************/
/*************************************************************/


//calcule le nombre de champs separes par une , dans buffer
static int nbre_champs(char *buffer)
{
	int i = 0;
	int test = 1;
	int nbre = 0;

	while (test == 1) {
		if (buffer[i] != ',' && buffer[i] != '\0') {
			i++;
		} else if (buffer[i] == ',') {
			nbre++;
			i++;
		} else {
			test = 0;
			nbre++;
		}
	}
	return nbre;
}




/*********************************************************/
/*********************************************************/



//enleve les "" ou les '' d'une chaine

static char *enleve(char *buffer)
{
	char *result;
	int i;

	i = 0;

	result = (char *) sdb_calloc(strlen(buffer), sizeof(char));
	if (buffer[0] == '"') {
		while (buffer[i + 1] != '"' && buffer[i + 1] != '\0') {
			result[i] = buffer[i + 1];
			i++;
		}
		if (buffer[i + 1] == '\0') {
			return (NULL);
		} else
			return (result);
	}
	if (buffer[0] == '\'') {
		while (buffer[i + 1] != '\'' && buffer[i + 1] != '\0') {
			result[i] = buffer[i + 1];
			i++;
		}
		if (buffer[i + 1] == '\0') {
			return (NULL);
		} else
			return (result);
	} else
		return (NULL);
}



/**********************************************************/
/**********************************************************/




//enleve les () d'une chaine

static char *enleve_par(char *buffer)
{
	char *result;
	int i, j;

	i = 0;
	j = 0;

	result = (char *) sdb_calloc(strlen(buffer), sizeof(char));
	while (buffer[i] == ' ') {
		i++;
	}

	if (buffer[i] == '(') {
		while (buffer[i + 1] != ')' && buffer[i + 1] != '\0') {
			result[j] = buffer[i + 1];
			i++;
			j++;
		}
		if (buffer[i + 1] == '\0') {
			return (NULL);
		} else
			return (result);
	} else
		return (NULL);
}



/**********************************************************/
/**********************************************************/




// traitement d'une requete de type select, renvoie une struct select. En cas d'erreur dans la requete il met -1 dans nbre_champs

static struct select trait_select(char *requete)
{
	struct select trait;
	struct select erreur;
	char *buffer;
	char *res;

	erreur.nbre_champs = -1;

	buffer = (char *) sdb_calloc(100, sizeof(char));
	res = (char *) sdb_calloc(100, sizeof(char));

	// on recupere la chaine qui se trouve apres select
	buffer = cherche(requete, "select");
	if (!buffer) {
		return (erreur);
	}
	// on recupere tous les champs dans res
	res = avant(buffer, " from ");
	if (!res) {
		return (erreur);
	}
	//on recupere le nbre de champs
	trait.nbre_champs = nbre_champs(res);
	trait.champs = (char **) sdb_calloc(trait.nbre_champs, sizeof(char *));
	//on separe les champs
	trait.champs = get_valeurs(res, ',', trait.nbre_champs);

	//on recupere la chaine apres from
	buffer = cherche(buffer + strlen(res), "from");
	if (!buffer) {
		return (erreur);
	}
	if (!strstr(buffer, " where ")) {	// pas de contrainte
		//on recuper le nom du fichier

		trait.fichier = (get_valeurs(buffer, ',', 1))[0];
		trait.contrainte = "*";
	} else {		//il y a une contrainte
		//on recupere la chaine avant where

		res = avant(buffer, " where ");
		//on recupere le nom du fichier
		trait.fichier = (get_valeurs(res, ',', 1))[0];
		trait.contrainte = (char *) sdb_calloc(50, sizeof(char));
		//on recupere la contrainte
		trait.contrainte = cherche(buffer + strlen(res), "where");
	}

	return trait;
}



/*************************************************************/
/*************************************************************/




// traitement d'une requete de type update, renvoie une struct update. En cas d'erreur dans la requete il met -1 dans nbre_champs

static struct update trait_update(char *requete)
{
	struct update trait;
	struct update erreur;
	char *buffer;
	char **container;
	char **temp;
	char *res;
	int i;

	erreur.nbre_champs = -1;

	buffer = (char *) sdb_calloc(100, sizeof(char));
	res = (char *) sdb_calloc(100, sizeof(char));

	// on recupere la chaine qui se trouve apres update
	buffer = cherche(requete, "update");
	if (!buffer) {
		return (erreur);
	}
	// on recupere la chaine avant set dans res
	res = avant(buffer, " set ");
	if (!res) {
		return (erreur);
	}
	trait.fichier = (get_valeurs(res, ',', 1))[0];

	// on recupere la chaine apres set
	buffer = cherche(buffer + strlen(res), "set");


	if (!strstr(buffer, " where ")) {	// pas de contrainte

		trait.contrainte = "*";
		res = buffer;
	} else {		//il y a une contrainte
		//on recupere la chaine avant where

		res = avant(buffer, " where ");

		trait.contrainte = (char *) sdb_calloc(50, sizeof(char));
		//on recupere la contrainte
		trait.contrainte = cherche(buffer + strlen(res), "where");
	}

	//on recupere le nbre de champs
	trait.nbre_champs = nbre_champs(res);
	trait.champs = (char **) sdb_calloc(trait.nbre_champs, sizeof(char *));
	trait.val_champs = (char **) sdb_calloc(trait.nbre_champs, sizeof(char *));
	container = (char **) sdb_calloc(trait.nbre_champs, sizeof(char *));
	//on separe les champs
	container = get_valeurs(res, ',', trait.nbre_champs);

	temp = (char **) sdb_calloc(2, sizeof(char *));

	for (i = 0; i < trait.nbre_champs; i++) {
		temp = get_valeurs(container[i], '=', 2);
		trait.champs[i] = temp[0];
		trait.val_champs[i] = enleve(temp[1]);
		if (!trait.val_champs[i])
			return (erreur);
	}

	return trait;
}





/************************************************************/
/************************************************************/


// traitement d'une requete de type insert, renvoie une struct insert. En cas d'erreur dans la requete il met -1 dans nbre_champs

static struct insert trait_insert(char *requete)
{
	struct insert trait;
	struct insert erreur;
	char *buffer;
	char *res;

	erreur.nbre_champs = -1;

	buffer = (char *) sdb_calloc(100, sizeof(char));
	res = (char *) sdb_calloc(100, sizeof(char));

	// on recupere la chaine qui se trouve apres insert
	buffer = cherche(requete, "insert");
	if (!buffer) {
		return (erreur);
	}
	// on recupere la chaine qui se trouve apres into
	buffer = cherche(buffer, "into");
	if (!buffer) {
		return (erreur);
	}
	// on recupere la chaine avant values dans res
	res = avant(buffer, " values ");
	if (!res) {
		return (erreur);
	}
	trait.fichier = (get_valeurs(res, ',', 1))[0];

	// on recupere la chaine apres values
	buffer = cherche(buffer + strlen(res), "values");
	res = enleve_par(buffer);

	//on recupere le nbre de champs
	trait.nbre_champs = nbre_champs(res);
	trait.val_champs = (char **) sdb_calloc(trait.nbre_champs, sizeof(char *));

	trait.val_champs = get_valeurs(res, ',', trait.nbre_champs);

	return trait;
}





/************************************************************/
/************************************************************/


// traitement d'une requete de type delete, renvoie une struct delete. En cas d'erreur dans la requete il met NULL dans fichier

static struct delete trait_delete(char *requete)
{
	struct delete trait;
	struct delete erreur;
	char *buffer;
	char *res;

	erreur.fichier = NULL;

	buffer = (char *) sdb_calloc(100, sizeof(char));
	res = (char *) sdb_calloc(100, sizeof(char));

	// on recupere la chaine qui se trouve apres delete
	buffer = cherche(requete, "delete");
	if (!buffer) {
		return (erreur);
	}
	// on recupere la chaine qui se trouve apres from
	buffer = cherche(buffer, "from");
	if (!buffer) {
		return (erreur);
	}
	if (!strstr(buffer, " where ")) {	// pas de contrainte

		trait.contrainte = "*";
		trait.fichier = (char *) sdb_calloc(20, sizeof(char));
		trait.fichier = (get_valeurs(buffer, ',', 1))[0];
	} else {		//il y a une contrainte
		//on recupere la chaine avant where

		res = avant(buffer, " where ");
		trait.fichier = (get_valeurs(res, ',', 1))[0];
		trait.contrainte = (char *) sdb_calloc(50, sizeof(char));
		//on recupere la contrainte
		trait.contrainte = cherche(buffer + strlen(res), "where");
	}

	return trait;
}





/************************************************************/
/************************************************************/






// renvoie un tanleau de chaine contenant les chaine du tableau big dans l'ordre precise dans nim

static char **get_result(char **big, int *num, int nbre)
{
	char **result;
	int i;

	result = (char **) sdb_calloc(nbre, sizeof(char));

	for (i = 0; i < nbre; i++) {
		result[i] = (char *) sdb_calloc(strlen(big[num[i]]) + 1, sizeof(char));
		strcpy(result[i], big[num[i]]);
	}
	return result;
}



/**********************************************************/
/**********************************************************/

//Trouve les chaines du tableau small dans tableau et renvoie l'ordre de ces chaines trouvees

static int *num_champs(char **big, int nb_big, char **small, int nb_small)
{
	int i, j;
	int *res;

	res = (int *) sdb_calloc(nb_small, sizeof(int));

	for (i = 0; i < nb_small; i++) {
		res[i] = -1;
		for (j = 0; j < nb_big; j++) {
			if (strcmp(big[j], small[i]) == 0) {
				res[i] = j;
			}
		}
	}

	return (res);
}



/**************************************************************/
/**************************************************************/

//recupere le premier argument d'une contrainte

static char *recupere1(char *contr)
{
	char *result;
	int i, j;

	result = (char *) sdb_calloc(30, sizeof(char));
	i = 0;
	j = 0;
	while (contr[i] == ' ') {
		i++;
	}
	while (contr[i] != ' ' && contr[i] != '=' && contr[i] != '<' && contr[i] != '>') {
		result[j] = contr[i];
		i++;
		j++;
	}
	return result;
}



/*************************************************************/
/*************************************************************/

//recupere le comparateur d'une contrainte

static char *recupere2(char *contr)
{
	char *result;
	int i, j;

	result = (char *) sdb_calloc(3, sizeof(char));
	i = 0;
	j = 0;

	while (contr[i] != '=' && contr[i] != '<' && contr[i] != '>') {
		i++;
	}
	while (contr[i] == '=' || contr[i] == '<' || contr[i] == '>') {
		result[j] = contr[i];
		i++;
		j++;
	}
	return result;
}



/***********************************************************/
/***********************************************************/

//recupere le deuxieme argument d'une contrainte

static char *recupere3(char *contr)
{
	char *result;
	int i, j;

	result = (char *) sdb_calloc(3, sizeof(char));
	i = 0;
	j = 0;

	while (contr[i] != '=' && contr[i] != '<' && contr[i] != '>') {
		i++;
	}
	while (contr[i] == '=' || contr[i] == '<' || contr[i] == '>' || contr[i] == ' ') {
		i++;
	}
	while (contr[i] != ' ' && contr[i] != '\0') {
		result[j] = contr[i];
		j++;
		i++;
	}
	return result;
}


/**********************************************************/
/**********************************************************/




// verifie une contrainte

static int verif_contrainte(char **nom_champs, char **champs, int nbre, char *contr)
{
	char *arg1;
	char *comp;
	char *arg2;
	int compare[2];
	int i;

	arg1 = (char *) sdb_calloc(30, sizeof(char));
	arg2 = (char *) sdb_calloc(30, sizeof(char));
	comp = (char *) sdb_calloc(3, sizeof(char));

	arg1 = recupere1(contr);
	comp = recupere2(contr);
	arg2 = recupere3(contr);

	compare[0] = compare[1] = -1;

	for (i = 0; i < nbre; i++) {
		if (strcmp(nom_champs[i], arg1) == 0) {
			compare[0] = i;
		}
		if (strcmp(nom_champs[i], arg2) == 0) {
			compare[1] = i;
		}
	}

	if (compare[0] == -1) {
		return (-1);
	}
	if (compare[1] == -1) {

		arg2 = enleve(arg2);

		if (!arg2) {
			return (-1);
		}
		if (strcmp(comp, "=") == 0 && strcmp(champs[compare[0]], arg2) == 0) {
			return 1;
		}
		if (strcmp(comp, "<") == 0 && strcmp(champs[compare[0]], arg2) < 0) {
			return 1;
		}
		if (strcmp(comp, ">") == 0 && strcmp(champs[compare[0]], arg2) > 0) {
			return 1;
		}
		if (strcmp(comp, "<=") == 0 && strcmp(champs[compare[0]], arg2) <= 0) {
			return 1;
		}
		if (strcmp(comp, ">=") == 0 && strcmp(champs[compare[0]], arg2) >= 0) {
			return 1;
		}
	} else {
		if (strcmp(comp, "=") == 0 && strcmp(champs[compare[0]], champs[compare[1]]) == 0) {
			return 1;
		}
		if (strcmp(comp, "<") == 0 && strcmp(champs[compare[0]], champs[compare[1]]) < 0) {
			return 1;
		}
		if (strcmp(comp, ">") == 0 && strcmp(champs[compare[0]], champs[compare[1]]) > 0) {
			return 1;
		}
		if (strcmp(comp, "<=") == 0 && strcmp(champs[compare[0]], champs[compare[1]]) <= 0) {
			return 1;
		}
		if (strcmp(comp, ">=") == 0 && strcmp(champs[compare[0]], champs[compare[1]]) >= 0) {
			return 1;
		}
	}

	return 0;
}


/************************************************************/
/************************************************************/




//calcule le nombre de champs de l'entete separes par sep
static int nbre_entete(char *buffer, char sep)
{
	int i = 0;
	int test = 1;
	int nbre = 0;

	while (test == 1) {
		if (buffer[i] != sep && buffer[i] != '\0') {
			i++;
		} else if (buffer[i] == sep) {
			nbre++;
			i++;
		} else {
			test = 0;
		}
	}
	return nbre;
}



/************************************************************/
/************************************************************/


static int trait_entete(char *buffer, char *sep, char ***champs, int *nbre_ch)
{

	char *result;
	int i, j;
	int test = 1;
	int nbre = 0;

	i = 0;
	j = 0;

	result = (char *) sdb_calloc(strlen(buffer), sizeof(char));

	if (buffer[0] == '{') {
		while (buffer[i + 1] != '}' && buffer[i + 1] != '\0') {
			result[i] = buffer[i + 1];
			i++;
		}
		if (buffer[i + 1] == '\0') {
			return (-1);
		} else
			*sep = buffer[i + 2];
	}
	*nbre_ch = nbre_entete(buffer, *sep);

	*champs = (char **) sdb_calloc(*nbre_ch, sizeof(char *));

	while (test == 1) {

		i = 0;
		memset(result, 0, strlen(result));
		if (buffer[0] == '{') {
			while (buffer[i + 1] != '}' && buffer[i + 1] != '\0') {
				result[i] = buffer[i + 1];
				i++;
			}
			if (buffer[i + 1] == '\0') {
				return (-1);
			} else if (buffer[i + 2] != *sep) {
				exit(-1);
			}
			(*champs)[nbre] = (char *) sdb_calloc(strlen(result), sizeof(char));
			strcpy((*champs)[nbre], result);
			nbre++;
			if (buffer[i + 3] == '\0') {
				test = 0;
			} else
				buffer = buffer + i + 3;
		} else
			return (-1);
	}

	return (0);
}


/************************************************************/
/************************************************************/



static int text_driver(void *pdb, char *d, char *q,
		   int (*callback) (int, char **, void *), void *closure)
{
	struct select query;
	struct update uquery;
	struct insert iquery;
	struct delete dquery;
	int res;
	int nbre_total = 0;
	int nbre_ch;
	char **result;
	char **resultat;
	char *buffer = malloc(255 * sizeof(char));
	char **champs;
	char sep;
	int i, j;
	int *num_ch;
	int modifie;
	char *file;
	FILE *ftemp;


	/****************************************/
	/*         REQUETE    SELECT            */
	/****************************************/

	if (strstr(q, "select")) {
		query = trait_select(q);
		if (query.nbre_champs == -1) {
			return (-1);
		}
		num_ch = (int *) sdb_calloc(query.nbre_champs, sizeof(int));

		file = (char *) sdb_calloc(30, sizeof(char));
		sprintf(file, "%s%s", d, query.fichier);
		res = file_open(file, "r");
		if (res < 0) {
			return (-1);
		}
		fscanf(f, "%s", buffer);

		res = trait_entete(buffer, &sep, &champs, &nbre_ch);

		if (res == -1) {
			printf("Erreur dans l'entete\n");
			fclose(f);
			return (-1);
		}
		if (query.nbre_champs == 1 && strcmp(query.champs[0], "*") == 0) {
			free(query.champs[0]);
			free(query.champs);
			query.nbre_champs = nbre_ch;
			query.champs = (char **) sdb_calloc(nbre_ch, sizeof(char *));
			for (i = 0; i < nbre_ch; i++) {
				query.champs[i] = (char *) sdb_calloc(strlen(champs[i]) + 1, sizeof(char));
				strcpy(query.champs[i], champs[i]);
			}
		}
		while (!feof(f)) {
			memset(buffer, 0, strlen(buffer));
			fscanf(f, "%s", buffer);
			result = get_valeurs(buffer, sep, nbre_ch);
			if (strcmp(query.contrainte, "*") == 0 || verif_contrainte(champs, result, nbre_ch, query.contrainte) > 0) {
				num_ch = num_champs(champs, nbre_ch, query.champs, query.nbre_champs);
				resultat = (char **) calloc(query.nbre_champs, sizeof(int));
				resultat = get_result(result, num_ch, query.nbre_champs);
				(*callback) (query.nbre_champs, resultat, closure);
				nbre_total++;
			}
			for (i = 0; i < nbre_ch; i++) {
				free(result[i]);
			}
			free(result);
		}
		fclose(f);
		return nbre_total;
	}
	/****************************************/
	/*         REQUETE    UPDATE            */
	/****************************************/

	else if (strstr(q, "update")) {
		uquery = trait_update(q);
		if (uquery.nbre_champs == -1) {
			return (-1);
		}
		num_ch = (int *) calloc(uquery.nbre_champs, sizeof(int));

		file = (char *) calloc(30, sizeof(char));
		sprintf(file, "%s%s", d, uquery.fichier);

		res = file_open(file, "r");
		ftemp = fopen("fichier_temporaire.perso", "w");
		if (res < 0) {
			return (-1);
		}
		fscanf(f, "%s", buffer);

		res = trait_entete(buffer, &sep, &champs, &nbre_ch);

		if (res == -1) {
			printf("Erreur dans l'entete\n");
			fclose(f);
			return (-1);
		}
		while (!feof(f)) {
			fscanf(f, "%s", buffer);
			result = get_valeurs(buffer, sep, nbre_ch);
			if (strcmp(uquery.contrainte, "*") == 0 || verif_contrainte(champs, result, nbre_ch, uquery.contrainte) > 0) {
				num_ch = num_champs(champs, nbre_ch, uquery.champs, uquery.nbre_champs);
				resultat = (char **) calloc(nbre_ch, sizeof(char *));
				for (i = 0; i < nbre_ch; i++) {
					modifie = -1;
					for (j = 0; j < uquery.nbre_champs; j++) {
						if (num_ch[j] == i)
							modifie = j;
					}
					if (modifie == -1)
						resultat[i] = result[i];
					else
						resultat[i] = uquery.val_champs[modifie];
				}
				for (i = 0; i < nbre_ch; i++) {
					fprintf(ftemp, "%s%c", resultat[i], sep);
				}
				if (!feof(f))
					fprintf(ftemp, "\n");
				nbre_total++;
			} else {
				for (i = 0; i < nbre_ch; i++) {
					fprintf(ftemp, "%s%c", result[i], sep);
				}
				if (!feof(f))
					fprintf(ftemp, "\n");
			}

			for (i = 0; i < nbre_ch; i++) {
				free(result[i]);
			}
			free(result);

		}
		fclose(f);
		fclose(ftemp);

		res = file_open(file, "w");
		if (res < 0) {
			return (-1);
		}
		ftemp = fopen("fichier_temporaire.perso", "r");

		for (i = 0; i < nbre_ch; i++) {
			fprintf(f, "{%s}%c", champs[i], sep);
		}

		while (!feof(ftemp)) {
			fscanf(ftemp, "%s", buffer);
			fprintf(f, "\n%s", buffer);
		}
		fclose(f);
		fclose(ftemp);

		unlink("fichier_temporaire.perso");
		return (nbre_total);
	}
	/****************************************/
	/*         REQUETE    INSERT            */
	/****************************************/


	else if (strstr(q, "insert")) {
		iquery = trait_insert(q);
		if (iquery.nbre_champs == -1) {
			return (-1);
		}
		file = (char *) calloc(30, sizeof(char));
		sprintf(file, "%s%s", d, iquery.fichier);

		res = file_open(file, "r");
		if (res < 0) {
			return (-1);
		}
		fscanf(f, "%s", buffer);

		res = trait_entete(buffer, &sep, &champs, &nbre_ch);

		if (res == -1) {
			printf("Erreur dans l'entete du fichier\n");
			fclose(f);
			return (-1);
		}
		fclose(f);

		res = file_open(file, "a");
		if (res < 0) {
			return (-1);
		}
		fprintf(f, "\n");

		for (i = 0; i < iquery.nbre_champs; i++) {
			memset(buffer, 0, strlen(buffer));
			buffer = enleve(iquery.val_champs[i]);
			fprintf(f, "%s%c", buffer, sep);
		}
		fclose(f);

		return (0);
	}
	/****************************************/
	/*         REQUETE    DELETE            */
	/****************************************/

	else if (strstr(q, "delete")) {
		dquery = trait_delete(q);
		if (!dquery.fichier) {
			return (-1);
		}
		file = (char *) calloc(30, sizeof(char));
		sprintf(file, "%s%s", d, dquery.fichier);

		res = file_open(file, "r");
		ftemp = fopen("fichier_temporaire.perso", "w");
		if (res < 0) {
			return (-1);
		}
		fscanf(f, "%s", buffer);

		res = trait_entete(buffer, &sep, &champs, &nbre_ch);

		if (res == -1) {
			printf("Erreur dans l'entete\n");
			fclose(f);
			return (-1);
		}
		while (!feof(f)) {
			fscanf(f, "%s", buffer);
			result = get_valeurs(buffer, sep, nbre_ch);
			if (strcmp(dquery.contrainte, "*") == 0 || verif_contrainte(champs, result, nbre_ch, dquery.contrainte) > 0) {
				nbre_total++;
			} else {
				for (i = 0; i < nbre_ch; i++) {
					fprintf(ftemp, "%s%c", result[i], sep);
				}
				if (!feof(f))
					fprintf(ftemp, "\n");
			}

			for (i = 0; i < nbre_ch; i++) {
				free(result[i]);
			}
			free(result);

		}
		fclose(f);
		fclose(ftemp);

		res = file_open(file, "w");
		if (res < 0) {
			return (-1);
		}
		ftemp = fopen("fichier_temporaire.perso", "r");

		for (i = 0; i < nbre_ch; i++) {
			fprintf(f, "{%s}%c", champs[i], sep);
		}

		while (!feof(ftemp)) {
			fscanf(ftemp, "%s", buffer);
			fprintf(f, "\n%s", buffer);
		}
		fclose(f);
		fclose(ftemp);

		unlink("fichier_temporaire.perso");

		return (nbre_total);
	} else {
		return -1;
	}
}

void sdb_init_text(void)
{
	sdb_register_driver("text", text_driver, NULL, NULL);
}
