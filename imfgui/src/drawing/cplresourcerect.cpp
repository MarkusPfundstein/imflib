#include "cplresourcerect.h"

CPLResourceRect::CPLResourceRect(int x1, int y1, int x2, int y2)
    :
    _color(230, 120, 120, 230),
    _startCoordinates(x1, y1),
    _endCoordinates(x2, y2),
    _sequenceBox(_startCoordinates, _endCoordinates)
{
    //ctor
}

CPLResourceRect::CPLResourceRect(int x1, int y1, int x2, int y2, int r, int g, int b, int a)
    :
    _color(r, g, b, a),
    _startCoordinates(x1, y1),
    _endCoordinates(x2, y2),
    _sequenceBox(_startCoordinates, _endCoordinates)
{
    //ctor
}

CPLResourceRect::CPLResourceRect(QPoint s, QPoint e, QColor c)
    :
    _color(c),
    _startCoordinates(s),
    _endCoordinates(e),
    _sequenceBox(_startCoordinates, _endCoordinates)
{
    //ctor
}

CPLResourceRect::~CPLResourceRect()
{
    //dtor
}

void CPLResourceRect::Draw(QPainter &painter)
{
    _sequenceBox.setCoords(_startCoordinates.x(), _startCoordinates.y(), _endCoordinates.x(), _endCoordinates.y());
    painter.fillRect(_sequenceBox, QBrush(_color));
}
