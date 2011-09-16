// List.hpp - implémentation de la classe template "List", inclus par List.h.

#include <iostream>

// Constructeur par défaut
template <class T>
List<T>::List()
: nb_elements(0), p_first(NULL), p_last(NULL)
{
}

// Constructeur de copie
template <class T>
List<T>::List(const List<T>& ref)
: nb_elements(0), p_first(NULL), p_last(NULL)
{
	*this = ref;
}

// Destructeur
template <class T>
List<T>::~List()
{
	clear();
}

// Ajoute un élément en début de liste
template <class T>
void List<T>::pushFront(const T& element)
{
	// Création du nouveau ListElement; utilisation du constructeur de copie de T.
	ListElement* p_new_list_element = new ListElement(element);

	// Cas particulier de si la liste n'a aucun élément
	if(nb_elements == 0)
	{
		p_first = p_last = p_new_list_element;
		p_first->p_following = NULL;	// NB : sur ces 2 lignes, p_first et p_last sont interchangeables,
		p_last->p_previous = NULL;		// c'est juste écrit comme ça pour une question de logique.
	}
	else
	{
		// On insère ce nouvel élément en début de liste.
		p_new_list_element->p_following = p_first;
		p_first->p_previous = p_new_list_element;
		p_first = p_new_list_element;
	}

	nb_elements++;
}

// Ajoute un élément en fin de liste
template <class T>
void List<T>::pushBack(const T& element)
{
	// Création du nouveau ListElement; utilisation du constructeur de copie de T.
	ListElement* p_new_list_element = new ListElement(element);

	// Cas particulier de si la liste n'a aucun élément
	if(nb_elements == 0)
	{
		p_first = p_last = p_new_list_element;
		p_first->p_following = NULL;	// NB : sur ces 2 lignes, p_first et p_last sont interchangeables,
		p_last->p_previous = NULL;		// c'est juste écrit comme ça pour une question de logique.
	}
	else
	{
		// On insére ce nouvel élément en fin de liste.
		p_new_list_element->p_previous = p_last;
		p_last->p_following = p_new_list_element;
		p_last = p_new_list_element;
	}

	nb_elements++;
}

// Supprime l'élément en début de liste
template <class T>
void List<T>::popFront()
{
	if(nb_elements == 0)
		return;
	else if(nb_elements == 1)
	{
		delete p_first;
		p_first = p_last = NULL;
		nb_elements = 0;
		return;
	}

	ListElement* p_new_first = p_first->p_following;
	p_new_first->p_previous = NULL;
	delete p_first;
	p_first = p_new_first;

	nb_elements--;
}

// Supprime l'élément en fin de liste
template <class T>
void List<T>::popBack()
{
	if(nb_elements == 0)
		return;
	else if(nb_elements == 1)
	{
		delete p_first;
		p_first = p_last = NULL;
		nb_elements = 0;
		return;
	}

	ListElement* p_new_last = p_last->p_previous;
	p_new_last->p_following = NULL;
	delete p_last;
	p_last = p_new_last;

	nb_elements--;
}

// insert() insère un nouvel élément à la place de l'élément pointé par Iterator en
// décalant le reste de la liste vers la droite (i.e. vers la fin).
// Si l'iterator pointe vers l'après-dernier élément, le nouvel élément sera le dernier.
// Le comportement n'est pas spécifié pour si l'iterator pointe vers l'avant-premier élément.
// NB : aprés l'appel à insert(), l'itérateur donné en paramètre pointe vers le nouvel élément inséré.
template <class T>
void List<T>::insert(Iterator& it, const T& element)
{
	ListElement* p_new_list_element = new ListElement(element);

	// Cas d'une liste vide : l'élément rajouté devient le seul élément de la liste
	if(this->nb_elements == 0)
	{
		this->p_first = this->p_last = p_new_list_element;
		nb_elements = 1;
		return;
	}

	// A partir d'ici : on sait que la liste posséde au moins 1 élément.

	// Cas de l'itérateur pointant vers le 1er élément de la liste : on modifie p_first.
	if(it.p_element == this->p_first)
		this->p_first = p_new_list_element;

	// Cas de l'itérateur pointant vers l'après-dernier élément de la liste : on modifie p_last.
	if(it.p_element == NULL && it.p_previous == this->p_last)
		this->p_last = p_new_list_element;

	// On insère l'élément (là, mieux vaut faire un dessin pour piger...).
	if(it.p_element != NULL)
		it.p_element->p_previous = p_new_list_element;

	if(it.p_previous != NULL)
		it.p_previous->p_following = p_new_list_element;

	p_new_list_element->p_following = it.p_element;

	p_new_list_element->p_previous = it.p_previous;

	// On rend valide l'itérateur, qui pointe maintenant vers le nouvel élément.
	it.p_element = p_new_list_element;
	it.p_previous = p_new_list_element->p_previous;
	it.p_following = p_new_list_element->p_following;

	nb_elements++;
}

