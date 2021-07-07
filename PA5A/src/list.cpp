#include "pa5a.hpp"

/**
 * @brief  Fonction permettant d'ajouter un noeud à la liste
 * @param  list    : Le pointeur sur la liste
 * @return newNode : Un pointeur sur le nouveau noeud
 */
Node* add_node(List* list)
{
    Node* newNode = NULL;

    if ((newNode = (Node*)malloc(sizeof(Node))) == NULL)
        return NULL;

    newNode->prev = NULL;
    newNode->next = NULL;
    newNode->data = NULL;

    if (list->head == NULL)
    {
        list->head = newNode;
        list->tail = newNode;
        (list->size)++;
        return newNode;
    }

    if (list->tail == NULL)
        return NULL;

    newNode->prev = list->tail;
    list->tail->next = newNode;
    list->tail = newNode;
    (list->size)++;

    return newNode;
}

/**********************************************************/
/**
 * @brief  Fonction permettant d'initialiser les données que contient le noeud avec les données du joueur
 * @param  node : Le noeud à initialiser
 * @param  data : Les données à stocker dans le noeud
 * @return void : Ne retourne rien
 */
void init_node_data(Node* node, size_t length)
{
    if (node == NULL)
        return;

    if ((node->data = calloc(length, 1)) == NULL)
        return;
}
/**********************************************************/
/**
 * @brief  Fonction permettant de supprimer le noeud situé à l'index voulu de la liste
 * @param  list  : La liste dont il faut supprimer un noeud
 * @param  index : L'index auquel le noeud est situé
 * @return void  : Ne retourne rien
 */
void del_node(List* list, int index)
{
    Node* current = list->head;

    if (list == NULL)
        return;
    if (list->size < index && index >= 0)
        return;

    for (int i = 0; i < index; i++)
    {
        if (current == NULL)
            return;
        current = current->next;
    }
    //Seul noeud de la liste
    if (current->prev == NULL && current->next == NULL)
    {
        list->head = NULL;
        list->tail = NULL;

        if (current->data != NULL)
            free(current->data);

        free(current);
        (list->size)--;
    }
    //Premier noeud de la liste
    else if (current->prev == NULL && current->next != NULL)
    {
        list->head = current->next;

        current->next->prev = NULL;

        if (current->data != NULL)
            free(current->data);

        free(current);
        (list->size)--;
    }
    //Dernier noeud de la liste
    else if (current->next == NULL && current->prev != NULL)
    {
        list->tail = current->prev;

        current->prev->next = NULL;

        if (current->data != NULL)
            free(current->data);

        free(current);
        (list->size)--;
    }
    //Noeud au milieu de la liste
    else if (current->prev != NULL && current->next != NULL)
    {
        current->prev->next = current->next;
        current->next->prev = current->prev;

        if (current->data != NULL)
            free(current->data);

        free(current);
        (list->size)--;
    }
}
/**********************************************************/
/**
 * @brief  Fonction permettant de connaitre l'index auquel un noeud est situé dans une liste
 * @param  head  : Le noeud courant de la liste
 * @param  node  : Le noeud dont on veut connaitre la position dans la liste
 * @return index : La position du noeud
 * @return -1    : Le noeud n'a pas été trouvé dans la liste
 */
int index_of_node(Node* head, Node* node)
{
    int index = 0;

    if (head == NULL || node == NULL)
        return -1;

    while (head != NULL)
    {
        if (head == node)
            return index;

        head = head->next;
        index++;
    }

    return -1;
}
/**********************************************************/
/**
 * @brief  Fonction permettant de connaitre le noeud depuis le pointeur data
 * @param  head  : Le premier noeud de la liste
 * @param  data  : Le pointeur data
 * @return head  : Le noeud correspondant au pointeur data
 * @return NULL    : Le noeud n'a pas été trouvé dans la liste
 */
Node* node_of_data(Node* head, void* data)
{
    int index = 0;

    if (head == NULL || data == NULL)
        return NULL;

    while (head != NULL)
    {
        if (head->data == data)
            return head;

        head = head->next;
        index++;
    }

    return NULL;
}
/**********************************************************/
/**
 * @brief  Fonction permettant de connaitre le noeud situé à un certain index dans une liste
 * @param  head  : Le noeud courant de la liste
 * @param  index : L'index du noeud qu'on souhaite récupérer
 * @return head  : Le noeud situé à l'index demandé
 * @return NULL  : Il n'y a pas de noeud dans la liste à l'index demandé
 */
Node* node_of_index(Node* head, int index)
{
    for (int i = 0; i < index && head != NULL; i++)
        head = head->next;

    if (head == NULL)
        return NULL;

    return head;
}