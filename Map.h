/*
 * Filename: c:/Users/lucas/Documents/Cour UCA/3A/Modélisation mathématique/Projet-Maths-RTS/Map.h
 * Path: c:/Users/lucas
 * Created Date: Wednesday, September 9th 2026, 4:23:19 pm
 * Author: Lucas Contreras Hodapp
 * 
 * Copyright (c) 2026 Your Company
 */

#ifndef _MAP_H
#define _MAP_H

#include <QWidget>

class Map: public QWidget
{
public:
    Map(QWidget *parent = nullptr);
    virtual ~Map();

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
    void paintEvent(QPaintEvent *event) override;

    void start();
};

#endif
