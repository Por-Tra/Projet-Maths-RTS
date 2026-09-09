#include <Map.h>

#include <QPainter>
#include <QThread>

Map::Map(QWidget *parent):
    QWidget(parent)
{
}

Map::~Map()
{
}

QSize Map::sizeHint() const
{
    // TODO
    return QSize(100, 100);
}

QSize Map::minimumSizeHint() const
{
    return sizeHint();
}

void Map::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    // TODO
}

void Map::start()
{
    QThread *thread = QThread::create([this]()
    {
        // TODO
    });

    thread->start();
}