// Supprime l'élément pointé par it.
// NB : it est rendu invalide aprés erase().
template <class T>
void List<T>::erase(Iterator& it)
{
	if(!it.isValid() || this->nb_elements == 0)
		return;

	// Cas où il n'y a qu'un seul élément dans la liste :
	if(this->nb_elements == 1)
	{
		nb_elements = 0;
		delete this->p_first;
		this->p_first = this->p_last = NULL;
		return;
	}

	// A partir d'ici : la liste a une taille d'au moins 2.

	// Cas où on supprime le 1er élément de la liste : on change this->p_first.
	if(it.p_element == this->p_first)
	{
		this->p_first = this->p_first->p_following;
		delete it.p_element;
		this->p_first->p_previous = NULL;
	}

	// Cas où on supprime le dernier élément de la liste :
	else if(it.p_element == this->p_last)
	{
		this->p_last = this->p_last->p_previous;
		delete it.p_element;
		this->p_last->p_following = NULL;
	}

	// Autres cas (i.e. un élément quelconque au milieu de la liste) :
	else
	{
		it.p_element->p_previous->p_following = it.p_element->p_following;
		it.p_element->p_following->p_previous = it.p_element->p_previous;
		delete it.p_element;
	}

	// On invalide it :
	it.p_element = it.p_following = it.p_previous = NULL;

	nb_elements--;
}

// Supprime le 1er élément qui soit égal à la valeur précisée en argument (utilise T::operator==)
template <class T>
bool List<T>::eraseFirstElement(const T& element)
{
	for(Iterator it = begin() ; it != end() ; it++)
	{
		if((*it) == element)
		{
			erase(it);
			return true;
		}
	}
	return false;
}

// Indique si un certain élément est contenu dans le tableau (utilise l'operator ==)
template <class T>
bool List<T>::contains(const T& element) const
{
	for(Iterator it = begin() ; it != end() ; it++)
	{
		if((*it) == element)
			return true;
	}
	return false;
}

// Supprime tout le contenu de la liste
template <class T>
void List<T>::clear()
{
	if(nb_elements == 0)
		return;

	ListElement* p_element_to_destroy = p_first;

	while(nb_elements > 0)
	{
		ListElement* p_next = p_element_to_destroy->p_following;
		delete p_element_to_destroy;
		p_element_to_destroy = p_next;
		nb_elements--;
	}

	p_first = p_last = NULL;
}

// Copie de la liste : opérateur d'affectation.
template <class T>
List<T>& List<T>::operator=(const List<T>& ref)
{
	this->clear();

	// On parcourt les éléments de "ref" et on les recopie
	for(Iterator it_ref = ref.begin() ; it_ref != ref.end() ; it_ref++)
		this->pushBack(*it_ref);

	return *this;
}

// Affichage du contenu de la liste : pour utiliser cet opérateur,
// il faut que la classe des éléments de la liste ait défini l'opérateur << pour iostream.
template <class T>
std::ostream& operator<<(std::ostream& o, const List<T>& list)
{
	o << '[';
	for(typename List<T>::Iterator it = list.begin() ; it != list.end() ; it++)
	{
		o << (*it);

		typename List<T>::Iterator it_temp = it;
		it_temp++;
		if(it_temp != list.end())
			o << ", ";
	}
	o << ']' << std::flush;
	return o;
}
