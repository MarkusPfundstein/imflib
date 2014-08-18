#include "genericitem.h"

#include <iostream>

static int GENERIC_ITEM_COUNTER = 0;

GenericItem::GenericItem(const std::string &uuid)
    :
    _uuid(uuid)
{
    GENERIC_ITEM_COUNTER++;
}

GenericItem::~GenericItem()
{
    GENERIC_ITEM_COUNTER--;
    std::cout << "DELETE generic item. [" << GENERIC_ITEM_COUNTER << "] left" << std::endl;
}
