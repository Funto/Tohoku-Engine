// List.h

/*
	"List" est une classe de liste chaînée personnalisée.
	Elle n'est PAS circulaire, et un itérateur est considéré comme invalide (Iterator::isValid() == false)
	lorsqu'il ne pointe pas vers un élément particulier (i.e. il ne peut pas être déréférencé).
	C'est par exemple le cas de l'itérateur renvoyé par end().

	Lors d'un ajout d'élément à la liste, une copie de l'élément ajouté est créée.
	-> La classe des éléments de la liste doit donc posséder un constructeur de copie.

	De plus, pour l'utilisation de l'operateur "=" et pour le constructeur de copie de "List" :
	-> Il faut que la classe des éléments de la liste possédent un operateur "=".

	Enfin, pour utiliser l'affichage via l'opérateur << :
	-> Il faut que la classe des éléments de la liste définisse l'opérateur << pour iostream.
*/

#ifndef LIST_H
#define LIST_H

#include <iostream>

template <class T>
class List
{
protected:
	// Structure représentant un élément de la liste
	struct ListElement
	{
		ListElement* p_previous;
		T element;
		ListElement* p_following;

		ListElement()
		: p_previous(NULL), element(), p_following(NULL)
		{
			std::cout << "ATTENTION : la fonction en " << __FILE__ << ":" << __LINE__ << " n'aurait pas du etre executee !" << std::endl;
		}

		// Constructeur utilisant un T& en paramètre : utilisé par pushFront() et pushBack().
		ListElement(const T& ref_T)
		: p_previous(NULL), element(ref_T), p_following(NULL)
		{
		}

		ListElement(const ListElement& ref)
		: p_previous(ref.p_previous), element(ref.element), p_following(ref.p_following)
		{
			std::cout << "ATTENTION : la fonction en " << __FILE__ << ":" << __LINE__ << " n'aurait pas du etre executee !" << std::endl;

			*this = ref;
		}

		~ListElement()
		{
		}

		ListElement& operator=(const ListElement& ref)
		{
			this->p_previous = ref.p_previous;
			this->element = ref.element;	// Appel de l'operateur "=" de la classe des éléments de la liste.
			this->p_following = ref.p_following;
			return *this;
		}
	};

	unsigned nb_elements;
	ListElement* p_first;
	ListElement* p_last;

public:
	// Classe représentant un itérateur, permettant de parcourir la liste dans les 2 sens
	class Iterator
	{
	protected:
		ListElement* p_previous;
		ListElement* p_element;
		ListElement* p_following;
	public:
		Iterator()
		: p_previous(NULL), p_element(NULL), p_following(NULL)
		{
		}

		Iterator(const Iterator& ref)
		{
			this->p_previous = ref.p_previous;
			this->p_element = ref.p_element;
			this->p_following = ref.p_following;
		}

		~Iterator()
		{
		}

		Iterator& operator=(const Iterator& ref)
		{
			this->p_previous = ref.p_previous;
			this->p_element = ref.p_element;
			this->p_following = ref.p_following;
			return *this;
		}

		bool isValid() const {return p_element != NULL;}

		void operator++(int i)	// i doit être fourni (question de syntaxe) mais est ignoré !
		{
			this->p_previous = this->p_element;
			this->p_element = this->p_following;
			if(this->p_following != NULL)
				this->p_following = this->p_following->p_following;
		}

		void operator--(int i)	// i doit être fourni (question de syntaxe) mais est ignoré !
		{
			this->p_following = this->p_element;
			this->p_element = this->p_previous;
			if(this->p_previous != NULL)
				this->p_previous = this->p_previous->p_previous;
		}

		bool operator==(const Iterator& ref) const
		{
			return	(p_previous == ref.p_previous) &&
					(p_element == ref.p_element) &&
					(p_following == ref.p_following);
		}

		bool operator!=(const Iterator& ref) const
		{
			return	!(*this == ref);
		}

