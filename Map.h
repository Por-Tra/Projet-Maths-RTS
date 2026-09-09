#ifndef _MAP_H
#define _MAP_H

#include <QWidget>
#include <QPainter>
#include <QThread>

class Map: public QWidget
{
public:
	Map(int x, int y, QWidget* parent = nullptr) : QWidget(parent), size_x(x), size_y(y) 
    {
    }
	virtual ~Map() = default;

    QSize sizeHint() const override
    {
		return QSize(size_x, size_y);
    }
    QSize minimumSizeHint() const override
    {
        return sizeHint();
    }
    void paintEvent(QPaintEvent* event) override
    {
        QPainter painter(this);
    }

    void start()
    {
        QThread* thread = QThread::create([this]()
            {
                // TODO
            });

        thread->start();
    }

private:
    int size_x, size_y;
};

#endif
