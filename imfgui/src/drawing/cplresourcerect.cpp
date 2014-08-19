#include "cplresourcerect.h"

#include <QEvent>
#include <QMouseEvent>
#include <iostream>
#include <QWidget>

CPLResourceRect::CPLResourceRect(QObject *_parent, QPoint s, QPoint e, QColor c)
    :
    QObject(_parent),
    _selected(false),
    _color(c),
    _startCoordinates(s),
    _endCoordinates(e),
    _sequenceBox(_startCoordinates, _endCoordinates)
{
    //ctor
    if (_parent) {
        _parent->installEventFilter(this);
    }
}

CPLResourceRect::~CPLResourceRect()
{
    //dtor
    std::cout << "Release Resource Rect" << std::endl;
}

void CPLResourceRect::Draw(QPainter &painter)
{
    _sequenceBox.setCoords(_startCoordinates.x(), _startCoordinates.y(), _endCoordinates.x(), _endCoordinates.y());
    painter.fillRect(_sequenceBox, QBrush(_color));
    if (_selected) {
        QLine line (_startCoordinates.x(), _startCoordinates.y(), _endCoordinates.x(), _endCoordinates.y());
        painter.setPen(QPen(QColor(0, 0, 0), 2));
        painter.drawLine(line);
    }
}

bool CPLResourceRect::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (_sequenceBox.contains(mouseEvent->pos())) {
            std::cout << "Click Box: " << _startCoordinates.x() << ":" << _startCoordinates.y() << std::endl;

            emit IGotSelected(this);

            return true;
        }
    }

    return QObject::eventFilter(object, event);
}

#include "../moc_cplresourcerect.cpp"