		// Déréférencement :
		T& operator*()
		{
			return p_element->element;
		}

		// Pour que la classe List ait accès aux membres de Iterator
		friend class List<T>;
	};

	List<T>();
	List<T>(const List& ref);
	~List<T>();

	void pushFront(const T& element);	// Ajoute un élément en début de liste
	void pushBack(const T& element);	// Ajoute un élément en fin de liste

	void popFront();	// Supprime l'élément en début de liste
	void popBack();		// Supprime l'élément en fin de liste

	// insert() insère un nouvel élément à la place de l'élément pointé par Iterator en
	// décalant le reste de la liste vers la droite (i.e. vers la fin).
	// Si l'iterator pointe vers l'aprés-dernier élément, le nouvel élément sera le dernier.
	// Le comportement n'est pas spécifié pour si l'iterator pointe vers l'avant-premier élément.
	// NB : aprés l'appel à insert(), l'itérateur donné en paramètre pointe vers le nouvel élément inséré.
	void insert(Iterator& it, const T& element);

	// Supprime l'élément pointé par it.
	// NB : it est rendu invalide aprés erase().
	void erase(Iterator& it);

	// Supprime le 1er élément qui soit égal à la valeur précisée en argument (utilise T::operator==)
	// Renvoie true si l'élément a été trouvé, et donc supprimé.
	bool eraseFirstElement(const T& element);

	void clear();	// Supprime tout le contenu de la liste

	unsigned size() const {return nb_elements;}	// Taille de la liste

	bool contains(const T& element) const;	// Indique si un certain élément est contenu dans le tableau (utilise l'operator ==)

	// Renvoie un iterateur pointant vers le 1er élément trouvé qui soit égal à la valeur passée, ou end() si l'élément
	// n'a pas été trouvé.
	Iterator findFirstElement(const T& element)
	{
		Iterator it;
		for(it = begin() ; it != end() ; it++)
			if(*it == element)
				break;
		return it;
	}

	// begin() renvoie un iterateur correspondant au 1er élément de la liste
	Iterator begin() const
	{
		Iterator it;

		if(nb_elements == 0)
			it.p_previous = it.p_element = it.p_following = NULL;
		else
		{
			it.p_previous = NULL;
			it.p_element = p_first;
			it.p_following = p_first->p_following;
		}

		return it;
	}

	// end() renvoie un itérateur correspondant à l'élément qui suivrait le dernier élément de la liste
	Iterator end() const
	{
		Iterator it;

		if(nb_elements == 0)
			it.p_previous = it.p_element = it.p_following = NULL;
		else
		{
			it.p_previous = p_last;
			it.p_element = NULL;
			it.p_following = NULL;
		}

		return it;
	}

	// rBegin() renvoie un itérateur correspondant à l'élément qui serait avant le 1er élément de la liste
	Iterator rBegin() const
	{
		Iterator it;

		if(nb_elements == 0)
			it.p_previous = it.p_element = it.p_following = NULL;
		else
		{
			it.p_previous = NULL;
			it.p_element = NULL;
			it.p_following = p_first;
		}

		return it;
	}

	// rEnd() renvoie un itérateur correspondant à l'élément qui serait aprés le dernier élément de la liste
	Iterator rEnd() const
	{
		Iterator it;

		if(nb_elements == 0)
			it.p_previous = it.p_element = it.p_following = NULL;
		else
		{
			it.p_previous = p_last->p_previous;
			it.p_element = p_last;
			it.p_following = NULL;
		}

		return it;
	}

	List& operator=(const List& ref);	// Copie de la liste
};

// Affichage du contenu de la liste : pour utiliser cet opérateur,
// il faut que la classe des éléments de la liste ait défini l'opérateur << pour iostream.
template <class T>
std::ostream& operator<<(std::ostream& o, const List<T>& list);

// Implémentation
#include "List.hpp"

#endif // LIST_H
